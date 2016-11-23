
// 00-0F
Op(Nop), Op(Dup1), Op(Dup2), Op(Dup), NULL, Op(Pop1), Op(Pop2), Op(Pop),
NULL, NULL, NULL, NULL, NULL, NULL, NULL, Op(Brk),

// 10-1F
Op(LdNull), Op(LdBool), Op(LdCh), Op(LdUCh), Op(LdStr), Op(LdSym), Op(LdObj), Op(LdClos),
Op(Ld8), Op(Ld16), Op(Ld32), Op(Ld64), Op(Ld128), NULL, NULL, NULL,

// 20-2F
NULL, Op(LdR16), Op(LdR32), Op(LdR64), Op(LdR128), NULL, NULL, NULL,
NULL, Op(LdF16), Op(LdF32), Op(LdF64), Op(LdF128), NULL, NULL, NULL,

// 30-3F
Op(LdLoc), Op(StLoc), Op(LdArg), Op(StArg), Op(LdX), Op(StX), NULL, NULL,
Op(LdProp), Op(StProp), Op(LdMember), Op(StMember), NULL, NULL, NULL, NULL,

// 40-4F
Op(LdArg0), Op(LdArg1), Op(LdArg2), Op(LdArg3), Op(LdArg4), Op(LdArg5), Op(LdArg6), Op(LdArg7),
Op(LdLoc0), Op(LdLoc1), Op(LdLoc2), Op(LdLoc3), Op(LdLoc4), Op(LdLoc5), Op(LdLoc6), Op(LdLoc7),

// 50-5F
Op(StArg0), Op(StArg1), Op(StArg2), Op(StArg3), Op(StArg4), Op(StArg5), Op(StArg6), Op(StArg7),
Op(StLoc0), Op(StLoc1), Op(StLoc2), Op(StLoc3), Op(StLoc4), Op(StLoc5), Op(StLoc6), Op(StLoc7),

// 60-6F
Op(Jmp), Op(Bt), Op(Bf), NULL, Op(Met), Op(Call), Op(CallEsc), Op(CallTail),
NULL, NULL, NULL, NULL, Op(LocalAlloc), Op(LocalFree), Op(Args), Op(Ret),

// 70-7F
Op(Call0), Op(Call1), Op(Call2), Op(Call3), Op(Call4), Op(Call5), Op(Call6), Op(Call7),
Op(Unary), Op(Binary), Op(Met2), Op(Met3), Op(Met4), Op(Met5), Op(Met6), Op(Met7),

// 80-8F
Op(Begin), Op(End), Op(Try), Op(EndTry), Op(JmpEsc), Op(Esc), NULL, NULL,
Op(SuperEq), Op(SuperNe), Op(Not), Op(Bool), Op(Is), Op(TypeOf), NULL, NULL,

// 90-9F
Op(Cons), Op(Car), Op(Cdr), NULL, Op(NewPair), Op(Left), Op(Right), NULL,
Op(NewFn), Op(NewObj), Op(NewRange), NULL, NULL, NULL, NULL, NULL,

// A0-AF
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

// B0-BF
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

// C0-CF
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

// D0-DF
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

// E0-EF
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

// F0-FF
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, Op(Label),
