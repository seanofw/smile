//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Command-Line Runner)
//  Copyright 2004-2017 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include "stdafx.h"
#include <stdlib.h>

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <io.h>
#elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
#	include <time.h>
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <dirent.h>
#	include <errno.h>
#endif

#include "style.h"

#define FILE_TYPE_MASK	0170000
#define FILE_TYPE_DIR	0040000
#define FILE_TYPE_CHR	0020000
#define FILE_TYPE_BLK	0060000
#define FILE_TYPE_REG	0100000
#define FILE_TYPE_LNK	0120000
#define FILE_TYPE_SOCK	0140000
#define FILE_TYPE_FIFO	0010000

#define FILE_MODE_SETUID	0004000
#define FILE_MODE_SETGID	0002000
#define FILE_MODE_STICKY	0001000

#define FILE_MODE_OWNER_READ	0000400
#define FILE_MODE_OWNER_WRITE	0000200
#define FILE_MODE_OWNER_EXEC	0000100
#define FILE_MODE_GROUP_READ	0000040
#define FILE_MODE_GROUP_WRITE	0000020
#define FILE_MODE_GROUP_EXEC	0000010
#define FILE_MODE_WORLD_READ	0000004
#define FILE_MODE_WORLD_WRITE	0000002
#define FILE_MODE_WORLD_EXEC	0000001

typedef struct FileInfoStruct {
	String name;
	String nameCaseFolded;
	UInt64 size;
	UInt32 mode;
	UInt16 year;
	Byte month;
	Byte day;
	Byte hour;
	Byte minute;
} *FileInfo;

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
	static String GetLastErrorString()
	{
		DWORD errorMessageID;
		LPWSTR messageBuffer;
		Int messageLength;
		String result;

		errorMessageID = GetLastError();
		if (!errorMessageID)
			return String_Empty;

		messageBuffer = NULL;
		messageLength = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorMessageID,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&messageBuffer,
			0,
			NULL
		);

		result = String_FromUtf16(messageBuffer, messageLength);

		LocalFree(messageBuffer);

		return result;
	}
#endif

static void AppendFile(FileInfo **files, Int *len, Int *max,
	String name, UInt64 size, UInt32 mode, UInt16 year, Byte month, Byte day, Byte hour, Byte minute)
{
	// Create a new FileInfo object for the data.
	FileInfo fileInfo = GC_MALLOC_STRUCT(struct FileInfoStruct);
	if (fileInfo == NULL)
		Smile_Abort_OutOfMemory();

	fileInfo->name = name;
	fileInfo->nameCaseFolded = String_CaseFold(name);
	fileInfo->size = size;
	fileInfo->mode = mode;
	fileInfo->year = year;
	fileInfo->month = month;
	fileInfo->day = day;
	fileInfo->hour = hour;
	fileInfo->minute = minute;

	// If we've run out of space, double the array.
	if (*len >= *max) {
		Int newMax = *max * 2;
		FileInfo *newFiles = GC_MALLOC_STRUCT_ARRAY(FileInfo, newMax);
		MemCpy(newFiles, *files, *len * sizeof(FileInfo));
		*max = newMax;
		*files = newFiles;
	}

	// Add the new file to the array.
	(*files)[(*len)++] = fileInfo;
}

static void GetTodaysDateAsFileInfo(FileInfo fileInfo)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		SYSTEMTIME systemTime;
		GetLocalTime(&systemTime);
		fileInfo->year = systemTime.wYear;
		fileInfo->month = (Byte)systemTime.wMonth;
		fileInfo->day = (Byte)systemTime.wDay;
		fileInfo->hour = (Byte)systemTime.wHour;
		fileInfo->minute = (Byte)systemTime.wMinute;

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		time_t now = time(NULL);
		struct tm tm;

		localtime_r(&now, &tm);

		fileInfo->year = tm.tm_year + 1900;
		fileInfo->month = tm.tm_mon + 1;
		fileInfo->day = tm.tm_mday;
		fileInfo->hour = tm.tm_hour;
		fileInfo->minute = tm.tm_min;

#	else
#		error Unsupported OS.
#	endif
}

static FileInfo *GetRawFileList(Bool allMode, Bool longMode, Int *numFiles)
{
	Int max;
	Int len;
	FileInfo *files;
	
	max = 64;
	len = 0;
	files = GC_MALLOC_STRUCT_ARRAY(FileInfo, max);
	if (files == NULL)
		Smile_Abort_OutOfMemory();

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
	{
		WIN32_FIND_DATAW findData;
		HANDLE findFileHandle;
		String name;
		UInt32 mode;
		SYSTEMTIME systemTime;
		FILETIME localFileTime;

		String exeExtension = String_FromC(".exe");
		String batExtension = String_FromC(".bat");
		String cmdExtension = String_FromC(".cmd");
		String msiExtension = String_FromC(".msi");

		findFileHandle = FindFirstFileW(L"*", &findData);
		if (findFileHandle == NULL) {
			printf("Error: %s", String_ToC(GetLastErrorString()));
			*numFiles = 0;
			return files;
		}

		if (allMode || !(findData.cFileName[0] == '.' || (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))) {
			name = String_FromUtf16(findData.cFileName, wcslen(findData.cFileName));

			mode = 0644;
			if (String_EndsWithI(name, exeExtension) || String_EndsWithI(name, batExtension)
				|| String_EndsWithI(name, cmdExtension) || String_EndsWithI(name, msiExtension))
				mode |= FILE_MODE_OWNER_EXEC | FILE_MODE_GROUP_EXEC | FILE_MODE_WORLD_EXEC;
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				mode |= FILE_TYPE_DIR | FILE_MODE_OWNER_EXEC | FILE_MODE_GROUP_EXEC | FILE_MODE_WORLD_EXEC;
			else mode |= FILE_TYPE_REG;
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) mode &= ~0222;

			FileTimeToSystemTime(&findData.ftLastWriteTime, &systemTime);

			AppendFile(&files, &len, &max,
				name,
				(UInt64)findData.nFileSizeLow | ((UInt64)findData.nFileSizeHigh << 32),
				mode,
				systemTime.wYear, (Byte)systemTime.wMonth, (Byte)systemTime.wDay,
				(Byte)systemTime.wHour, (Byte)systemTime.wMinute);
		}

		while (FindNextFileW(findFileHandle, &findData)) {
			if (allMode || !(findData.cFileName[0] == '.' || (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))) {
				name = String_FromUtf16(findData.cFileName, wcslen(findData.cFileName));

				mode = 0644;
				if (String_EndsWithI(name, exeExtension) || String_EndsWithI(name, batExtension)
					|| String_EndsWithI(name, cmdExtension) || String_EndsWithI(name, msiExtension))
					mode |= FILE_MODE_OWNER_EXEC | FILE_MODE_GROUP_EXEC | FILE_MODE_WORLD_EXEC;
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					mode |= FILE_TYPE_DIR | FILE_MODE_OWNER_EXEC | FILE_MODE_GROUP_EXEC | FILE_MODE_WORLD_EXEC;
				else mode |= FILE_TYPE_REG;
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) mode &= ~0222;

				FileTimeToLocalFileTime(&findData.ftLastWriteTime, &localFileTime);
				FileTimeToSystemTime(&localFileTime, &systemTime);

				AppendFile(&files, &len, &max,
					name,
					(UInt64)findData.nFileSizeLow | ((UInt64)findData.nFileSizeHigh << 32),
					mode,
					systemTime.wYear, (Byte)systemTime.wMonth, (Byte)systemTime.wDay,
					(Byte)systemTime.wHour, (Byte)systemTime.wMinute);
			}
		}

		FindClose(findFileHandle);
	}

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)

	{
		DIR *dir;
		struct dirent *dirent;
		struct stat statbuf;
		String name;
		struct tm tm;

		dir = opendir(".");
		if (dir == NULL) {
			printf("Error opening directory: %s", String_ToC(Smile_Unix_GetErrorString(errno)));
			*numFiles = 0;
			return files;
		}

		while ((dirent = readdir(dir)) != NULL) {

			if (allMode || !(dirent->d_name[0] == '.')) {
				if (lstat(dirent->d_name, &statbuf)) {
					printf("Error stat'ing file: %s", String_ToC(Smile_Unix_GetErrorString(errno)));
					*numFiles = 0;
					return files;
				}

				name = String_FromC(dirent->d_name);

				localtime_r(&statbuf.st_mtime, &tm);

				AppendFile(&files, &len, &max,
					name,
					statbuf.st_size,
					statbuf.st_mode,
					tm.tm_year + 1900, (Byte)(tm.tm_mon + 1), (Byte)tm.tm_mday,
					(Byte)tm.tm_hour, (Byte)tm.tm_min);
			}
			
		}

		closedir(dir);
	}

#	else
#		error Unsupported OS.
#	endif

	if (numFiles != NULL)
		*numFiles = len;
	return files;
}

static Int *ComputeColumnWidths(FileInfo *files, Int numFiles, Int numRows, Int *numColumnsResult)
{
	Int numColumns;
	Int *widths;
	Int i, j, jend;
	Int columnWidth;
	Int nameLength;

	if (numRows <= 0) {
		if (numColumnsResult != NULL) {
			*numColumnsResult = 0;
		}
		return NULL;
	}

	numColumns = (numFiles + (numRows - 1)) / numRows;
	widths = (Int *)GC_MALLOC_ATOMIC(sizeof(Int) * numColumns);
	for (i = 0; i < numColumns; i++) {
		columnWidth = 0;
		jend = (i + 1) * numRows;
		if (jend > numFiles) jend = numFiles;
		for (j = i * numRows; j < jend; j++) {
			nameLength = String_Length(files[j]->name);
			if (nameLength > columnWidth) columnWidth = nameLength;
		}
		widths[i] = columnWidth;
	}

	if (numColumnsResult != NULL) {
		*numColumnsResult = numColumns;
	}

	return widths;
}

static Int ComputeTotalWidth(FileInfo *files, Int numFiles, Int numRows, Int padding)
{
	Int numColumns;
	Int *widths = ComputeColumnWidths(files, numFiles, numRows, &numColumns);
	Int totalWidth;
	Int i;

	totalWidth = 0;
	for (i = 0; i < numColumns; i++) {
		totalWidth += widths[i] + padding;
	}
	return totalWidth;
}

static Int FindOptimalRowCount(FileInfo *files, Int numFiles, Int width, Int padding)
{
	Int averageLength;
	Int numColumns;
	Int numRows;
	Int i;
	Int currentWidth;

	if (!numFiles) return 0;

	// Compute the average length of the names, which is just the sum of the lengths divided
	// by the count.  We use a count/2 trick to round the result up or down, toward the nearest
	// integer, without relying on (slow!) floating point.
	averageLength = 0;
	for (i = 0; i < numFiles; i++) {
		averageLength += String_Length(files[i]->name);
	}
	averageLength = (averageLength + numFiles / 2) / numFiles;

	// Make reasonable initial guesses at the number of columns and rows.
	numColumns = width / (averageLength + padding);	// Optimal column count is probably near this.
	numRows = (numFiles + (numColumns - 1)) / numColumns;	// Initial guess of row count.

	// Determine how wide the display would be at the given number of rows.
	currentWidth = ComputeTotalWidth(files, numFiles, numRows, padding);

	// See how close we got.
	if (currentWidth == width) {
		// Nailed it!
		return numRows;
	}
	else if (currentWidth > width) {
		// Initial guess was too wide, so try more rows until we find the optimal width.
		while (currentWidth > width && numRows < numFiles) {
			numRows++;
			currentWidth = ComputeTotalWidth(files, numFiles, numRows, padding);
		}
		return numRows;
	}
	else {
		// Initial guess was too narrow, so try fewer rows until we find the optimal width.
		while (numRows > 1) {
			Int nextWidth;

			numRows--;
			nextWidth = ComputeTotalWidth(files, numFiles, numRows, padding);
			if (nextWidth >= width) {
				numRows++;
				break;
			}
		}
		return numRows;
	}
}

static int CompareFilesForSorting(const void *a, const void *b)
{
	FileInfo aInfo = *(FileInfo *)a;
	FileInfo bInfo = *(FileInfo *)b;

	return (int)String_Compare(aInfo->nameCaseFolded, bInfo->nameCaseFolded);
}

static void ListFilesMultiColumnMode(FileInfo *files, Int numFiles, Int consoleWidth, Bool typeMode)
{
	Int numColumns, numRows;
	Int *columnWidths;
	String name, line;
	Int row, column;
	Int index, i;
	const char *color;
	char typeChar;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	printf_styled("\033[0;37;40m");

	// Tack on suffixes, if we need them.
	if (typeMode) {
		for (i = 0; i < numFiles; i++) {
			name = files[i]->name;
			switch (files[i]->mode & FILE_TYPE_MASK) {
				case FILE_TYPE_DIR: name = String_ConcatByte(name, '/'); break;
				case FILE_TYPE_CHR: break;
				case FILE_TYPE_BLK: break;
				case FILE_TYPE_LNK: name = String_ConcatByte(name, '@'); break;
				case FILE_TYPE_SOCK: break;
				case FILE_TYPE_FIFO: break;
				default: name = (files[i]->mode & 0111 ? String_ConcatByte(name, '*') : name); break;
			}
			files[i]->name = name;
		}
	}

	// Find out how many rows of files there really should be.
	numRows = FindOptimalRowCount(files, numFiles, consoleWidth, 2);

	// Find out how big that means each column must be.
	columnWidths = ComputeColumnWidths(files, numFiles, numRows, &numColumns);

	// Render the files as a multi-column list of names.
	for (row = 0; row < numRows; row++) {

		StringBuilder_SetLength(stringBuilder, 0);

		// Render this column to the StringBuilder, padded to the column width,
		// and with trailing space if it's not the last column.
		for (column = 0; column < numColumns; column++) {
			index = column * numRows + row;
			if (index >= numFiles) continue;
			name = files[index]->name;
			switch (files[index]->mode & FILE_TYPE_MASK) {
				case FILE_TYPE_DIR: typeChar = 'd'; break;
				case FILE_TYPE_CHR: typeChar = 'c'; break;
				case FILE_TYPE_BLK: typeChar = 'b'; break;
				case FILE_TYPE_LNK: typeChar = 'l'; break;
				case FILE_TYPE_SOCK: typeChar = 's'; break;
				case FILE_TYPE_FIFO: typeChar = 'f'; break;
				default: typeChar = ' '; break;
			}
			color = typeChar == 'd' ? "\033[0;37;44;1m"
				: typeChar == 'l' ? "\033[0;36m"
				: (files[index]->mode & (FILE_MODE_OWNER_EXEC | FILE_MODE_GROUP_EXEC | FILE_MODE_WORLD_EXEC)) ? "\033[0;32m"
				: "\033[0;37m";
			StringBuilder_Append(stringBuilder, (const Byte *)color, 0, StrLen(color));
			StringBuilder_AppendString(stringBuilder, name);
			StringBuilder_Append(stringBuilder, (const Byte *)"\033[0;37;40m", 0, 10);
			StringBuilder_AppendRepeat(stringBuilder, ' ', columnWidths[column] - String_Length(name));
			if (column < numColumns - 1) {
				StringBuilder_AppendRepeat(stringBuilder, ' ', 2);
			}
		}

		// Add a newline at the end.
		StringBuilder_AppendByte(stringBuilder, '\n');
		line = StringBuilder_ToString(stringBuilder);
		
		// Write it to stdout.
		fwrite_styled(String_GetBytes(line), 1, String_Length(line), stdout);
	}
}

static void ListFilesLongMode(FileInfo *files, Int numFiles, Bool typeMode)
{
	struct FileInfoStruct todaysDate;
	Byte lastMonth;
	UInt16 lastMonthsYear;
	Byte thisMonth;
	UInt16 thisMonthsYear;
	Int i;
	char typeChar;
	char dateBuffer[32];
	char *suffix;
	const char *color;

	static char *months[] = {
		"",
		"Jan", "Feb", "Mar", "Apr",
		"May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec",
	};

	GetTodaysDateAsFileInfo(&todaysDate);

	thisMonth = todaysDate.month;
	thisMonthsYear = todaysDate.year;
	lastMonth = todaysDate.month;
	lastMonthsYear = todaysDate.year;

	if (--lastMonth == 0) {
		lastMonth = 12;
		lastMonthsYear--;
	}

	printf_styled("\033[0;37;40m");

	for (i = 0; i < numFiles; i++) {
		switch (files[i]->mode & FILE_TYPE_MASK) {
			case FILE_TYPE_DIR: typeChar = 'd'; suffix = "/"; break;
			case FILE_TYPE_CHR: typeChar = 'c'; suffix = ""; break;
			case FILE_TYPE_BLK: typeChar = 'b'; suffix = ""; break;
			case FILE_TYPE_LNK: typeChar = 'l'; suffix = " ->"; break;
			case FILE_TYPE_SOCK: typeChar = 's'; suffix = ""; break;
			case FILE_TYPE_FIFO: typeChar = 'f'; suffix = ""; break;
			default: typeChar = ' '; suffix = (files[i]->mode & 0111 ? "*" : ""); break;
		}

		if (!typeMode) suffix = "";

		if ((files[i]->year == thisMonthsYear && files[i]->month == thisMonth)
			|| (files[i]->month == lastMonthsYear && files[i]->month == lastMonth)) {
			// Recent date: This month or last month, so show month/day/hour/minute.
			sprintf(dateBuffer, "%s %2u %2u:%02u",
				files[i]->month < 13 ? months[files[i]->month] : "???", files[i]->day,
				files[i]->hour, files[i]->minute);
		}
		else {
			// Longer ago, so show the month/day/year.
			sprintf(dateBuffer, "%s %2u %5u",
				files[i]->month < 13 ? months[files[i]->month] : "???", files[i]->day, files[i]->year);
		}

		color = typeChar == 'd' ? "\033[0;37;44;1m"
			: typeChar == 'l' ? "\033[0;36m"
			: (files[i]->mode & (FILE_MODE_OWNER_EXEC | FILE_MODE_GROUP_EXEC | FILE_MODE_WORLD_EXEC)) ? "\033[0;32m"
			: "\033[0m";

		printf_styled("%c%c%c%c%c%c%c%c%c%c %7llu  %s  %s%s%s\033[0;37;40m\n",
			typeChar,
			files[i]->mode & FILE_MODE_OWNER_READ ? 'r' : '-',
			files[i]->mode & FILE_MODE_OWNER_WRITE ? 'w' : '-',
			files[i]->mode & FILE_MODE_OWNER_EXEC ? 'x' : '-',
			files[i]->mode & FILE_MODE_GROUP_READ ? 'r' : '-',
			files[i]->mode & FILE_MODE_GROUP_WRITE ? 'w' : '-',
			files[i]->mode & FILE_MODE_GROUP_EXEC ? 'x' : '-',
			files[i]->mode & FILE_MODE_WORLD_READ ? 'r' : '-',
			files[i]->mode & FILE_MODE_WORLD_WRITE ? 'w' : '-',
			files[i]->mode & FILE_MODE_WORLD_EXEC ? 'x' : '-',
			files[i]->size,
			dateBuffer,
			color,
			String_ToC(files[i]->name),
			suffix
		);
	}
}

void ListFiles(String commandLine, Bool longMode, Int consoleWidth)
{
	FileInfo *files;
	Int numFiles;
	Bool allMode = False;
	Bool typeMode = True;
	SmileList argCell, args;
	String path = NULL;
	
	// Break the command-line string into arguments.
	args = String_SplitCommandLine(commandLine);

	// Process each of the arguments into the command-line options.
	for (argCell = args; SMILE_KIND(argCell) != SMILE_KIND_NULL; argCell = LIST_REST(argCell)) {
		String arg = (String)(argCell->a);
		if (String_Length(arg) > 0 && String_At(arg, 0) == '-') {
			const Byte *text = String_GetBytes(arg) + 1;
			Byte ch;
			while ((ch = *text++)) {
				switch (ch) {
					// A handful of more-or-less GNU-ls-compatible switches.
					case 'l': longMode = True; break;
					case 'a': allMode = True; break;
					case 'f': typeMode = False; break;
					case 'F': typeMode = True; break;
					default:
						printf("ls: Unknown argument '-%c'.\n", ch);
						return;
				}
			}
		}
		else {
			if (path == NULL) path = arg;
			else {
				printf("ls: Too many paths given; only one is allowed.\n");
				return;
			}
		}
	}

	files = GetRawFileList(allMode, longMode, &numFiles);

	qsort(files, numFiles, sizeof(FileInfo), CompareFilesForSorting);

	if (longMode) {
		ListFilesLongMode(files, numFiles, typeMode);
	}
	else {
		ListFilesMultiColumnMode(files, numFiles, consoleWidth, typeMode);
	}
}
