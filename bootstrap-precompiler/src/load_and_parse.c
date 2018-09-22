
#include "stdafx.h"
#include "load_and_parse.h"
#include "error.h"

String LoadFile(String filename)
{
	FILE *fp;
	StringBuilder stringBuilder;
	Byte *buffer;
	size_t readLength;

	const int ReadLength = 0x10000;	// Read 64K at a time.

	if ((fp = fopen(String_ToC(filename), "rb")) == NULL) {
		Error("smile", 0, "Cannot open \"%s\" for reading.", String_ToC(filename));
		return NULL;
	}

	stringBuilder = StringBuilder_Create();

	buffer = GC_MALLOC_ATOMIC(ReadLength);
	if (buffer == NULL)
		Smile_Abort_OutOfMemory();

	while ((readLength = fread(buffer, 1, ReadLength, fp)) > 0) {
		StringBuilder_Append(stringBuilder, buffer, 0, readLength);
	}

	fclose(fp);

	return StringBuilder_ToString(stringBuilder);
}

SmileObject Parse(String text, String filename, ClosureInfo closureInfo)
{
	Lexer lexer;
	Parser parser;
	ParseScope globalScope;
	SmileObject expr;

	globalScope = ParseScope_CreateRoot();
	ParseScope_DeclareVariablesFromClosureInfo(globalScope, closureInfo);

	lexer = Lexer_Create(text, 0, String_Length(text), Path_Resolve(Path_GetCurrentDir(), filename), 1, 1);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();

	expr = Parser_Parse(parser, lexer, globalScope);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(parser);
		if (hasErrors) return NULL;
	}

	return expr;
}
