
// 00-0F
Op(Nop), Op(Dup1), Op(Dup2), Op(Dup), NULL, Op(Pop1), Op(Pop2), Op(Pop),
NULL, Op(Rep1), Op(Rep2), Op(Rep), NULL, NULL, NULL, Op(Brk),

// 10-1F
Op(LdNull), Op(LdBool), Op(LdStr), Op(LdSym), Op(LdObj), Op(LdClos), Op(LdChar), Op(LdUni),
Op(Ld8), Op(Ld16), Op(Ld32), Op(Ld64), Op(Ld128), NULL, NULL, NULL,

// 20-2F
NULL, Op(LdR16), Op(LdR32), Op(LdR64), Op(LdR128), NULL, NULL, NULL,
NULL, Op(LdF16), Op(LdF32), Op(LdF64), Op(LdF128), NULL, NULL, NULL,

// 30-3F
Op(LdLoc), Op(StLoc), Op(StpLoc), NULL, Op(LdArg), Op(StArg), Op(StpArg), NULL,
Op(LdX), Op(StX), Op(StpX), NULL, Op(NullLoc0), Op(NullArg0), Op(NullX), NULL,

// 40-4F
Op(LdArg0), Op(LdArg1), Op(LdArg2), Op(LdArg3), Op(LdArg4), Op(LdArg5), Op(LdArg6), Op(LdArg7),
Op(LdLoc0), Op(LdLoc1), Op(LdLoc2), Op(LdLoc3), Op(LdLoc4), Op(LdLoc5), Op(LdLoc6), Op(LdLoc7),

// 50-5F
Op(StArg0), Op(StArg1), Op(StArg2), Op(StArg3), Op(StArg4), Op(StArg5), Op(StArg6), Op(StArg7),
Op(StLoc0), Op(StLoc1), Op(StLoc2), Op(StLoc3), Op(StLoc4), Op(StLoc5), Op(StLoc6), Op(StLoc7),

// 60-6F
Op(StpArg0), Op(StpArg1), Op(StpArg2), Op(StpArg3), Op(StpArg4), Op(StpArg5), Op(StpArg6), Op(StpArg7),
Op(StpLoc0), Op(StpLoc1), Op(StpLoc2), Op(StpLoc3), Op(StpLoc4), Op(StpLoc5), Op(StpLoc6), Op(StpLoc7),

// 70-7F
Op(LdProp), Op(StProp), Op(StpProp), NULL, Op(LdMember), Op(StMember), Op(StpMember), NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

// 80-8F
Op(Cons), Op(Car), Op(Cdr), NULL, Op(NewPair), Op(Left), Op(Right), NULL,
Op(NewFn), Op(NewObj), NULL, Op(SuperEq), Op(SuperNe), Op(Not), Op(Is), Op(TypeOf),

// 90-9F
Op(Call0), Op(Call1), Op(Call2), Op(Call3), Op(Call4), Op(Call5), Op(Call6), Op(Call7),
Op(Unary), Op(Binary), Op(Met2), Op(Met3), Op(Met4), Op(Met5), Op(Met6), Op(Met7),

// A0-AF
Op(TCall0), Op(TCall1), Op(TCall2), Op(TCall3), Op(TCall4), Op(TCall5), Op(TCall6), Op(TCall7),
Op(TUnary), Op(TBinary), Op(TMet2), Op(TMet3), Op(TMet4), Op(TMet5), Op(TMet6), Op(TMet7),

// B0-BF
Op(Jmp), Op(Bt), Op(Bf), NULL, Op(Met), Op(TMet), Op(Call), Op(TCall),
Op(NewTill), Op(EndTill), Op(TillEsc), NULL, Op(Try), Op(EndTry), NULL, Op(Ret),

// C0-CF
Op(Add), Op(Sub), Op(Mul), Op(Div), Op(Mod), Op(Rem), NULL, Op(RangeTo),
Op(Eq), Op(Ne), Op(Lt), Op(Gt), Op(Le), Op(Ge), Op(Cmp), Op(Compare),

// D0-DF
Op(Each), Op(Map), Op(Where), NULL, Op(Count), Op(Any), Op(Join), NULL,
Op(UCount), Op(UAny), Op(UJoin), Op(Neg), Op(Bool), Op(Int), Op(String), Op(Hash),

// E0-EF
Op(NullQ), Op(ListQ), Op(PairQ), Op(FnQ), Op(BoolQ), Op(IntQ), Op(StringQ), Op(SymbolQ),
Op(LdA), Op(LdD), Op(LdLeft), Op(LdRight), Op(LdStart), Op(LdEnd), Op(LdCount), Op(LdLength),

// F0-FF
Op(StateMachStart), Op(StateMachBody), NULL, NULL, NULL, NULL, NULL, NULL,
Op(Pseudo), NULL, NULL, NULL, NULL, Op(EndBlock), Op(Label), Op(Block),
