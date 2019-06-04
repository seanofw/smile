//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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

#include <smile/parsing/parsemessage.h>
#include <smile/smiletypes/easyobject.h>

SMILE_EASY_OBJECT_VTABLE(ParseMessage);

SMILE_EASY_OBJECT_COMPARE(ParseMessage, SMILE_KIND_PARSEMESSAGE,
	a->messageKind == b->messageKind
	&& LexerPosition_Equals(a->position, b->position)
	&& String_Equals(a->message, b->message));
SMILE_EASY_OBJECT_DEEP_COMPARE(ParseMessage, SMILE_KIND_PARSEMESSAGE,
	a->messageKind == b->messageKind
	&& LexerPosition_Equals(a->position, b->position)
	&& String_Equals(a->message, b->message));
SMILE_EASY_OBJECT_HASH(ParseMessage, obj->position->line * 100 + obj->position->column);

SMILE_EASY_OBJECT_READONLY_SECURITY(ParseMessage);
SMILE_EASY_OBJECT_NO_PROPERTIES(ParseMessage);
SMILE_EASY_OBJECT_NO_CALL(ParseMessage, "A ParseMessage object");
SMILE_EASY_OBJECT_NO_SOURCE(ParseMessage);
SMILE_EASY_OBJECT_NO_UNBOX(ParseMessage)

SMILE_EASY_OBJECT_TOBOOL(ParseMessage, True);

static String ParseMessage_ToString(ParseMessage parseMessage, SmileUnboxedData unboxedData)
{
	LexerPosition position = parseMessage->position;
	String message;
	const char *prefix;

	UNUSED(unboxedData);

	switch (parseMessage->messageKind) {

		case PARSEMESSAGE_WARNING:
			prefix = "Warning: ";
			break;
		case PARSEMESSAGE_ERROR:
			prefix = "Error: ";
			break;
		case PARSEMESSAGE_FATAL:
			prefix = "Fatal: ";
			break;
		default:
			prefix = "";
			break;
	}

	if (position != NULL && position->filename != NULL) {
		if (position->line > 0) {
			// Have a filename and a line number.
			message = String_Format("%s%S:%d: %S", prefix, position->filename, position->line, parseMessage->message);
		}
		else {
			// Have a filename but no line number.
			message = String_Format("%s%S: %S", prefix, position->filename, parseMessage->message);
		}
	}
	else {
		// Have no filename.
		message = String_Format("%s%S", prefix, parseMessage->message);
	}

	return message;
}