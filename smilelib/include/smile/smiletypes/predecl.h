#ifndef __SMILE_SMILETYPES_PREDECL_H__
#define __SMILE_SMILETYPES_PREDECL_H__

typedef struct LexerPositionStruct *LexerPosition;

typedef struct SmileVTableInt *SmileVTable;

typedef struct SmileObjectInt *SmileObject;

typedef struct SmileHandleInt *SmileHandle;

typedef struct SmileListInt *SmileList;
typedef struct SmileListInt *SmileNull;

typedef struct SmileFunctionInt *SmileFunction;
typedef struct SmileUserObjectInt *SmileUserObject;

typedef struct SmileBoolInt *SmileBool;
typedef struct SmileSymbolInt *SmileSymbol;
typedef struct SmileCharInt *SmileChar;
typedef struct SmileUniInt *SmileUni;

typedef struct SmileByteInt *SmileByte;
typedef struct SmileInteger16Int *SmileInteger16;
typedef struct SmileInteger32Int *SmileInteger32;
typedef struct SmileInteger64Int *SmileInteger64;
typedef struct SmileTimestampInt *SmileTimestamp;

typedef struct SmileUnboxedByteInt *SmileUnboxedByte;
typedef struct SmileUnboxedInteger16Int *SmileUnboxedInteger16;
typedef struct SmileUnboxedInteger32Int *SmileUnboxedInteger32;
typedef struct SmileUnboxedInteger64Int *SmileUnboxedInteger64;

typedef struct SmileUnboxedBoolInt *SmileUnboxedBool;
typedef struct SmileUnboxedSymbolInt *SmileUnboxedSymbol;
typedef struct SmileUnboxedCharInt *SmileUnboxedChar;
typedef struct SmileUnboxedUniInt *SmileUnboxedUni;

typedef struct SmileReal32Int *SmileReal32;
typedef struct SmileReal64Int *SmileReal64;
typedef struct SmileReal128Int *SmileReal128;
typedef struct SmileFloat32Int *SmileFloat32;
typedef struct SmileFloat64Int *SmileFloat64;
typedef struct SmileFloat128Int *SmileFloat128;

typedef struct SmileUnboxedReal32Int *SmileUnboxedReal32;
typedef struct SmileUnboxedReal64Int *SmileUnboxedReal64;
typedef struct SmileUnboxedReal128Int *SmileUnboxedReal128;
typedef struct SmileUnboxedFloat32Int *SmileUnboxedFloat32;
typedef struct SmileUnboxedFloat64Int *SmileUnboxedFloat64;
typedef struct SmileUnboxedFloat128Int *SmileUnboxedFloat128;

typedef struct SmileByteRangeInt *SmileByteRange;
typedef struct SmileInteger16RangeInt *SmileInteger16Range;
typedef struct SmileInteger32RangeInt *SmileInteger32Range;
typedef struct SmileInteger64RangeInt *SmileInteger64Range;
typedef struct SmileReal32RangeInt *SmileReal32Range;
typedef struct SmileReal64RangeInt *SmileReal64Range;
typedef struct SmileFloat32RangeInt *SmileFloat32Range;
typedef struct SmileFloat64RangeInt *SmileFloat64Range;
typedef struct SmileCharRangeInt *SmileCharRange;
typedef struct SmileUniRangeInt *SmileUniRange;

typedef struct SmileSyntaxInt *SmileSyntax;
typedef struct SmileNonterminalInt *SmileNonterminal;
typedef struct SmileLoanwordInt *SmileLoanword;

typedef struct SmileByteArrayInt *SmileByteArray;

typedef struct EvalResultStruct *EvalResult;
typedef struct ClosureInfoStruct *ClosureInfo;
typedef struct ClosureStruct *Closure;
typedef struct ByteCodeSegmentStruct *ByteCodeSegment;
typedef struct ByteCodeStruct *ByteCode;

typedef struct ParserStruct *Parser;
typedef struct ParseScopeStruct *ParseScope;
typedef struct ParseMessageStruct *ParseMessage;
typedef struct ParseDeclStruct *ParseDecl;

typedef struct SmileTillContinuationInt *SmileTillContinuation;

typedef struct ModuleInfoStruct *ModuleInfo;

typedef struct RegexStruct *Regex;
typedef struct RegexMatchStruct *RegexMatch;

#endif
