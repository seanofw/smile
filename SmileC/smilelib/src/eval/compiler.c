//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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

#include <smile/eval/compiler.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuchar.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smileinteger128.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static void Compiler_CompileProperty(Compiler compiler, SmilePair pair, Bool store);
static void Compiler_CompileVariable(Compiler compiler, Symbol symbol, Bool store);
static Bool Compiler_CompileStandardForm(Compiler compiler, Symbol symbol, SmileList args);
static void Compiler_CompileMethodCall(Compiler compiler, SmilePair pair, SmileList args);

static void Compiler_CompileSetf(Compiler compiler, SmileList args);
static void Compiler_CompileOpEquals(Compiler compiler, SmileList args);
static void Compiler_CompileIf(Compiler compiler, SmileList args);
static void Compiler_CompileWhile(Compiler compiler, SmileList args);
static void Compiler_CompileTill(Compiler compiler, SmileList args);
static void Compiler_CompileCatch(Compiler compiler, SmileList args);
static void Compiler_CompileReturn(Compiler compiler, SmileList args);
static void Compiler_CompileFn(Compiler compiler, SmileList args);
static void Compiler_CompileQuote(Compiler compiler, SmileList args);
static void Compiler_CompileProg1(Compiler compiler, SmileList args);
static void Compiler_CompileProgN(Compiler compiler, SmileList args);
static void Compiler_CompileScope(Compiler compiler, SmileList args);
static void Compiler_CompileNew(Compiler compiler, SmileList args);
static void Compiler_CompileIs(Compiler compiler, SmileList args);
static void Compiler_CompileTypeOf(Compiler compiler, SmileList args);
static void Compiler_CompileSuperEq(Compiler compiler, SmileList args);
static void Compiler_CompileSuperNe(Compiler compiler, SmileList args);
static void Compiler_CompileAnd(Compiler compiler, SmileList args);
static void Compiler_CompileOr(Compiler compiler, SmileList args);
static void Compiler_CompileNot(Compiler compiler, SmileList args);

/// <summary>
/// Create a new CompiledTables object.
/// </summary>
/// <returns>A new CompiledTables object, with room for strings, objects, and functions.</returns>
CompiledTables CompiledTables_Create(void)
{
	CompiledTables compiledTables;

	compiledTables = GC_MALLOC_STRUCT(struct CompiledTablesStruct);
	if (compiledTables == NULL)
		Smile_Abort_OutOfMemory();

	compiledTables->strings = GC_MALLOC_STRUCT_ARRAY(String, 16);
	if (compiledTables->strings == NULL)
		Smile_Abort_OutOfMemory();
	compiledTables->numStrings = 0;
	compiledTables->maxStrings = 16;

	compiledTables->stringLookup = StringIntDict_Create();

	compiledTables->compiledFunctions = GC_MALLOC_STRUCT_ARRAY(CompiledFunction, 16);
	if (compiledTables->compiledFunctions == NULL)
		Smile_Abort_OutOfMemory();
	compiledTables->numCompiledFunctions = 0;
	compiledTables->maxCompiledFunctions = 16;

	return compiledTables;
}

/// <summary>
/// Create a new Compiler object.
/// </summary>
/// <returns>A new Compiler object, ready to begin compiling to a new set of CompiledTables.</returns>
Compiler Compiler_Create(void)
{
	Compiler compiler;

	compiler = GC_MALLOC_STRUCT(struct CompilerStruct);
	if (compiler == NULL)
		Smile_Abort_OutOfMemory();

	compiler->compiledTables = CompiledTables_Create();
	compiler->currentFunction = NULL;

	compiler->firstMessage = NULL;
	compiler->lastMessage = NULL;

	return compiler;
}

/// <summary>
/// Begin compiling a function.
/// </summary>
/// <param name="compiler">The compiler that will be compiling this function.</param>
/// <param name="args">The function argument list.</param>
/// <param name="body">The function body.</param>
/// <returns>A new CompiledFunction object, which has its args/body assigned, but which is not yet
/// populated with any instructions.</returns>
CompiledFunction Compiler_BeginFunction(Compiler compiler, SmileList args, SmileObject body)
{
	CompiledFunction newFunction;
	CompiledTables compiledTables = compiler->compiledTables;
	Int index;

	// Create the new function.
	newFunction = GC_MALLOC_STRUCT(struct CompiledFunctionStruct);
	if (newFunction == NULL)
		Smile_Abort_OutOfMemory();
	newFunction->args = args;
	newFunction->body = body;
	newFunction->byteCodeSegment = ByteCodeSegment_Create();
	newFunction->numArgs = 0;
	newFunction->isCompiled = False;
	newFunction->parent = compiler->currentFunction;
	newFunction->localDepth = 0;
	newFunction->functionDepth = compiler->currentFunction != NULL ? compiler->currentFunction->functionDepth + 1 : 0;

	// Do we have enough space to add it?  If not, reallocate.
	if (compiledTables->numCompiledFunctions >= compiledTables->maxCompiledFunctions) {
		CompiledFunction *newCompiledFunctions;
		Int newMax;

		newMax = compiledTables->maxCompiledFunctions * 2;
		newCompiledFunctions = GC_MALLOC_STRUCT_ARRAY(CompiledFunction, newMax);
		if (newCompiledFunctions == NULL)
			Smile_Abort_OutOfMemory();
		MemCpy(newCompiledFunctions, compiledTables->compiledFunctions, compiledTables->numCompiledFunctions);
		compiledTables->compiledFunctions = newCompiledFunctions;
		compiledTables->maxCompiledFunctions = newMax;
	}

	// Okay, we have enough space, and it's not there yet, so add it.
	index = compiledTables->numCompiledFunctions++;
	compiledTables->compiledFunctions[index] = newFunction;

	// Assign this function its index in the list.
	newFunction->functionIndex = index;

	// Finally, make the new function the current function.
	compiler->currentFunction = newFunction;

	return newFunction;
}

CompileScope Compiler_BeginScope(Compiler compiler, Int kind)
{
	CompileScope newScope;

	// Create the new scope.
	newScope = GC_MALLOC_STRUCT(struct CompileScopeStruct);
	if (newScope == NULL)
		Smile_Abort_OutOfMemory();

	newScope->kind = kind;
	newScope->symbolDict = Int32Dict_Create();
	newScope->parent = compiler->currentScope;
	newScope->function = compiler->currentFunction;
	compiler->currentScope = newScope;

	return newScope;
}

void CompileScope_DefineSymbol(CompileScope scope, Symbol symbol, Int kind, Int index)
{
	CompiledLocalSymbol localSymbol = (CompiledLocalSymbol)GC_MALLOC_ATOMIC(sizeof(struct CompiledLocalSymbolStruct));
	if (localSymbol == NULL)
		Smile_Abort_OutOfMemory();

	localSymbol->kind = kind;
	localSymbol->index = index;
	localSymbol->symbol = symbol;
	localSymbol->scope = scope;

	Int32Dict_SetValue(scope->symbolDict, symbol, localSymbol);
}

CompiledLocalSymbol CompileScope_FindSymbol(CompileScope compileScope, Symbol symbol)
{
	CompiledLocalSymbol localSymbol;

	for (; compileScope != NULL; compileScope = compileScope->parent)
	{
		if (Int32Dict_TryGetValue(compileScope->symbolDict, symbol, &localSymbol))
			return localSymbol;
	}

	return NULL;
}

CompiledLocalSymbol CompileScope_FindSymbolHere(CompileScope compileScope, Symbol symbol)
{
	CompiledLocalSymbol localSymbol;
	return Int32Dict_TryGetValue(compileScope->symbolDict, symbol, &localSymbol) ? localSymbol : NULL;
}

/// <summary>
/// Add the given string to the compiler's string table (or find a preexisting string in the
/// string table that matches).
/// </summary>
/// <param name="compiler">The compiler that has the string table that this string will be added to.</param>
/// <param name="string">The string to add.</param>
/// <returns>The index of that string in the string table.</returns>
Int Compiler_AddString(Compiler compiler, String string)
{
	CompiledTables compiledTables = compiler->compiledTables;
	Int index;

	// See if we have this string already.
	if (StringIntDict_TryGetValue(compiledTables->stringLookup, string, &index))
		return index;

	// Do we have enough space to add it?  If not, reallocate.
	if (compiledTables->numStrings >= compiledTables->maxStrings) {
		String *newStrings;
		Int newMax;
	
		newMax = compiledTables->maxStrings * 2;
		newStrings = GC_MALLOC_STRUCT_ARRAY(String, newMax);
		if (newStrings == NULL)
			Smile_Abort_OutOfMemory();
		MemCpy(newStrings, compiledTables->strings, compiledTables->numStrings);
		compiledTables->strings = newStrings;
		compiledTables->maxStrings = newMax;
	}

	// Okay, we have enough space, and it's not there yet, so add it.
	index = compiledTables->numStrings++;
	compiledTables->strings[index] = string;
	StringIntDict_Add(compiledTables->stringLookup, string, index);

	return index;
}

/// <summary>
/// Add the given object to the compiler's static-object table.
/// </summary>
/// <param name="compiler">The compiler that has the object table that this object will be added to.</param>
/// <param name="obj">The object to add.</param>
/// <returns>The index of that object in the object table.</returns>
Int Compiler_AddObject(Compiler compiler, SmileObject obj)
{
	CompiledTables compiledTables = compiler->compiledTables;
	Int index;

	// Do we have enough space to add it?  If not, reallocate.
	if (compiledTables->numObjects >= compiledTables->maxObjects) {
		SmileObject *newObjects;
		Int newMax;

		newMax = compiledTables->maxObjects * 2;
		newObjects = GC_MALLOC_STRUCT_ARRAY(SmileObject, newMax);
		if (newObjects == NULL)
			Smile_Abort_OutOfMemory();
		MemCpy(newObjects, compiledTables->objects, compiledTables->numObjects);
		compiledTables->objects = newObjects;
		compiledTables->maxObjects = newMax;
	}

	// Okay, we have enough space, and it's not there yet, so add it.
	index = compiledTables->numObjects++;
	compiledTables->objects[index] = obj;

	return index;
}

/// <summary>
/// Compile the given *single* expression.
/// </summary>
/// <param name="compiler">The compiler that will be compiling this expression.</param>
/// <param name="expr">The expression to compile.</param>
/// <returns>The offset of the first instruction of this expression in the current function's ByteCodeSegment.</returns>
/// <remarks>
/// This is the core of the compiler:  It transforms expressions into bytecode, adding that new
/// bytecode to the end of the ByteCodeSegment of the current function.
///
/// This function's behavior is undefined (i.e., broken) if the compiler has a NULL 'currentFunction'.
///
/// Note that you must have evaluated all macros before calling this function:  In particular,
/// this means special handling for the [$scope] expression.  This will compile a full [$scope],
/// but it compiles that [$scope] as-is, and does not evaluate macros.
///
/// Note also that this function does not compile nested functions:  It merely creates new
/// CompiledFunction objects for them, with their 'isCompiled' flags set to False.
/// </remarks>
Int Compiler_CompileExpr(Compiler compiler, SmileObject expr)
{
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	Int startIndex = segment->numByteCodes;
	SmileList list;
	Int argCount, index;

	switch (SMILE_KIND(expr)) {

		// Primitive constants.
		case SMILE_KIND_NULL:
			ByteCodeSegment_Emit(segment, Op_LdNull);
			break;
		case SMILE_KIND_BOOL:
			ByteCodeSegment_Emit(segment, Op_LdBool)->u.boolean = ((SmileBool)expr)->value;
			break;
		case SMILE_KIND_CHAR:
			ByteCodeSegment_Emit(segment, Op_LdCh)->u.ch = ((SmileChar)expr)->value;
			break;
		case SMILE_KIND_UCHAR:
			ByteCodeSegment_Emit(segment, Op_LdUCh)->u.uch = ((SmileUChar)expr)->value;
			break;
		case SMILE_KIND_STRING:
			ByteCodeSegment_Emit(segment, Op_LdStr)->u.index = Compiler_AddString(compiler, (String)&((SmileString)expr)->string);
			break;
		case SMILE_KIND_OBJECT:
			ByteCodeSegment_Emit(segment, Op_LdObj)->u.index = Compiler_AddObject(compiler, Smile_KnownObjects.Object);
			break;

		// Integer constants evaluate to themselves.
		case SMILE_KIND_BYTE:
			ByteCodeSegment_Emit(segment, Op_Ld8)->u.byte = ((SmileByte)expr)->value;
			break;
		case SMILE_KIND_INTEGER16:
			ByteCodeSegment_Emit(segment, Op_Ld16)->u.int16 = ((SmileInteger16)expr)->value;
			break;
		case SMILE_KIND_INTEGER32:
			ByteCodeSegment_Emit(segment, Op_Ld32)->u.int32 = ((SmileInteger32)expr)->value;
			break;
		case SMILE_KIND_INTEGER64:
			ByteCodeSegment_Emit(segment, Op_Ld64)->u.int64 = ((SmileInteger64)expr)->value;
			break;
		case SMILE_KIND_INTEGER128:
			Smile_Abort_FatalError("Integer128 is not yet supported.");
			break;
		case SMILE_KIND_BIGINT:
			Smile_Abort_FatalError("BigInt is not yet supported.");
			break;

		// Real constants evaluate to themselves.
		case SMILE_KIND_REAL16:
		case SMILE_KIND_REAL32:
		case SMILE_KIND_REAL64:
		case SMILE_KIND_REAL128:
		case SMILE_KIND_BIGREAL:
			Smile_Abort_FatalError("Reals are not yet supported.");
			break;

		// Float constants evaluate to themselves.
		case SMILE_KIND_FLOAT16:
		case SMILE_KIND_FLOAT32:
		case SMILE_KIND_FLOAT64:
		case SMILE_KIND_FLOAT128:
		case SMILE_KIND_BIGFLOAT:
			Smile_Abort_FatalError("Floats are not yet supported.");
			break;

		// User data evaluates to iteself.
		case SMILE_KIND_FUNCTION:
		case SMILE_KIND_HANDLE:
		case SMILE_KIND_NONTERMINAL:
		case SMILE_KIND_USEROBJECT:
		case SMILE_KIND_CLOSURE:
		case SMILE_KIND_FACADE:
			Smile_Abort_FatalError("These special forms are not yet supported.");
			break;

		// Intermediate forms (if they somehow make it this far) throw errors.
		case SMILE_KIND_MACRO:
		case SMILE_KIND_PARSEDECL:
		case SMILE_KIND_PARSEMESSAGE:
			Smile_Abort_FatalError("Intermediate forms are not supported.");
			break;
		
		// Simple pairs resolve by performing a property lookup on the object in question,
		// which may walk up the base chain for that object.
		case SMILE_KIND_PAIR:
			Compiler_CompileProperty(compiler, (SmilePair)expr, False);
			break;
		
		// Symbols (variables) resolve to whatever the current closure (or any base closure) says they are.
		case SMILE_KIND_SYMBOL:
			Compiler_CompileVariable(compiler, ((SmileSymbol)expr)->symbol, False);
			break;
		
		// Syntax objects resolve to themselves, like most other special user data does.
		case SMILE_KIND_SYNTAX:
			index = Compiler_AddObject(compiler, expr);
			ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_LdObj)->u.index = index;
			break;
		
		// In general, lists resolve by evaluating their arguments and then passing the results
		// to the function described by the first argument.  There are, however, a few forms which
		// evaluate specially, more forms than McCarthy's Lisp had, but still relatively few overall.
		case SMILE_KIND_LIST:
			list = (SmileList)expr;
			switch (SMILE_KIND(list->a)) {

				// Invocation of a method on an object, of the form [obj.method ...].
				case SMILE_KIND_PAIR:
					Compiler_CompileMethodCall(compiler, (SmilePair)list->a, (SmileList)list->d);
					break;
				
				// Either invocation of a known built-in (like [$if] or [$set] or [$scope]), or resolution
				// of a named function in the current scope.
				case SMILE_KIND_SYMBOL:
					if (SMILE_KIND(list->d) != SMILE_KIND_LIST && SMILE_KIND(list->d) != SMILE_KIND_NULL) {
						Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(list),
							String_FromC("Cannot compile list: List is not well-formed.")));
					}
					if (Compiler_CompileStandardForm(compiler, ((SmileSymbol)list->a)->symbol, (SmileList)list->d))
						break;
					// ...fall-thru to default case...

				// Resolve the given expression, and then call it, passing the rest of the list as
				// arguments (after they have been evaluated).
				default:
					// Resolve each element of the list.  The first element will become the function,
					// and the rest will become the arguments.
					argCount = 0;
					for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list), argCount++) {
						Compiler_CompileExpr(compiler, list->a);
					}
					
					// Under the hood, Call does some magic:
					//   1.  If the would-be function is actually a function, it is called directly;
					//   2.  Otherwise, if it is not a function, but it is an object that has an 'fn' method,
					//        then that method is called instead;
					//   3.  Otherwise, if it does not have an 'fn' method, then Call attempts to invoke
					//        [x.does-not-understand `fn ...] on it.
					//   4.  Otherwise, if it does not have a 'does-not-understand' method, a run-time exception is thrown.
					ByteCodeSegment_Emit(segment, Op_Call)->u.index = argCount;
					break;
			}
			break;
		
		default:
			Smile_Abort_FatalError(String_ToC(String_Format("Cannot compile unknown/unsupported object type 0x%02X.", SMILE_KIND(expr))));
			break;
	}

	return startIndex;
}

static void Compiler_CompileProperty(Compiler compiler, SmilePair pair, Bool store)
{
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	Symbol symbol;

	if (SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmilePair_GetSourceLocation(pair),
			String_FromC("Cannot compile pair: right side must be a symbol.")));
	}

	// Evaluate the left side first, which will leave the left side on the stack.
	Compiler_CompileExpr(compiler, pair->left);

	// Extract the property named by the symbol on the right side, leaving the property's value on the stack.
	symbol = ((SmileSymbol)(pair->right))->symbol;
	ByteCodeSegment_Emit(segment, (store ? Op_StProp : Op_LdProp))->u.symbol = symbol;
}

static void Compiler_CompileVariable(Compiler compiler, Symbol symbol, Bool store)
{
	ByteCode byteCode;
	Int functionDepth;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	CompiledLocalSymbol localSymbol = CompileScope_FindSymbol(compiler->currentScope, symbol);

	if (localSymbol == NULL) {
		// Don't know what this is, so it comes from an outer closure.
		ByteCodeSegment_Emit(segment, (store ? Op_StX : Op_LdX))->u.symbol = symbol;
		return;
	}

	functionDepth = compiler->currentFunction->functionDepth - localSymbol->scope->function->functionDepth;

	switch (localSymbol->kind) {

		case PARSEDECL_ARGUMENT:
			if (functionDepth <= 7) {
				byteCode = ByteCodeSegment_Emit(segment, (store ? Op_StArg0 : Op_LdArg0) + functionDepth);
			}
			else {
				byteCode = ByteCodeSegment_Emit(segment, (store ? Op_StArg : Op_LdArg));
				byteCode->u.i2.a = functionDepth;
				byteCode->u.i2.b = localSymbol->index;
			}
			break;

		case PARSEDECL_VARIABLE:
			if (functionDepth <= 7) {
				byteCode = ByteCodeSegment_Emit(segment, (store ? Op_StLoc0 : Op_LdLoc0) + functionDepth);
				byteCode->u.index = localSymbol->index;
			}
			else {
				byteCode = ByteCodeSegment_Emit(segment, (store ? Op_StLoc : Op_LdLoc));
				byteCode->u.i2.a = functionDepth;
				byteCode->u.i2.b = localSymbol->index;
			}
			break;

		default:
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, NULL,
				String_FromC("Cannot compile symbol:  Fatal internal error.")));
			break;
	}
}

static void Compiler_CompileMethodCall(Compiler compiler, SmilePair pair, SmileList args)
{
	Int length;
	SmileList temp;
	ByteCode byteCode;
	Symbol symbol;

	// First, make sure the args are well-formed, and count how many of them there are.
	length = SmileList_Length(args);

	// Make sure this is a valid method call form.
	if (length < 0 || SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, NULL,
			String_FromC("Cannot compile method call: Argument list is not well-formed.")));
		return;
	}

	// Evaluate the left side of the pair (the object to invoke).
	Compiler_CompileExpr(compiler, pair->left);
	symbol = ((SmileSymbol)pair->right)->symbol;

	// Evaluate all of the arguments.
	for (temp = args; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
		Compiler_CompileExpr(compiler, temp->a);
	}

	// Invoke the method by symbol name.
	if (length <= 7) {
		// If this is the special get-member method, use the special fast instruction for that.
		if (length == 1 && symbol == Smile_KnownSymbols.get_member) {
			ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_LdMember);
		}
		else {
			// Use a short form.
			ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Met0 + length)->u.symbol = symbol;
		}
	}
	else {
		byteCode = ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Met);
		byteCode->u.i2.a = length;
		byteCode->u.i2.b = (Int32)symbol;
	}
}

static Bool Compiler_CompileStandardForm(Compiler compiler, Symbol symbol, SmileList args)
{
	switch (symbol) {

		// Assignment.
		case SMILE_SPECIAL_SYMBOL__SET:
			Compiler_CompileSetf(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__OPSET:
			Compiler_CompileOpEquals(compiler, args);
			return True;

		// Control flow.
		case SMILE_SPECIAL_SYMBOL__IF:
			Compiler_CompileIf(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__WHILE:
			Compiler_CompileWhile(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__TILL:
			Compiler_CompileTill(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__CATCH:
			Compiler_CompileCatch(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__RETURN:
			Compiler_CompileReturn(compiler, args);
			return True;

		// Expression-evaluation control.
		case SMILE_SPECIAL_SYMBOL__FN:
			Compiler_CompileFn(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__QUOTE:
			Compiler_CompileQuote(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__PROG1:
			Compiler_CompileProg1(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__PROGN:
			Compiler_CompileProgN(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__SCOPE:
			Compiler_CompileScope(compiler, args);
			return True;

		// Object creation and type testing.
		case SMILE_SPECIAL_SYMBOL__NEW:
			Compiler_CompileNew(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__IS:
			Compiler_CompileIs(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__TYPEOF:
			Compiler_CompileTypeOf(compiler, args);
			return True;

		// Reference comparison.
		case SMILE_SPECIAL_SYMBOL__EQ:
			Compiler_CompileSuperEq(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__NE:
			Compiler_CompileSuperNe(compiler, args);
			return True;
		
		// Logical operations.
		case SMILE_SPECIAL_SYMBOL__AND:
			Compiler_CompileAnd(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__OR:
			Compiler_CompileOr(compiler, args);
			return True;
		case SMILE_SPECIAL_SYMBOL__NOT:
			Compiler_CompileNot(compiler, args);
			return True;

		// Don't know what this is, so it's likely an evaluation of whatever's in scope.
		default:
			return False;
	}
}

static void Compiler_CompileSetf(Compiler compiler, SmileList args)
{
	Int length;
	SmileObject dest, value, index;
	SmilePair pair;
	Symbol symbol;

	// There are three possible legal forms for the arguments:
	//
	//   [$set symbol value]
	//   [$set obj.property value]
	//   [$set [(obj.get-member) index] value]
	//
	// We have to determine which one of these we have, and compile an appropriate
	// assignment (or method invocation) accordingly.

	// Make sure this is a well-formed list of exactly two elements.
	length = SmileList_Length(args);
	if (length != 2) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$set]: Expression is not well-formed.")));
		return;
	}

	// Get the destination object, and the value to be assigned.
	dest = args->a;
	value = ((SmileList)args->d)->a;

	switch (SMILE_KIND(dest)) {

		case SMILE_KIND_SYMBOL:
			// This is of the form [$set symbol value].
			symbol = ((SmileSymbol)dest)->symbol;
		
			// Load the value to store.
			Compiler_CompileExpr(compiler, value);

			// Store it, leaving a duplicate on the stack.
			Compiler_CompileVariable(compiler, symbol, True);
			break;

		case SMILE_KIND_PAIR:
			// This is probably of the form [$set obj.property value].  Make sure the right side
			// of the pair is a symbol.
			pair = (SmilePair)dest;
			if (SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL) {
				Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
					String_FromC("Cannot compile [=]: Expression is not well-formed.")));
				return;
			}
			symbol = ((SmileSymbol)pair->right)->symbol;
		
			// Evaluate the left side first.
			Compiler_CompileExpr(compiler, pair->left);
		
			// Evaluate the value second.  (Doing this second ensures that everything always
			// evaluates left-to-right, the order in which it was written.)
			Compiler_CompileExpr(compiler, value);
		
			// Assign the property.
			ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_StProp)->u.symbol = symbol;
			break;
		
		case SMILE_KIND_LIST:
			// This is probably of the form [$set [(obj.get-member) index] value].  Make sure that the
			// inner list is well-formed, has two elements, the first element is a pair, and the right
			// side of the first element is the special symbol "get-member".
			if (SmileList_Length((SmileList)dest) != 2 || SMILE_KIND(((SmileList)dest)->a) != SMILE_KIND_PAIR) {
				Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
					String_FromC("Cannot compile [=]: Expression is not well-formed.")));
				return;
			}
			pair = (SmilePair)(((SmileList)dest)->a);
			if (SMILE_KIND(pair->right) != SMILE_KIND_SYMBOL) {
				Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
					String_FromC("Cannot compile [=]: Expression is not well-formed.")));
				return;
			}
			index = LIST_SECOND((SmileList)dest);
		
			// Okay.  We now have pair->left, pair->right, index, and value.  Let's compile them.
			Compiler_CompileExpr(compiler, pair->left);
			Compiler_CompileExpr(compiler, index);
			Compiler_CompileExpr(compiler, value);
			ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_StMember);
			break;
		
		default:
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
				String_FromC("Cannot compile [=]: Expression is not well-formed.")));
			return;
	}
}

static void Compiler_CompileOpEquals(Compiler compiler, SmileList args)
{
	UNUSED(compiler);
	UNUSED(args);
}

static Bool StripNots(SmileObject *objPtr)
{
	Bool not = False;
	SmileList list = (SmileList)*objPtr;

	// If this is of the form [$not x]...
	while (SMILE_KIND(list) == SMILE_KIND_LIST && SMILE_KIND(list->a) == SMILE_KIND_SYMBOL
		&& ((SmileSymbol)list->a)->symbol == Smile_KnownSymbols.not_
		&& SMILE_KIND(list->d) == SMILE_KIND_LIST && SMILE_KIND(((SmileList)list->d)->d) == SMILE_KIND_NULL)
	{
		// ...substitute it with just x, and keep track of how many [$not]s we removed.
		list = (SmileList)((SmileList)list->d)->a;
		not = !not;
	}

	*objPtr = (SmileObject)list;

	return not;
}

static void Compiler_CompileIf(Compiler compiler, SmileList args)
{
	SmileObject condition, thenClause, elseClause, temp;
	Int elseKind;
	Bool not;
	ByteCodeSegment segment;
	Int bfOffset, bfLabelOffset, bfDelta;
	Int jmpOffset, jmpLabelOffset, jmpDelta;
	ByteCode bf, jmp, bfLabel, jmpLabel;

	segment = compiler->currentFunction->byteCodeSegment;

	// Must be an expression of the form [$if cond then-clause] or [$if cond then-clause else-clause].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return;
	}

	// Get the condition.
	condition = args->a;

	// Get the then-clause.
	args = (SmileList)args->d;
	thenClause = args->a;

	// Figure out how this form ends, and consume the rest of it.
	if ((elseKind = SMILE_KIND(args->d)) == SMILE_KIND_LIST) {
	
		// If there's an else-clause, consume it.
		args = (SmileList)args->d;
		elseClause = args->a;
	
		// This should be the end of the form.
		if ((elseKind = SMILE_KIND(args->d)) != SMILE_KIND_NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
				String_FromC("Cannot compile [if]: Expression is not well-formed.")));
			return;
		}
	}
	else if (elseKind == SMILE_KIND_NULL) {
		// If no else clause, this should end the form.
		elseClause = NullObject;
	}
	else {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return;
	}

	// Extract off any [$not] operators, and if there were any, swap then/else clauses.
	not = StripNots(&condition);
	if (not) {
		temp = elseClause;
		elseClause = thenClause;
		thenClause = temp;
	}

	// Evaluate the condition.
	Compiler_CompileExpr(compiler, condition);

	// Emit the conditional logic.
	bfOffset = segment->numByteCodes;
	bf = ByteCodeSegment_Emit(segment, Op_Bf);
	Compiler_CompileExpr(compiler, thenClause);
	jmpOffset = segment->numByteCodes;
	jmp = ByteCodeSegment_Emit(segment, Op_Jmp);
	bfLabelOffset = segment->numByteCodes;
	bfLabel = ByteCodeSegment_Emit(segment, Op_Label);
	Compiler_CompileExpr(compiler, elseClause);
	jmpLabelOffset = segment->numByteCodes;
	jmpLabel = ByteCodeSegment_Emit(segment, Op_Label);

	// Fill in the relative branch targets.
	bfDelta = bfLabelOffset - bfOffset;
	bf->u.index = bfDelta;
	bfLabel->u.index = -bfDelta;
	jmpDelta = jmpLabelOffset - jmpOffset;
	jmp->u.index = jmpDelta;
	jmpLabel->u.index = -jmpDelta;
}

static void Compiler_CompileWhile(Compiler compiler, SmileList args)
{
	SmileObject condition, preClause, postClause;
	Int postKind, tailKind;
	Bool not, hasPre, hasPost;
	ByteCodeSegment segment;
	Int bOffset, bLabelOffset, bDelta;
	Int jmpOffset, jmpLabelOffset, jmpDelta;
	ByteCode b, jmp, bLabel, jmpLabel;

	segment = compiler->currentFunction->byteCodeSegment;

	// Must be an expression of the form [$while cond postBody] or [$while pre-body cond post-body].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return;
	}

	// Get the condition.
	condition = args->a;

	// Get the body.
	args = (SmileList)args->d;
	postClause = args->a;

	// Figure out how this form ends, and consume the rest of it.
	if ((postKind = SMILE_KIND(args->d)) == SMILE_KIND_LIST) {
	
		// There are three parts, so this is a pre-cond-post form.
		args = (SmileList)args->d;
		preClause = condition;
		condition = postClause;
		postClause = args->a;
	
		// This should be the end of the form.
		if ((tailKind = SMILE_KIND(args->d)) != SMILE_KIND_NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
				String_FromC("Cannot compile [$while]: Expression is not well-formed.")));
			return;
		}
	}
	else if (postKind == SMILE_KIND_NULL) {
		// If only two parts, there is no pre-clause.
		preClause = NullObject;
	}
	else {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$while]: Expression is not well-formed.")));
		return;
	}

	// Extract off any [$not] operators, and if there were any, we'll invert the branches.
	not = StripNots(&condition);

	// Find out the structure of this [$while] form.
	hasPre = (SMILE_KIND(preClause) != SMILE_KIND_NULL);
	hasPost = (SMILE_KIND(postClause) != SMILE_KIND_NULL);

	if (hasPre && hasPost) {
		// Form: do {...} while cond then {...}
		//
		// Emit this in order of:
		//   l1: eval stmts1
		//       eval cond
		//       branch l2
		//       eval stmts2
		//       jmp l1
		//   l2:

		jmpLabelOffset = segment->numByteCodes;
		jmpLabel = ByteCodeSegment_Emit(segment, Op_Label);

		Compiler_CompileExpr(compiler, preClause);

		Compiler_CompileExpr(compiler, condition);

		bOffset = segment->numByteCodes;
		b = ByteCodeSegment_Emit(segment, not ? Op_Bf : Op_Bt);
	
		Compiler_CompileExpr(compiler, postClause);

		jmpOffset = segment->numByteCodes;
		jmp = ByteCodeSegment_Emit(segment, Op_Jmp);

		bLabelOffset = segment->numByteCodes;
		bLabel = ByteCodeSegment_Emit(segment, Op_Label);

		bDelta = bLabelOffset - bOffset;
		b->u.index = bDelta;
		bLabel->u.index = -bDelta;

		jmpDelta = jmpLabelOffset - jmpOffset;
		jmp->u.index = jmpDelta;
		jmpLabel->u.index = -jmpDelta;
	}
	else if (hasPre) {
		// Form: do {...} while cond
		//
		// Emit this in order of:
		//   l1: eval stmts
		//       eval cond
		//       branch l1

		bLabelOffset = segment->numByteCodes;
		bLabel = ByteCodeSegment_Emit(segment, Op_Label);
	
		Compiler_CompileExpr(compiler, preClause);
		Compiler_CompileExpr(compiler, condition);

		bOffset = segment->numByteCodes;
		b = ByteCodeSegment_Emit(segment, not ? Op_Bf : Op_Bt);

		bDelta = bLabelOffset - bOffset;
		b->u.index = bDelta;
		bLabel->u.index = -bDelta;
	}
	else if (hasPost) {
		// Form: while cond do {...}
		//
		// Emit this in order of:
		//       jmp l1
		//   l2: eval stmts
		//   l1: eval cond
		//       branch l2
	
		jmpOffset = segment->numByteCodes;
		jmp = ByteCodeSegment_Emit(segment, Op_Jmp);
	
		bLabelOffset = segment->numByteCodes;
		bLabel = ByteCodeSegment_Emit(segment, Op_Label);
	
		Compiler_CompileExpr(compiler, postClause);

		jmpLabelOffset = segment->numByteCodes;
		jmpLabel = ByteCodeSegment_Emit(segment, Op_Label);
	
		Compiler_CompileExpr(compiler, condition);

		bOffset = segment->numByteCodes;
		b = ByteCodeSegment_Emit(segment, not ? Op_Bf : Op_Bt);
	
		bDelta = bLabelOffset - bOffset;
		b->u.index = bDelta;
		bLabel->u.index = -bDelta;
	
		jmpDelta = jmpLabelOffset - jmpOffset;
		jmp->u.index = jmpDelta;
		jmpLabel->u.index = -jmpDelta;
	}
	else {
		// Form: while cond {}
		//
		// Emit this in order of:
		//   l1: eval cond
		//       branch l1

		bLabelOffset = segment->numByteCodes;
		bLabel = ByteCodeSegment_Emit(segment, Op_Label);
	
		Compiler_CompileExpr(compiler, condition);

		bOffset = segment->numByteCodes;
		b = ByteCodeSegment_Emit(segment, not ? Op_Bf : Op_Bt);
	
		bDelta = bLabelOffset - bOffset;
		b->u.index = bDelta;
		bLabel->u.index = -bDelta;
	}
}

static void Compiler_CompileTill(Compiler compiler, SmileList args)
{
	UNUSED(compiler);
	UNUSED(args);
}

static void Compiler_CompileCatch(Compiler compiler, SmileList args)
{
	UNUSED(compiler);
	UNUSED(args);
}

static void Compiler_CompileReturn(Compiler compiler, SmileList args)
{
	if (SMILE_KIND(args) == SMILE_KIND_NULL) {
		// Naked [$return], so we're implicitly returning null.
		ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_LdNull);
		ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Ret);
	}
	else if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [return]: Expression is not well-formed.")));
		return;
	}
	else {
		// Compile the return expression...
		Compiler_CompileExpr(compiler, args->a);
	
		// ...and return it.
		ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Ret);
	}
}

static void Compiler_CompileFn(Compiler compiler, SmileList args)
{
	CompiledFunction compiledFunction;
	CompileScope scope;
	SmileList functionArgs, temp;
	SmileObject functionBody;
	Int numFunctionArgs;

	// The [$fn] expression must be of the form:  [$fn [args...] body].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->a) != SMILE_KIND_LIST
		|| SMILE_KIND(args->d) != SMILE_KIND_LIST || SMILE_KIND(((SmileList)args->d)->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [fn]: Expression is not well-formed.")));
		return;
	}

	// Begin a new symbol scope.
	scope = Compiler_BeginScope(compiler, PARSESCOPE_FUNCTION);

	// Declare the argument symbols, so that they can be correctly resolved.
	functionArgs = (SmileList)args->a;
	numFunctionArgs = 0;
	for (temp = functionArgs; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
		if (SMILE_KIND(temp->a) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(temp),
				String_Format("Cannot compile [fn]: Argument #%d is not a valid argument name.", numFunctionArgs + 1)));
		}
		Symbol symbol = ((SmileSymbol)temp->a)->symbol;
		CompileScope_DefineSymbol(scope, symbol, PARSEDECL_ARGUMENT, numFunctionArgs++);
	}
	if (SMILE_KIND(temp) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [fn]: Arguments are not well-formed.")));
		return;
	}

	// Create the function.
	functionBody = ((SmileList)args->d)->a;
	compiledFunction = Compiler_BeginFunction(compiler, functionArgs, functionBody);
	compiledFunction->numArgs = numFunctionArgs;

	// If this function has arguments, emit an 'args' instruction to ensure at least that many arguments exist.
	if (numFunctionArgs > 0) {
		ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Args)->u.index = numFunctionArgs;
	}

	// Compile the body.
	Compiler_CompileExpr(compiler, functionBody);

	// Emit a return instruction at the end.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Ret);

	// We're done compiling this function.
	Compiler_EndFunction(compiler);

	// Finally, emit an instruction to load a new instance of this function onto its parent's stack.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_NewFn)->u.index = compiledFunction->functionIndex;
}

static void Compiler_CompileQuote(Compiler compiler, SmileList args)
{
	Int objectIndex;

	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [quote]: Expression is not well-formed.")));
		return;
	}

	if (SMILE_KIND(args->a) == SMILE_KIND_SYMBOL) {
		// A quoted symbol can just be loaded directly.
		ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_LdSym)->u.symbol = ((SmileSymbol)args->a)->symbol;
		return;
	}
	else if (SMILE_KIND(args->a) != SMILE_KIND_LIST && SMILE_KIND(args->a) != SMILE_KIND_PAIR) {
		// It's neither a list nor a pair nor a symbol, so quoting it just results in *it*, whatever it is.
		Compiler_CompileExpr(compiler, args->a);
		return;
	}

	// Add the quoted form as a literal stored object.
	objectIndex = Compiler_AddObject(compiler, args->a);

	// Add an instruction to load it.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_LdObj);
}

static void Compiler_CompileProg1(Compiler compiler, SmileList args)
{
	if (SMILE_KIND(args) != SMILE_KIND_LIST)
		return;

	// Compile this expression, and keep it.
	Compiler_CompileExpr(compiler, args->a);
	args = LIST_REST(args);

	for (; SMILE_KIND(args) == SMILE_KIND_LIST; args = LIST_REST(args)) {
	
		// Compile this next expression...
		Compiler_CompileExpr(compiler, args->a);
	
		// ...and discard its result.
		ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Pop1);
	}
}

Int Compiler_CompileExprs(Compiler compiler, SmileList exprs)
{
	Int offset = compiler->currentFunction->byteCodeSegment->numByteCodes;
	Compiler_CompileProgN(compiler, exprs);
	return offset;
}

static void Compiler_CompileProgN(Compiler compiler, SmileList args)
{
	if (SMILE_KIND(args) != SMILE_KIND_LIST)
		return;

	for (;;) {
		// Compile this next expression...
		Compiler_CompileExpr(compiler, args->a);
		
		// ...and if it's the last one, keep its value.
		args = LIST_REST(args);
		if (SMILE_KIND(args) != SMILE_KIND_LIST) break;

		// Otherwise, discard it and move to the next expression.
		ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Pop1);
	}
}

static void Compiler_CompileScope(Compiler compiler, SmileList args)
{
	CompileScope scope;
	SmileList scopeVars, temp;
	Int numScopeVars;

	// The [$scope] expression must be of the form:  [$scope [locals...] ...].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->a) != SMILE_KIND_LIST
		|| !(SMILE_KIND(args->d) == SMILE_KIND_LIST || SMILE_KIND(args->d) == SMILE_KIND_NULL)) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$scope]: Expression is not well-formed.")));
		return;
	}

	scope = Compiler_BeginScope(compiler, PARSESCOPE_SCOPEDECL);

	// Declare the [locals...] list, which must be well-formed, and must consist only of symbols.
	scopeVars = (SmileList)args->a;
	numScopeVars = 0;
	for (temp = scopeVars; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
		if (SMILE_KIND(temp->a) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(temp),
				String_Format("Cannot compile [$scope]: Variable #%d is not a valid local variable name.", numScopeVars + 1)));
		}

		Symbol symbol = ((SmileSymbol)temp->a)->symbol;
		CompileScope_DefineSymbol(scope, symbol, PARSEDECL_VARIABLE, numScopeVars++);
	}
	if (SMILE_KIND(temp) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$scope]: Local-variable list is not well-formed.")));
		return;
	}

	// Allocate more space on the stack for these locals.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_LocalAlloc)->u.index = numScopeVars;
	compiler->currentFunction->localDepth += numScopeVars;

	// Compile the rest of the [scope] as though it was just a [progn].
	Compiler_CompileProgN(compiler, (SmileList)args->d);

	// Free the local variables, now that we no longer need them.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_LocalFree)->u.index = numScopeVars;
	compiler->currentFunction->localDepth -= numScopeVars;

	Compiler_EndScope(compiler);
}

static void Compiler_CompileNew(Compiler compiler, SmileList args)
{
	Int numPairs;
	SmileList pairs, pair;
	Symbol symbol;
	SmileObject value;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;

	// Must be an expression of the form: [$new base [[sym1 val1] [sym2 val2] [sym3 val3] ...]]
	if (SmileList_Length(args) != 2) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$new]: Expression is not well-formed.")));
		return;
	}

	// Compile the base object reference.
	Compiler_CompileExpr(compiler, LIST_FIRST(args));

	// Compile all the pairs.
	for (pairs = (SmileList)LIST_SECOND(args), numPairs = 0; SMILE_KIND(pairs) == SMILE_KIND_LIST; pairs = (SmileList)pairs->d, numPairs++) {
		pair = (SmileList)pairs->a;
		if (SMILE_KIND(pair) != SMILE_KIND_LIST) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
				String_FromC("Cannot compile [$new]: Expression is not well-formed.")));
			return;
		}
		if (SmileList_Length(pair) != 2 || SMILE_KIND(pair->a) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(pair),
				String_FromC("Cannot compile [$new]: Expression is not well-formed.")));
			return;
		}
		symbol = ((SmileSymbol)pair->a)->symbol;
		value = LIST_SECOND(pair);
		ByteCodeSegment_Emit(segment, Op_LdSym)->u.symbol = symbol;
		Compiler_CompileExpr(compiler, value);
	}
	if (SMILE_KIND(pairs) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$new]: Expression is not well-formed.")));
		return;
	}

	// Create the new object.
	ByteCodeSegment_Emit(segment, Op_NewObj)->u.int32 = numPairs;
}

static void Compiler_CompileIs(Compiler compiler, SmileList args)
{
	// Must be an expression of the form [$is x y].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST
		|| SMILE_KIND(((SmileList)args->d)->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [is]: Expression is not well-formed.")));
		return;
	}

	// Compile the first expression.
	Compiler_CompileExpr(compiler, args->a);

	// Compile the second expression.
	Compiler_CompileExpr(compiler, ((SmileList)args->d)->a);

	// Add an instruction to perform inheritance comparison on the result.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Is);
}

static void Compiler_CompileTypeOf(Compiler compiler, SmileList args)
{
	// Must be an expression of the form [$typeof x].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [typeof]: Expression is not well-formed.")));
		return;
	}

	// Compile the expression.
	Compiler_CompileExpr(compiler, args->a);

	// Add an instruction to get the type symbol for this object.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_TypeOf);
}

static void Compiler_CompileSuperEq(Compiler compiler, SmileList args)
{
	// Must be an expression of the form [$eq x y].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST
		|| SMILE_KIND(((SmileList)args->d)->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [===]: Expression is not well-formed.")));
		return;
	}

	// Compile the first expression.
	Compiler_CompileExpr(compiler, args->a);

	// Compile the second expression.
	Compiler_CompileExpr(compiler, ((SmileList)args->d)->a);

	// Add an instruction to perform reference comparison on the result.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_SuperEq);
}

static void Compiler_CompileSuperNe(Compiler compiler, SmileList args)
{
	// Must be an expression of the form [$ne x y].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST
		|| SMILE_KIND(((SmileList)args->d)->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [!==]: Expression is not well-formed.")));
		return;
	}

	// Compile the first expression.
	Compiler_CompileExpr(compiler, args->a);

	// Compile the second expression.
	Compiler_CompileExpr(compiler, ((SmileList)args->d)->a);

	// Add an instruction to perform reference comparison on the result.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_SuperNe);
}

static void Compiler_CompileAnd(Compiler compiler, SmileList args)
{
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	SmileList temp;
	Int i, length;
	SmileObject condition;
	Bool not;

	ByteCode localBfByteCodes[16];
	Int localBfOffsets[16];
	ByteCode *bfByteCodes;
	Int *bfOffsets;
	Int falseOffset;
	ByteCode jmpByteCode, jmpLabelByteCode;
	Int jmpOffset, jmpLabelOffset, jmpDelta;

	// Must be a well-formed expression of the form [$and x y z ...].
	if ((length = SmileList_Length(args)) <= 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [and]: Expression is not well-formed.")));
		return;
	}

	// Create somewhere to store the byte-code branches, if there are a lot of them.
	if (length > 16) {
		bfByteCodes = GC_MALLOC_STRUCT_ARRAY(ByteCode, length);
		bfOffsets = (Int *)GC_MALLOC_ATOMIC(sizeof(Int) * length);
		if (bfByteCodes == NULL || bfOffsets == NULL)
			Smile_Abort_OutOfMemory();
	}
	else {
		bfByteCodes = localBfByteCodes;
		bfOffsets = localBfOffsets;
	}

	// Emit all of the conditionals.
	for (i = 0, temp = args; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d, i++) {
	
		// Extract off any [$not] operators, and if there were any, invert the branch below.
		condition = temp->a;
		not = StripNots(&condition);

		// Compile the next expression.
		Compiler_CompileExpr(compiler, condition);
	
		// If falsy, branch to result in 'false'.
		bfOffsets[i] = segment->numByteCodes;
		bfByteCodes[i] = ByteCodeSegment_Emit(segment, not ? Op_Bt : Op_Bf);
	
		// It's truthy, so keep going.
	}

	// We passed all the tests, so the result is true.
	ByteCodeSegment_Emit(segment, Op_LdBool)->u.boolean = True;
	jmpOffset = segment->numByteCodes;
	jmpByteCode = ByteCodeSegment_Emit(segment, Op_Jmp);

	// Now handle the falsy case.
	falseOffset = segment->numByteCodes;
	ByteCodeSegment_Emit(segment, Op_LdBool)->u.boolean = False;

	// Add a branch target for the jump.
	jmpLabelOffset = segment->numByteCodes;
	jmpLabelByteCode = ByteCodeSegment_Emit(segment, Op_Label);

	// Now fill in all the branch deltas for the conditional branches.
	for (i = 0; i < length; i++) {
		bfByteCodes[i]->u.index = falseOffset - bfOffsets[i];
	}

	// And fill in the branch delta for the unconditional branch.
	jmpDelta = jmpLabelOffset - jmpOffset;
	jmpByteCode->u.index = jmpDelta;
	jmpLabelByteCode->u.index = -jmpDelta;
}

static void Compiler_CompileOr(Compiler compiler, SmileList args)
{
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	SmileList temp;
	Int i, length;
	SmileObject condition;
	Bool not;

	ByteCode localBtByteCodes[16];
	Int localBtOffsets[16];
	ByteCode *btByteCodes;
	Int *btOffsets;
	Int trueOffset;
	ByteCode jmpByteCode, jmpLabelByteCode;
	Int jmpOffset, jmpLabelOffset, jmpDelta;

	// Must be a well-formed expression of the form [$or x y z ...].
	if ((length = SmileList_Length(args)) <= 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [or]: Expression is not well-formed.")));
		return;
	}

	// Create somewhere to store the byte-code branches, if there are a lot of them.
	if (length > 16) {
		btByteCodes = GC_MALLOC_STRUCT_ARRAY(ByteCode, length);
		btOffsets = (Int *)GC_MALLOC_ATOMIC(sizeof(Int) * length);
		if (btByteCodes == NULL || btOffsets == NULL)
			Smile_Abort_OutOfMemory();
	}
	else {
		btByteCodes = localBtByteCodes;
		btOffsets = localBtOffsets;
	}

	// Emit all of the conditionals.
	for (i = 0, temp = args; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d, i++) {
	
		// Extract off any [$not] operators, and if there were any, invert the branch below.
		condition = temp->a;
		not = StripNots(&condition);

		// Compile the next expression.
		Compiler_CompileExpr(compiler, temp->a);

		// If truthy, branch to result in 'true'.
		btOffsets[i] = segment->numByteCodes;
		btByteCodes[i] = ByteCodeSegment_Emit(segment, not ? Op_Bf : Op_Bt);
	
		// It's truthy, so keep going.
	}

	// We failed all the tests, so the result is false.
	ByteCodeSegment_Emit(segment, Op_LdBool)->u.boolean = False;
	jmpOffset = segment->numByteCodes;
	jmpByteCode = ByteCodeSegment_Emit(segment, Op_Jmp);

	// Now handle the truthy case.
	trueOffset = segment->numByteCodes;
	ByteCodeSegment_Emit(segment, Op_LdBool)->u.boolean = True;

	// Add a branch target for the jump.
	jmpLabelOffset = segment->numByteCodes;
	jmpLabelByteCode = ByteCodeSegment_Emit(segment, Op_Label);

	// Now fill in all the branch deltas for the conditional branches.
	for (i = 0; i < length; i++) {
		btByteCodes[i]->u.index = trueOffset - btOffsets[i];
	}

	// And fill in the branch delta for the unconditional branch.
	jmpDelta = jmpLabelOffset - jmpOffset;
	jmpByteCode->u.index = jmpDelta;
	jmpLabelByteCode->u.index = -jmpDelta;
}

static void Compiler_CompileNot(Compiler compiler, SmileList args)
{
	// Must be an expression of the form [$not x].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [not]: Expression is not well-formed.")));
		return;
	}

	// Compile the expression.
	Compiler_CompileExpr(compiler, args->a);

	// Add an instruction to convert to boolean and then invert the result.
	ByteCodeSegment_Emit(compiler->currentFunction->byteCodeSegment, Op_Not);
}
