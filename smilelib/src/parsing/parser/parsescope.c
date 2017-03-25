//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

#include <smile/numeric/real64.h>

static ParseError ParseScope_DeclareGlobalOperators(ParseScope parseScope);

static const char *ParseDecl_Names[] = {
	"an undeclared variable",
	"a primitive",
	"a global variable",
	"a function argument",
	"a local variable",
	"a const value",
	"an auto variable",
	"a postcondition result",
	"a till-loop name",
};

/// <summary>
/// Create a root scope that contains only the eighteen or so global forms.
/// </summary>
/// <returns>The newly-created root scope.</returns>
ParseScope ParseScope_CreateRoot(void)
{
	ParseError error;

	ParseScope parseScope = GC_MALLOC_STRUCT(struct ParseScopeStruct);
	if (parseScope == NULL)
		Smile_Abort_OutOfMemory();

	parseScope->kind = PARSESCOPE_OUTERMOST;
	parseScope->parentScope = NULL;
	parseScope->symbolDict = Int32Int32Dict_Create();
	parseScope->syntaxTable = ParserSyntaxTable_CreateNew();
	parseScope->decls = GC_MALLOC_STRUCT_ARRAY(ParseDecl, 16);
	if (parseScope->decls == NULL)
		Smile_Abort_OutOfMemory();
	parseScope->numDecls = 0;
	parseScope->maxDecls = 16;

	if ((error = ParseScope_DeclareGlobalOperators(parseScope)) != NULL) {
		String errorMessage = String_Format("Unable to declare global operators; this is most likely due to a corrupt memory space. Reported error was: %S", error->message);
		Smile_Abort_FatalError(String_ToC(errorMessage));
	}

	return parseScope;
}

// Helper struct for ParseScope_DeclareVariablesFromClosureInfo, used while walking the VarDict.
struct DeclareVariablesInfo {
	ParseScope scope;
	ParseError error;
};

// Helper function for ParseScope_DeclareVariablesFromClosureInfo, used while walking the VarDict.
static Bool AddVariablesToScope(VarInfo varInfo, void *param)
{
	struct DeclareVariablesInfo *declareVariablesInfo = (struct DeclareVariablesInfo *)param;
	ParseError error;

	error = ParseScope_DeclareHere(declareVariablesInfo->scope, varInfo->symbol, PARSEDECL_GLOBAL, NULL, NULL);
	declareVariablesInfo->error = error;

	return (error == NULL);
}

/// <summary>
/// Declare all of the global variables found in the given ClosureInfo in the given scope.
/// This recursively walks the ClosureInfo to any parent ClosureInfos, declaring everything
/// that the given ClosureInfo can see all the way back to its root.
/// </summary>
/// <param name="scope">The scope in which to declare all the global variables.</param>
/// <param name="closureInfo">A ClosureInfo object that identifies all the names of the global variables to
/// declare (their values will be ignored during parsing).</param>
/// <returns>The first parse error that results from declaring these variables, or NULL if no errors were
/// produced.  (An error could result from a duplicate variable declaration.)</returns>
ParseError ParseScope_DeclareVariablesFromClosureInfo(ParseScope scope, ClosureInfo closureInfo)
{
	ParseError error;
	struct DeclareVariablesInfo declareVariablesInfo;

	declareVariablesInfo.scope = scope;
	declareVariablesInfo.error = NULL;

	if (closureInfo->parent != NULL) {
		error = ParseScope_DeclareVariablesFromClosureInfo(scope, closureInfo->parent);
		if (error != NULL) return error;
	}

	VarDict_ForEach(closureInfo->variableDictionary, AddVariablesToScope, &declareVariablesInfo);

	return declareVariablesInfo.error;
}

/// <summary>
/// Declare the eighteen-or-so global operators in the given (presumably root-level) scope.
/// This also declares 'null', 'true', and 'false'; even though they're not operators,
/// everybody expects them to exist in the earliest stages of the universe.
/// </summary>
/// <param name="parseScope">The scope in which to declare the root-level operators.</param>
/// <returns>True if all were declared successfully, False if any failed to be declared.</returns>
static ParseError ParseScope_DeclareGlobalOperators(ParseScope parseScope)
{
	ParseError error;

	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__SET, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__OPSET, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__IF, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__WHILE, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__TILL, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__FN, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__QUOTE, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__SCOPE, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__PROG1, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__PROGN, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__RETURN, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__CATCH, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__NOT, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__OR, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__AND, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__EQ, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__NE, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__NEW, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__IS, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, SMILE_SPECIAL_SYMBOL__TYPEOF, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
/*
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.null_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.true_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
	if ((error = ParseScope_Declare(parseScope, Smile_KnownSymbols.false_, PARSEDECL_PRIMITIVE, NULL, NULL)) != NULL) return error;
*/
	return NULL;
}

/// <summary>
/// Create a child scope under some given parent scope.
/// </summary>
/// <param name="parentScope">The parent scope under which the new child should be created.</param>
/// <param name="kind">What kind of scope to create (a PARSESCOPE_* value).</param>
/// <returns>The newly-created child scope.</returns>
ParseScope ParseScope_CreateChild(ParseScope parentScope, Int kind)
{
	ParseScope parseScope = GC_MALLOC_STRUCT(struct ParseScopeStruct);
	if (parseScope == NULL)
		Smile_Abort_OutOfMemory();

	parseScope->kind = kind;
	parseScope->parentScope = parentScope;
	parseScope->symbolDict = Int32Int32Dict_Create();
	parseScope->decls = GC_MALLOC_STRUCT_ARRAY(ParseDecl, 16);
	if (parseScope->decls == NULL)
		Smile_Abort_OutOfMemory();
	parseScope->numDecls = 0;
	parseScope->maxDecls = 16;
	ParserSyntaxTable_AddRef(parseScope->syntaxTable = parentScope->syntaxTable);

	return parseScope;
}

/// <summary>
/// This is called by the parser to tell this scope that it is finished,
/// and no new content will be added to it.
/// </summary>
void ParseScope_Finish(ParseScope parseScope)
{
	ParserSyntaxTable_RemoveRef(parseScope->syntaxTable);

	// Allow GC to reclaim the attached objects, if this was the last reference.
	parseScope->syntaxTable = NULL;
	parseScope->symbolDict = NULL;
	parseScope->decls = NULL;
	parseScope->numDecls = 0;
	parseScope->maxDecls = 0;
	parseScope->parentScope = NULL;
}

/// <summary>
/// Declare or redeclare a name in the given scope.
/// </summary>
/// <param name="scope">The scope where the declaration will be created.</param>
/// <param name="symbol">The name to declare.</param>
/// <param name="kind">What kind of thing to declare that name as (one of the PARSEDECL_* values,
/// like ARGUMENT or VARIABLE or CONST).</param>
/// <param name="position">The position in the source code, if known, of this declaration.</param>
/// <param name="decl">This will be filled in with the new ParseDecl object.</param>
/// <returns>Any errors generated by the declaration, or NULL on success.</returns>
ParseError ParseScope_DeclareHere(ParseScope scope, Symbol symbol, Int kind, LexerPosition position, ParseDecl *decl)
{
	ParseDecl parseDecl;
	ParseDecl previousDecl;
	ParseError error;
	Int32 declIndex;

	// First, see if we already declared this.
	if ((previousDecl = ParseScope_FindDeclaration(scope, symbol)) != NULL) {
	
		// Already declared.  This isn't necessarily an error, but it depends on what
		// kind of thing is being declared and how it relates to what was already declared.
		if ((previousDecl->declKind != kind && previousDecl->declKind >= PARSEDECL_UNDECLARED)
			|| previousDecl->declKind >= PARSEDECL_CONST) {

			if (decl != NULL)
				*decl = NULL;
			error = ParseMessage_Create(PARSEMESSAGE_ERROR, position,
				String_Format("Cannot redeclare \"%S\" as %s; it is already declared as %s, on line \"%d\".",
				SymbolTable_GetName(Smile_SymbolTable, symbol),
				ParseDecl_Names[kind],
				ParseDecl_Names[previousDecl->declKind],
				previousDecl->position != NULL ? previousDecl->position->line : 0));
			return error;
		}
	}

	// It's not already declared, so are we allowed to declare a new variable in this scope?  If not, bail.
	if (scope->kind == PARSESCOPE_EXPLICIT) {
		if (decl != NULL)
			*decl = NULL;
		error = ParseMessage_Create(PARSEMESSAGE_ERROR, position,
			String_Format("Cannot add \"%S\" to the current scope, which explicitly lists its variables.",
			SymbolTable_GetName(Smile_SymbolTable, symbol)));
		return error;
	}

	// Construct the base declaration object for this name.
	parseDecl = ParseDecl_Create(symbol, kind, scope->numDecls, position, NULL);

	// Allocate a new index for the declaration.
	if (scope->numDecls >= scope->maxDecls) {
		Int32 newMax = (Int32)scope->maxDecls * 2;
		ParseDecl *newDecls = GC_MALLOC_STRUCT_ARRAY(ParseDecl, newMax);
		if (newDecls == NULL)
			Smile_Abort_OutOfMemory();
		MemCpy(newDecls, scope->decls, sizeof(ParseDecl) * scope->numDecls);
		scope->decls = newDecls;
		scope->maxDecls = newMax;
	}

	// Store the new declaration.
	declIndex = (Int32)scope->numDecls++;
	scope->decls[declIndex] = parseDecl;

	// And record it in the lookup table.
	Int32Int32Dict_SetValue(scope->symbolDict, symbol, declIndex);

	// Last, return the declaration object itself.
	if (decl != NULL)
		*decl = parseDecl;
	return NULL;
}

// scope-vars ::= . names
static ParseError Parser_ParseClassicScopeVariableNames(Parser parser, SmileList *result)
{
	SmileList head = NullList, tail = NullList;
	Token token;
	Symbol name;
	ParseError error;

	for (;;) {
		token = Parser_NextToken(parser);
		switch (token->kind) {

			case TOKEN_BAR:
			case TOKEN_LEFTBRACE:
			case TOKEN_LEFTBRACKET:
			case TOKEN_LEFTPARENTHESIS:
			case TOKEN_RIGHTBRACE:
			case TOKEN_RIGHTBRACKET:
			case TOKEN_RIGHTPARENTHESIS:
				Lexer_Unget(parser->lexer);
				*result = head;
				return NULL;

			case TOKEN_ALPHANAME:
			case TOKEN_UNKNOWNALPHANAME:
			case TOKEN_PUNCTNAME:
			case TOKEN_UNKNOWNPUNCTNAME:
				name = token->data.symbol;
				LIST_APPEND_WITH_SOURCE(head, tail, SmileSymbol_Create(name), Token_GetPosition(token));
				break;

			default:
				error = ParseMessage_Create(PARSEMESSAGE_ERROR, Token_GetPosition(token), String_FromC("Missing name for [$scope] variable."));
				Parser_AddMessage(parser, error);
				break;
		}
	}
}

// term ::= '[' '$scope' . scope-vars exprs-opt ']'
ParseError Parser_ParseClassicScope(Parser parser, SmileObject *result, LexerPosition startPosition)
{
	SmileList variableNames;
	SmileList head, tail;
	SmileList temp;
	ParseError error;
	ParseDecl decl;
	SmileSymbol smileSymbol;

	// Make sure there is a '[' to start the name list.
	if ((error = Parser_ExpectLeftBracket(parser, result, NULL, "$scope", startPosition)) != NULL)
		return error;

	Parser_BeginScope(parser, PARSESCOPE_SCOPEDECL);

	// Parse the names.
	if ((error = Parser_ParseClassicScopeVariableNames(parser, &variableNames)) != NULL) {
		Parser_AddMessage(parser, error);
		variableNames = NullList;
	}

	// Make sure there is a ']' to end the variable-names list.
	if ((error = Parser_ExpectRightBracket(parser, result, NULL, "$scope variables", startPosition)) != NULL) {
		Parser_EndScope(parser);
		return error;
	}

	// Spin over the variable-names list and declare each one in the new parsing scope.
	if (SMILE_KIND(variableNames) == SMILE_KIND_LIST) {
		for (temp = variableNames; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
			smileSymbol = (SmileSymbol)temp->a;
			ParseScope_DeclareHere(parser->currentScope, smileSymbol->symbol, PARSEDECL_VARIABLE, SMILE_VCALL(temp, getSourceLocation), &decl);
		}
	}

	// Now that all of the variables are declared, we mark this as an "explicit" scope, which
	// prohibits any new ones from being added to it.
	parser->currentScope->kind = PARSESCOPE_EXPLICIT;

	// Parse the body expressions.
	head = tail = NullList;
	Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

	// End the scope.
	Parser_EndScope(parser);

	// Make sure there is a ']' to end the scope.
	if ((error = Parser_ExpectRightBracket(parser, result, NULL, "$scope", startPosition)) != NULL)
		return error;

	// Construct the resulting [$scope names exprs] form.
	*result =
		(SmileObject)SmileList_ConsWithSource((SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__SCOPE),
			(SmileObject)SmileList_ConsWithSource((SmileObject)variableNames,
				(SmileObject)head,
			startPosition),
		startPosition);

	return NULL;
}
