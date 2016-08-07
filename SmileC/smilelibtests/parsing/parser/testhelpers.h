
#ifndef __SMILELIBTESTS_PARSING_PARSER_TESTHELPERS_H__
#define __SMILELIBTESTS_PARSING_PARSER_TESTHELPERS_H__

SmileObject SimpleParse(const char *input);
SmileList FullParse(const char *input);
Bool RecursiveEquals(SmileObject a, SmileObject b);
Lexer SetupLexerFromString(String source);
Lexer SetupLexer(const char *string);

#endif
