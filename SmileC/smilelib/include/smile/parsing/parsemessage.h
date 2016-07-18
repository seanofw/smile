
#ifndef __SMILE_PARSING_PARSEMESSAGE_H__
#define __SMILE_PARSING_PARSEMESSAGE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif
#ifndef __SMILE_PARSING_LEXER_H__
#include <smile/parsing/lexer.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Parse-message kinds.

#define PARSEMESSAGE_INFO		0		// A helpful informational message
#define PARSEMESSAGE_WARNING	1		// A warning about possible undesirable behavior
#define PARSEMESSAGE_ERROR		2		// An error in source code
#define PARSEMESSAGE_FATAL		3		// A fatal error that has caused the parser to abort

//-------------------------------------------------------------------------------------------------
//  Parse messages.

/// <summary>
/// This describes the shape of a single parser output message.
/// </summary>
struct ParseMessageStruct {

	DECLARE_BASE_OBJECT_PROPERTIES;

	// What kind of message this is (see PARSEMESSAGE_*).
	Int messageKind;

	// Where in the source code this parser message was generated.
	LexerPosition position;

	// The text of the message itself.
	String message;

};

SMILE_API_DATA SmileVTable ParseMessage_VTable;

//-------------------------------------------------------------------------------------------------
//  Functions.

/// <summary>
/// Simple helper function to create ParseMessage structs.
/// </summary>
Inline ParseMessage ParseMessage_Create(Int messageKind, LexerPosition position, String message)
{
	ParseMessage parseMessage = GC_MALLOC_STRUCT(struct ParseMessageStruct);

	parseMessage->kind = SMILE_KIND_PARSEMESSAGE;
	parseMessage->base = Smile_KnownObjects.Object;
	parseMessage->vtable = ParseMessage_VTable;
	parseMessage->assignedSymbol = 0;

	parseMessage->messageKind = messageKind;
	parseMessage->position = position;
	parseMessage->message = message;

	return parseMessage;
}

#endif
