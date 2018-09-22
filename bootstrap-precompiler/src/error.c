
#include "stdafx.h"

void Error(const char *filename, Int line, const char *format, ...)
{
	va_list v;
	va_start(v, format);

	if (filename != NULL) {
		if (line > 0) {
			fprintf(stderr, "%s:%d: ", filename, (int)line);
		}
		else {
			fprintf(stderr, "%s: ", filename);
		}
	}

	vfprintf(stderr, format, v);
	fprintf(stderr, "\n");

	va_end(v);
}

Bool PrintParseMessages(Parser parser)
{
	SmileList list;
	ParseMessage parseMessage;
	Bool hasErrors;
	Bool shouldPrint;
	const char *prefix;
	LexerPosition position;
	String message;

	hasErrors = False;

	for (list = parser->firstMessage; SMILE_KIND(list) != SMILE_KIND_NULL; list = LIST_REST(list)) {
		parseMessage = (ParseMessage)LIST_FIRST(list);

		shouldPrint = False;
		prefix = "";

		switch (parseMessage->messageKind) {
			case PARSEMESSAGE_INFO:
				break;

			case PARSEMESSAGE_WARNING:
				shouldPrint = True;
				prefix = "warning: ";
				hasErrors = True;
				break;

			case PARSEMESSAGE_ERROR:
				shouldPrint = True;
				prefix = "";
				hasErrors = True;
				break;
		}
	
		if (!shouldPrint) continue;
	
		position = parseMessage->position;
		if (position->filename != NULL) {
			if (position->line > 0) {
				// Have a filename and a line number.
				message = String_Format("%S:%d: %s%S\r\n", position->filename, position->line, prefix, parseMessage->message);
			}
			else {
				// Have a filename but no line number.
				message = String_Format("%S: %s%S\r\n", position->filename, prefix, parseMessage->message);
			}
		}
		else {
			// Have no filename.
			message = String_Format("smile: %s%S\r\n", prefix, parseMessage->message);
		}
		
		fwrite(String_GetBytes(message), 1, String_Length(message), stderr);
	}

	return hasErrors;
}
