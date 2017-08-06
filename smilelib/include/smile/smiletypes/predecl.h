#ifndef __SMILE_SMLIETYPES_PREDECL_H__
#define __SMILE_SMLIETYPES_PREDECL_H__

typedef struct LexerPositionStruct *LexerPosition;

typedef struct SmileVTableInt *SmileVTable;

typedef struct SmileObjectInt *SmileObject;

typedef struct SmileHandleInt *SmileHandle;

typedef struct SmileListInt *SmileList;
typedef struct SmileListInt *SmileNull;
typedef struct SmilePairInt *SmilePair;

typedef struct SmileFunctionInt *SmileFunction;
typedef struct SmileUserObjectInt *SmileUserObject;

typedef struct SmileBoolInt *SmileBool;
typedef struct SmileSymbolInt *SmileSymbol;

typedef struct SmileByteInt *SmileByte;
typedef struct SmileInteger16Int *SmileInteger16;
typedef struct SmileInteger32Int *SmileInteger32;
typedef struct SmileInteger64Int *SmileInteger64;
typedef struct SmileInteger128Int *SmileInteger128;

typedef struct SmileUnboxedByteInt *SmileUnboxedByte;
typedef struct SmileUnboxedInteger16Int *SmileUnboxedInteger16;
typedef struct SmileUnboxedInteger32Int *SmileUnboxedInteger32;
typedef struct SmileUnboxedInteger64Int *SmileUnboxedInteger64;
typedef struct SmileUnboxedBoolInt *SmileUnboxedBool;
typedef struct SmileUnboxedSymbolInt *SmileUnboxedSymbol;

typedef struct SmileSyntaxInt *SmileSyntax;
typedef struct SmileNonterminalInt *SmileNonterminal;

typedef struct SmileByteArrayInt *SmileByteArray;

typedef struct ParseMessageStruct *ParseMessage;
typedef struct EvalResultStruct *EvalResult;
typedef struct ClosureInfoStruct *ClosureInfo;
typedef struct ClosureStruct *Closure;
typedef struct ByteCodeSegmentStruct *ByteCodeSegment;
typedef struct ByteCodeStruct *ByteCode;

typedef struct LibraryInfoStruct *LibraryInfo;

#endif
