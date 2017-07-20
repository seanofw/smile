// Yasuhiro Matsumoto's (mattn's) "ansicolor-w32.c" library,
// found at https://github.com/mattn/ansicolor-w32.c .
// It is covered under the MIT open-source license.

#ifdef _WIN32
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

#ifndef FOREGROUND_MASK
#	define FOREGROUND_MASK (FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY)
#endif
#ifndef BACKGROUND_MASK
#	define BACKGROUND_MASK (BACKGROUND_RED|BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_INTENSITY)
#endif

HANDLE _get_osfhandle(int fileno);

typedef struct ConsoleStateStruct {
	CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	WORD attr, attrOld;
} *ConsoleState;

static void SetMode_SetColumns(HANDLE handle, int numColumns)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD written, csize;
	COORD coord;
	int width, height;

	GetConsoleScreenBufferInfo(handle, &csbi);

	width = csbi.dwSize.X;
	height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	csize = width * (height + 1);
	coord.X = 0;
	coord.Y = csbi.srWindow.Top;

	FillConsoleOutputCharacter(handle, ' ', csize, coord, &written);
	FillConsoleOutputAttribute(handle, csbi.wAttributes, csize, coord, &written);

	SetConsoleCursorPosition(handle, csbi.dwCursorPosition);

	csbi.dwSize.X = numColumns;
	SetConsoleScreenBufferSize(handle, csbi.dwSize);

	csbi.srWindow.Right = csbi.srWindow.Left + (numColumns - 1);
	SetConsoleWindowInfo(handle, TRUE, &csbi.srWindow);
}

static void SetMode_ReverseVideo(HANDLE handle, ConsoleState consoleState)
{
	consoleState->attr =
		((consoleState->attr & FOREGROUND_MASK) << 4) |
		((consoleState->attr & BACKGROUND_MASK) >> 4);

	SetConsoleTextAttribute(handle, consoleState->attr);
}

static void SetMode_ShowCursor(HANDLE handle, BOOL shown)
{
	CONSOLE_CURSOR_INFO cci;

	GetConsoleCursorInfo(handle, &cci);
	cci.bVisible = shown;
	SetConsoleCursorInfo(handle, &cci);
}

static void SetMode_MoveCursor(HANDLE handle, int x, int y)
{
	COORD coord;

	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(handle, coord);
}

static void SetColors(HANDLE handle, ConsoleState consoleState, int *values, int numValues)
{
	int i;
	WORD attr = consoleState->attrOld;

	for (i = 0; i <= numValues; i++) {
		if (values[i] == -1 || values[i] == 0)
			attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		else if (values[i] == 1)
			attr |= FOREGROUND_INTENSITY;
		else if (values[i] == 4)
			attr |= FOREGROUND_INTENSITY;
		else if (values[i] == 5)
			attr |= FOREGROUND_INTENSITY;
		else if (values[i] == 7)
			attr =
				((attr & FOREGROUND_MASK) << 4) |
				((attr & BACKGROUND_MASK) >> 4);
		else if (values[i] == 10)
			; // symbol on
		else if (values[i] == 11)
			; // symbol off
		else if (values[i] == 22)
			attr &= ~FOREGROUND_INTENSITY;
		else if (values[i] == 24)
			attr &= ~FOREGROUND_INTENSITY;
		else if (values[i] == 25)
			attr &= ~FOREGROUND_INTENSITY;
		else if (values[i] == 27)
			attr =
				((attr & FOREGROUND_MASK) << 4) |
				((attr & BACKGROUND_MASK) >> 4);
		else if (values[i] >= 30 && values[i] <= 37) {
			attr = (attr & (BACKGROUND_MASK | FOREGROUND_INTENSITY));
			if ((values[i] - 30) & 1)
				attr |= FOREGROUND_RED;
			if ((values[i] - 30) & 2)
				attr |= FOREGROUND_GREEN;
			if ((values[i] - 30) & 4)
				attr |= FOREGROUND_BLUE;
		}
		//else if (v[i] == 39)
		//	attr = (~attr & BACKGROUND_MASK);
		else if (values[i] >= 40 && values[i] <= 47) {
			attr = (attr & (FOREGROUND_MASK | BACKGROUND_INTENSITY));
			if ((values[i] - 40) & 1)
				attr |= BACKGROUND_RED;
			if ((values[i] - 40) & 2)
				attr |= BACKGROUND_GREEN;
			if ((values[i] - 40) & 4)
				attr |= BACKGROUND_BLUE;
		}
		//else if (v[i] == 49)
		//	attr = (~attr & FOREGROUND_MASK);
		else if (values[i] == 100)
			attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	}

	SetConsoleTextAttribute(handle, attr);

	consoleState->attrOld = attr;
}

static void ClearLine(HANDLE handle, ConsoleState consoleState, int *values, int numValues)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD coord;
	DWORD written, csize;

	GetConsoleScreenBufferInfo(handle, &csbi);
	coord = csbi.dwCursorPosition;

	switch (values[0]) {
		default:
		case 0:
			csize = csbi.dwSize.X - coord.X;
			break;
		case 1:
			csize = coord.X;
			coord.X = 0;
			break;
		case 2:
			csize = csbi.dwSize.X;
			coord.X = 0;
			break;
	}

	FillConsoleOutputCharacter(handle, ' ', csize, coord, &written);
	FillConsoleOutputAttribute(handle, csbi.wAttributes, csize, coord, &written);

	SetConsoleCursorPosition(handle, csbi.dwCursorPosition);
}

static void ClearScreen(HANDLE handle, ConsoleState consoleState, int *values, int numValues)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD coord;
	DWORD written, csize;
	int width, height;

	GetConsoleScreenBufferInfo(handle, &csbi);

	width = csbi.dwSize.X;
	height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	coord = csbi.dwCursorPosition;

	switch (values[0]) {
		default:
		case 0:
			csize = width * (height - coord.Y) - coord.X;
			coord.X = 0;
			break;
		case 1:
			csize = width * coord.Y + coord.X;
			coord.X = 0;
			coord.Y = csbi.srWindow.Top;
			break;
		case 2:
			csize = width * (height + 1);
			coord.X = 0;
			coord.Y = csbi.srWindow.Top;
			break;
	}

	FillConsoleOutputCharacter(handle, ' ', csize, coord, &written);
	FillConsoleOutputAttribute(handle, csbi.wAttributes, csize, coord, &written);

	SetConsoleCursorPosition(handle, csbi.dwCursorPosition);
}

static void MoveCursor(HANDLE handle, ConsoleState consoleState, int *values, int numValues)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD coord;

	GetConsoleScreenBufferInfo(handle, &csbi);

	coord = csbi.dwCursorPosition;

	if (values[0] != -1) {
		if (values[1] != -1) {
			coord.Y = csbi.srWindow.Top + values[0] - 1;
			coord.X = values[1] - 1;
		}
		else
			coord.X = values[0] - 1;
	}
	else {
		coord.X = 0;
		coord.Y = csbi.srWindow.Top;
	}

	if (coord.X < csbi.srWindow.Left)
		coord.X = csbi.srWindow.Left;
	else if (coord.X > csbi.srWindow.Right)
		coord.X = csbi.srWindow.Right;

	if (coord.Y < csbi.srWindow.Top)
		coord.Y = csbi.srWindow.Top;
	else if (coord.Y > csbi.srWindow.Bottom)
		coord.Y = csbi.srWindow.Bottom;

	SetConsoleCursorPosition(handle, coord);
}

static void ProcessAnsiEscapeCode(HANDLE handle, ConsoleState consoleState, char mode, char modifier, int *values, int numValues)
{
	int i;

	switch (mode) {

		case 'h':
			if (modifier == '?') {
				for (i = 0; i <= numValues; i++) {
					switch (values[i]) {
						case 3:
							SetMode_SetColumns(handle, 132);
							break;
						case 5:
							SetMode_ReverseVideo(handle, consoleState);
							break;
						case 25:
							SetMode_ShowCursor(handle, TRUE);
							break;
						case 47:
							SetMode_MoveCursor(handle, 0, 0);
							break;
					}
				}
			}
			else if (modifier == '>' && values[0] == 5) {
				SetMode_ShowCursor(handle, FALSE);
			}
			break;

		case 'l':
			if (modifier == '?') {
				for (i = 0; i <= numValues; i++) {
					switch (values[i]) {
						case 3:
							SetMode_SetColumns(handle, 80);
							break;
						case 5:
							SetMode_ReverseVideo(handle, consoleState);
							break;
						case 25:
							SetMode_ShowCursor(handle, FALSE);
							break;
					}
				}
			}
			else if (modifier == '>' && values[0] == 5) {
				SetMode_ShowCursor(handle, TRUE);
			}
			break;

		case 'm':
			SetColors(handle, consoleState, values, numValues);
			break;

		case 'K':
			ClearLine(handle, consoleState, values, numValues);
			break;

		case 'J':
			ClearScreen(handle, consoleState, values, numValues);
			break;

		case 'H':
		case 'f':
			MoveCursor(handle, consoleState, values, numValues);
			break;
	}
}

static const char *ParseAndExecuteEscapeCode(FILE *fp, const char *ptr, HANDLE handle, ConsoleState consoleState)
{
	char mode;
	int values[8];
	int i, numValues, modifier;

	// Reset the 'values' collection.
	for (i = 0; i < 8; i++) {
		values[i] = -1;
	}
	numValues = 0;

	// Consume the '\e' itself.
	ptr++;

	// Parse the input into a list of  until we reach either the action code or an EOI.
	for (;;) {
		char ch;
		if ((ch = *ptr++) == '\0')
			return ptr - 1;

		if (isdigit(ch)) {
			if (values[numValues] == -1)
				values[numValues] = ch - '0';
			else
				values[numValues] = values[numValues] * 10 + ch - '0';
		}
		else if (ch == '[') {
			continue;
		}
		else if (ch == ';') {
			if (numValues < 7) numValues++;
		}
		else if (ch == '>' || ch == '?') {
			modifier = ch;
		}
		else {
			mode = ch;
			break;
		}
	}

	// Now go do it.
	ProcessAnsiEscapeCode(handle, consoleState, mode, modifier, values, numValues);

	return ptr;
}

int __write_w32(FILE *fp, const char *buf)
{
	static int first = 1;
	HANDLE handle = INVALID_HANDLE_VALUE;
	struct ConsoleStateStruct consoleState;
	const char *ptr = buf;

	if (fp != stdout && fp != stderr) {
		int len = strlen(buf);
		fwrite(buf, 1, len, fp);
		return len;
	}

	handle = (HANDLE)_get_osfhandle(fileno(fp));
	GetConsoleScreenBufferInfo(handle, &consoleState.consoleScreenBufferInfo);

	consoleState.attrOld = consoleState.consoleScreenBufferInfo.wAttributes;

	while (*ptr) {
		if (*ptr == '\033') {
			// Handle this escape code.
			ptr = ParseAndExecuteEscapeCode(fp, ptr, handle, &consoleState);
		}
		else {
			// Find the extent of this sequence of characters that *isn't* escape codes.
			const char *start = ptr;
			while (*ptr != '\033' && *ptr != '\0') ptr++;

			// Write the whole sequence.
			if (ptr > start) {
				fwrite(start, 1, ptr - start, fp);
			}
		}
	}

	return (int)(ptr - buf);
}

int _fprintf_w32(FILE* fp, const char* format, ...)
{
	va_list args;
	int r;
	char *buf = NULL;

	va_start(args, format);
	r = _vscprintf(format, args);
	buf = malloc(sizeof(char) * (r + 1));
	if (buf == NULL) return 0;

	r = vsprintf(buf, format, args);
	va_end(args);

	if (r != -1)
		r = __write_w32(fp, buf);
	if (buf) free(buf);

	return r;
}

int _fputs_w32(FILE* fp, const char* s)
{
	int r = __write_w32(fp, s);
	r += __write_w32(fp, "\n");

	return r;
}

#endif