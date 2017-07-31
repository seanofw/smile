#ifndef __SMILE_PARSING_PARSEINCLUDE_H__
#define __SMILE_PARSING_PARSEINCLUDE_H__

#ifndef __SMILE_PARSING_PARSER_H__
#include <smile/parsing/parser.h>
#endif

struct LibraryInfoStruct {
	String name;
	ClosureInfo globalClosureInfo;

	ParseMessage *parseMessages;
	Int numParseMessages;

	Bool loadedSuccessfully;
};

SMILE_API_FUNC LibraryInfo LibraryInfo_Create(String name, Bool loadedSuccessfully, ClosureInfo globalClosureInfo,
	ParseMessage *parseMessages, Int numParseMessages);
SMILE_API_FUNC SmileObject LibraryInfo_ExposeAll(LibraryInfo libraryInfo, Parser parser, ParseScope target);
SMILE_API_FUNC SmileObject LibraryInfo_ExposeOne(LibraryInfo libraryInfo, Parser parser, ParseScope target, Symbol oldName, Symbol newName);

SMILE_API_FUNC LibraryInfo Stdio_Main(void);

#endif