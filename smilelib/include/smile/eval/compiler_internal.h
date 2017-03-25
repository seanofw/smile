#ifndef __SMILE_EVAL_COMPILER_INTERNAL_H__
#define __SMILE_EVAL_COMPILER_INTERNAL_H__

#ifndef __SMILE_EVAL_COMPILER_H__
#include <smile/eval/compiler.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILELIST_H__
#include <smile/smiletypes/smilelist.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILEPAIR_H__
#include <smile/smiletypes/smilepair.h>
#endif

//-------------------------------------------------------------------------------------------------
// External functions

extern void Compiler_EmitPop1(Compiler compiler);
extern Int Compiler_SetSourceLocation(Compiler compiler, LexerPosition lexerPosition);
extern Int Compiler_SetAssignedSymbol(Compiler compiler, Symbol symbol);
extern Bool Compiler_StripNots(SmileObject *objPtr);

extern Int CompilerFunction_AddLocal(CompilerFunction compilerFunction, Symbol local);

extern void Compiler_CompileProperty(Compiler compiler, SmilePair pair, Bool store);
extern void Compiler_CompileVariable(Compiler compiler, Symbol symbol, Bool store);
extern void Compiler_CompileMethodCall(Compiler compiler, SmilePair pair, SmileList args);

extern Bool Compiler_CompileStandardForm(Compiler compiler, Symbol symbol, SmileList args);

extern void Compiler_CompileSetf(Compiler compiler, SmileList args);
extern void Compiler_CompileOpEquals(Compiler compiler, SmileList args);
extern void Compiler_CompileIf(Compiler compiler, SmileList args);
extern void Compiler_CompileWhile(Compiler compiler, SmileList args);
extern void Compiler_CompileTill(Compiler compiler, SmileList args);
extern void Compiler_CompileCatch(Compiler compiler, SmileList args);
extern void Compiler_CompileReturn(Compiler compiler, SmileList args);
extern void Compiler_CompileFn(Compiler compiler, SmileList args);
extern void Compiler_CompileQuote(Compiler compiler, SmileList args);
extern void Compiler_CompileProg1(Compiler compiler, SmileList args);
extern void Compiler_CompileProgN(Compiler compiler, SmileList args);
extern void Compiler_CompileScope(Compiler compiler, SmileList args);
extern void Compiler_CompileNew(Compiler compiler, SmileList args);
extern void Compiler_CompileAnd(Compiler compiler, SmileList args);
extern void Compiler_CompileOr(Compiler compiler, SmileList args);

//-------------------------------------------------------------------------------------------------
// Inline functions
//
// (These are things short and simple enough that the function-call overhead
// is likely more CPU time than the actual work being done by the function, so
// they're good candidates for inlining.)

/// <summary>
/// Adjust the given function's stack depth by the given amount.  This also
/// updates the function's maximum stack depth, if necessary.  (Also, since it's
/// used in macros, it returns a useless zero that the C compiler should optimize
/// away.)
/// </summary>
Inline Int ApplyStackDelta(CompilerFunction compilerFunction, Int stackDelta)
{
	compilerFunction->currentStackDepth += stackDelta;

	if (compilerFunction->currentStackDepth > compilerFunction->stackSize) {
		compilerFunction->stackSize = compilerFunction->currentStackDepth;
	}

	return 0;
}

/// <summary>
/// Set the compiler's current source location back to the given source location.
/// </summary>
Inline void Compiler_RevertSourceLocation(Compiler compiler, Int oldSourceLocation)
{
	compiler->currentFunction->currentSourceLocation = oldSourceLocation;
}

/// <summary>
/// If the given List is annotated with a source location, update the compiler's
/// current source location to be that source location.  Returns the index of the
/// previous source location in the collection of source locations.
/// </summary>
Inline Int Compiler_SetSourceLocationFromList(Compiler compiler, SmileList list)
{
	if (!(list->kind & SMILE_FLAG_WITHSOURCE))
		return compiler->currentFunction->currentSourceLocation;

	return Compiler_SetSourceLocation(compiler, ((struct SmileListWithSourceInt *)list)->position);
}

/// <summary>
/// If the given Pair is annotated with a source location, update the compiler's
/// current source location to be that source location.  Returns the index of the
/// previous source location in the collection of source locations.
/// </summary>
Inline Int Compiler_SetSourceLocationFromPair(Compiler compiler, SmilePair pair)
{
	if (!(pair->kind & SMILE_FLAG_WITHSOURCE))
		return compiler->currentFunction->currentSourceLocation;

	return Compiler_SetSourceLocation(compiler, ((struct SmilePairWithSourceInt *)pair)->position);
}


//-------------------------------------------------------------------------------------------------
// Macros
//
// (These are partial functions; they depend on certain declarations being in
// place in the functions that use them, and on certain data being set up correctly.
// Caveats, provisos, exclusions, and ipso-factos may apply.  Your mileage may vary.
// Void where prohibited by law.  NULL and void where declared NULL and void.)

/// <summary>
/// Emit the given opcode to the current segment's instruction stream, adjusting the stack's
/// count of pushed items by the given delta.  This opcode requires no (zero) operands.
/// This will also tag the new instruction with the compiler's current source location.
/// </summary>
#define EMIT0(__opcode__, __stackDelta__) \
	((offset = ByteCodeSegment_Emit(segment, (__opcode__), compiler->currentFunction->currentSourceLocation)), \
		ApplyStackDelta(compiler->currentFunction, __stackDelta__), \
		offset)

/// <summary>
/// Emit the given opcode to the current segment's instruction stream, adjusting the stack's
/// count of pushed items by the given delta.  This opcode requires one operand.
/// This will also tag the new instruction with the compiler's current source location.
/// </summary>
#define EMIT1(__opcode__, __stackDelta__, __operand1__) \
	((offset = ByteCodeSegment_Emit(segment, (__opcode__), compiler->currentFunction->currentSourceLocation)), \
		segment->byteCodes[offset].u.__operand1__, \
		ApplyStackDelta(compiler->currentFunction, __stackDelta__), \
		offset)

/// <summary>
/// Emit the given opcode to the current segment's instruction stream, adjusting the stack's
/// count of pushed items by the given delta.  This opcode requires two operands.
/// This will also tag the new instruction with the compiler's current source location.
/// </summary>
#define EMIT2(__opcode__, __stackDelta__, __operand1__, __operand2__) \
	((offset = ByteCodeSegment_Emit(segment, (__opcode__), compiler->currentFunction->currentSourceLocation)), \
		segment->byteCodes[offset].u.__operand1__, \
		segment->byteCodes[offset].u.__operand2__, \
		ApplyStackDelta(compiler->currentFunction, __stackDelta__), \
		offset)

/// <summary>
/// Go back and correct the instruction at the given absolute offset in the current segment's
/// instruction stream to have the given index value.  This is ususally used for branch instructions,
/// to correct their branch deltas to the given amount after their target label has been emitted.
/// </summary>
#define FIX_BRANCH(__offset__, __index__) \
	(segment->byteCodes[(__offset__)].u.index = (__index__))

/// <summary>
/// Locate a recently-emitted bytecode, at the given delta.  For example, RECENT_BYTECODE(-1)
/// is the most-recently-emitted bytecode; RECENT_BYTECODE(-2) is the one before it, and so
/// on.  This results in the correct 'struct ByteCodeStruct' (not a pointer) of the instruction.
/// </summary>
#define RECENT_BYTECODE(__delta__) (segment->byteCodes[segment->numByteCodes + (__delta__)])

#endif
