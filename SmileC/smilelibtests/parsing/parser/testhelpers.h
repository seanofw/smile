
#ifndef __SMILELIBTESTS_PARSING_PARSER_TESTHELPERS_H__
#define __SMILELIBTESTS_PARSING_PARSER_TESTHELPERS_H__

String Stringify(SmileObject obj);
const char *StringifyToC(SmileObject obj);

Bool IsRegularList(SmileObject list);
Bool ContainsNestedList(SmileObject obj);

SmileObject SimpleParse(const char *input);
Bool RecursiveEquals(SmileObject a, SmileObject b);
Lexer SetupLexerFromString(String source);
Lexer SetupLexer(const char *string);

#endif
