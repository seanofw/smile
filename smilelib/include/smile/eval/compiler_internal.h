#ifndef __SMILE_EVAL_COMPILER_INTERNAL_H__
#define __SMILE_EVAL_COMPILER_INTERNAL_H__

#ifndef __SMILE_EVAL_COMPILER_H__
#include <smile/eval/compiler.h>
#endif
#ifndef __SMILE_SMILETYPES_SMILELIST_H__
#include <smile/smiletypes/smilelist.h>
#endif

//-------------------------------------------------------------------------------------------------
// Compiler functions

extern Int Compiler_SetSourceLocation(Compiler compiler, LexerPosition lexerPosition);
extern Int Compiler_SetAssignedSymbol(Compiler compiler, Symbol symbol);
extern Bool Compiler_StripNots(SmileObject *objPtr);

extern Int CompilerFunction_AddLocal(CompilerFunction compilerFunction, Symbol local);

extern CompiledBlock Compiler_CompileLoadProperty(Compiler compiler, SmileList dotArgs, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileLoadMember(Compiler compiler, SmileList indexArgs, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileLoadVariable(Compiler compiler, Symbol symbol, CompileFlags compileFlags);
extern void Compiler_CompileStoreVariable(Compiler compiler, Symbol symbol, CompileFlags compileFlags, CompiledBlock compiledBlock);
extern CompiledBlock Compiler_CompileMethodCall(Compiler compiler, SmileList dotArgs, SmileList args, CompileFlags compileFlags);

extern CompiledBlock Compiler_CompileStandardForm(Compiler compiler, Symbol symbol, SmileList args, CompileFlags compileFlags);

extern CompiledBlock Compiler_CompileSetf(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileOpEquals(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileIf(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileWhile(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileTill(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileCatch(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileReturn(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileFn(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileQuote(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileProg1(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileProgN(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileScope(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileNew(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileDot(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileIndex(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileAnd(Compiler compiler, SmileList args, CompileFlags compileFlags);
extern CompiledBlock Compiler_CompileOr(Compiler compiler, SmileList args, CompileFlags compileFlags);

extern Bool Compiler_ValidateDotArgs(Compiler compiler, SmileList dotArgs);
extern Bool Compiler_ValidateIndexArgs(Compiler compiler, SmileList indexArgs);

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
	(CompiledBlock_Emit(compiledBlock, (__opcode__), (__stackDelta__), compiler->currentFunction->currentSourceLocation))

/// <summary>
/// Emit the given opcode to the current segment's instruction stream, adjusting the stack's
/// count of pushed items by the given delta.  This opcode requires one operand.
/// This will also tag the new instruction with the compiler's current source location.
/// </summary>
#define EMIT1(__opcode__, __stackDelta__, __operand1__) \
	((instr = CompiledBlock_Emit(compiledBlock, (__opcode__), (__stackDelta__), compiler->currentFunction->currentSourceLocation)), \
		(instr->u.__operand1__), \
		instr)

/// <summary>
/// Emit the given opcode to the current segment's instruction stream, adjusting the stack's
/// count of pushed items by the given delta.  This opcode requires two operands.
/// This will also tag the new instruction with the compiler's current source location.
/// </summary>
#define EMIT2(__opcode__, __stackDelta__, __operand1__, __operand2__) \
	((instr = CompiledBlock_Emit(compiledBlock, (__opcode__), (__stackDelta__), compiler->currentFunction->currentSourceLocation)), \
		(instr->u.__operand1__), \
		(instr->u.__operand2__), \
		instr)

//-------------------------------------------------------------------------------------------------
// Inline functions
//
// (These are things short and simple enough that the function-call overhead
// is likely more CPU time than the actual work being done by the function, so
// they're good candidates for inlining.)

/// <summary>
/// The previous operation resulted in an object on the stack, but if one wasn't desired, then
/// we have a little cleanup.  If necessary, this emits an Op_Pop1, and then produces a suitable
/// CompileResult describing whatever the resulting data looks like.
/// </summary>
Inline void Compiler_PopIfNecessary(Compiler compiler, CompiledBlock compiledBlock, CompileFlags compileFlags)
{
	if (compileFlags & COMPILE_FLAG_NORESULT) {
		// The previous operation couldn't avoid emitting output, so just pop it.
		EMIT0(Op_Pop1, -1);
	}
}

/// <summary>
/// If the previous operation resulted in unwanted content on the stack, get rid of it by popping it.
/// </summary>
Inline void Compiler_EmitNoResult(Compiler compiler, CompiledBlock compiledBlock)
{
	if (compiledBlock->finalStackDelta != 0) {
		EMIT0(Op_Pop1, -1);
	}
}

/// <summary>
/// Force there to be at least a null on the stack, even if the previous operation didn't
/// emit any output.
/// </summary>
Inline void Compiler_EmitRequireResult(Compiler compiler, CompiledBlock compiledBlock)
{
	if (compiledBlock->finalStackDelta == 0) {
		EMIT0(Op_LdNull, +1);
	}
}

Inline void Compiler_MakeStackMatchCompileFlags(Compiler compiler, CompiledBlock compiledBlock, CompileFlags compileFlags)
{
	if (compileFlags & COMPILE_FLAG_NORESULT) {
		Compiler_EmitNoResult(compiler, compiledBlock);
	}
	else {
		Compiler_EmitRequireResult(compiler, compiledBlock);
	}
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

#endif
