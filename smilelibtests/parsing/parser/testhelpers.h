
#ifndef __SMILELIBTESTS_PARSING_PARSER_TESTHELPERS_H__
#define __SMILELIBTESTS_PARSING_PARSER_TESTHELPERS_H__

SmileObject SimpleParse(const char *input);
SmileObject FullParse(const char *input);
Lexer SetupLexerFromString(String source);
Lexer SetupLexer(const char *string);

#define RecursiveEquals(__a__, __b__) \
	(SmileObject_RecursiveEquals((__a__), (__b__)))

#endif
