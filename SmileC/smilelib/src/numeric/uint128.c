
SMILE_API_FUNC Bool UInt128_TryParse(String str, UInt128 *result);
SMILE_API_FUNC String UInt128_ToString(UInt128 value, Int minDigits);
SMILE_API_FUNC UInt128 UInt128_ShiftLeft(UInt128 value, Int amount);
SMILE_API_FUNC UInt128 UInt128_ShiftRight(UInt128 value, Int amount);
SMILE_API_FUNC UInt128 UInt128_Mul(UInt128 a, UInt128 b);
SMILE_API_FUNC void UInt128_DivMod(UInt128 a, UInt128 b, UInt128 *div, UInt128 *mod);

//---------------------------------------------------------------------------
// Type conversion

Inline UInt128 UInt128_FromUInt32(UInt32 i)
{
	UInt128 result;
	result.hi = 0;
	result.lo = i;
}

Inline UInt128 UInt128_FromUInt64(UInt64 i)
{
	UInt128 result;
	result.hi = 0;
	result.lo = i;
}

Inline UInt32 UInt128_ToUInt32(UInt128 i)
{
	return (UInt32)result.lo;
}

Inline UInt64 UInt128_ToUInt64(UInt128 i)
{
	return (UInt64)result.hi;
}

Inline Bool UInt128_ToBool(UInt128 value)
{
	return (value.hi | value.lo) != 0;
}

Inline Bool UInt128_LogicalNot(UInt128 value)
{
	return (value.hi | value.lo) == 0;
}

//---------------------------------------------------------------------------
// Bitwise operations

Inline UInt128 UInt128_BitNot(UInt128 a)
{
	UInt128 result;
	result.hi = ~value.hi;
	result.lo = ~value.lo;
	return result;
}

Inline UInt128 UInt128_BitAnd(UInt128 a, UInt128 b)
{
	UInt128 result;
	result.hi = a.hi & b.hi;
	result.lo = a.lo & b.lo;
	return result;
}

Inline UInt128 UInt128_BitOr(UInt128 a, UInt128 b)
{
	UInt128 result;
	result.hi = a.hi | b.hi;
	result.lo = a.lo | b.lo;
	return result;
}

Inline UInt128 UInt128_BitXor(UInt128 a, UInt128 b)
{
	UInt128 result;
	result.hi = a.hi ^ b.hi;
	result.lo = a.lo ^ b.lo;
	return result;
}

UInt128 UInt128_ShiftLeft(UInt128 value, Int amount)
{
	UInt128 result;
	
	if (amount >= 128)
		return UInt128_Zero;
	else if (amount >= 64) {
		result.hi = value.lo << (amount - 64);
		result.lo = 0;
		return result;
	}
	else {
		// Amount of 63 or less; we have to push bits from lo to hi.
		result.hi = (value.hi << amount) | (value.lo >> (64 - amount));
		result.lo = (value.lo << amount);
		return result;
	}
}

UInt128 UInt128_ShiftRight(UInt128 value, Int amount)
{
	UInt128 result;
	
	if (amount >= 128)
		return UInt128_Zero;
	else if (amount >= 64) {
		result.lo = value.hi >> (amount - 64);
		result.hi = 0;
		return result;
	}
	else {
		// Amount of 63 or less; we have to push bits from hi to lo.
		result.lo = (value.lo >> amount) | (value.hi << (64 - amount));
		result.hi = (value.hi >> amount);
		return result;
	}
}

//---------------------------------------------------------------------------
// Comparisons

Inline Bool UInt128_Equal(UInt128 a, UInt128 b)
{
	return (a.hi == b.hi && a.lo == b.lo);
}

Inline Bool UInt128_NotEqual(UInt128 a, UInt128 b)
{
	return (a.hi != b.hi || a.lo != b.lo);
}

Inline Bool UInt128_GreaterThan(UInt128 a, UInt128 b)
{
	if (a.hi != b.hi)
		return a.hi > b.hi;
	else
		return a.lo > b.lo;
}

Inline Bool UInt128_GreaterThanOrEqual(UInt128 a, UInt128 b)
{
	if (a.hi != b.hi)
		return a.hi >= b.hi;
	else
		return a.lo >= b.lo;
}

Inline Bool UInt128_LessThan(UInt128 a, UInt128 b)
{
	if (a.hi != b.hi)
		return a.hi < b.hi;
	else
		return a.lo < b.lo;
}

Inline Bool UInt128_LessThanOrEqual(UInt128 a, UInt128 b)
{
	if (a.hi != b.hi)
		return a.hi <= b.hi;
	else
		return a.lo <= b.lo;
}

Inline Int UInt128_Compare(UInt128 a, UInt128 b)
{
	if (a.hi != b.hi) {
		return a.hi < b.hi ? -1
			: a.hi > b.hi ? +1
			: 0;
	}
	else {
		return a.lo < b.lo ? -1
			: a.lo > b.lo ? +1
			: 0;
	}
}

//---------------------------------------------------------------------------
// Arithmetic

Inline UInt128 UInt128_Inc(UInt128 value)
{
	if (++value.lo == 0) ++value.hi;
	return value;
}

Inline UInt128 UInt128_Dec(UInt128 value)
{
	if (value.lo-- == 0) value.hi--;
	return value;
}

Inline UInt128 UInt128_Neg(UInt128 value)
{
	return UInt128_Inc(UInt128_BitNot(value));
}

Inline UInt128 UInt128_Add(UInt128 a, UInt128 b)
{
	UInt128 result;
	result.hi = a.hi + b.hi + ((a.lo + b.lo) < a.lo);
	result.lo = a.lo + b.lo;
	return result;
}

Inline UInt128 UInt128_Sub(UInt128 a, UInt128 b)
{
	UInt128 result;
	result.hi = a.hi - b.hi - ((a.lo - b.lo) > a.lo);
	result.lo = a.lo - b.lo;
	return result;	
}

SMILE_API_FUNC UInt128 UInt128_Mul(UInt128 a, UInt128 b)
{
	
}

SMILE_API_FUNC void UInt128_DivMod(UInt128 a, UInt128 b, UInt128 *div, UInt128 *mod)
{
	
}

Inline UInt128 UInt128_Div(UInt128 a, UInt128 b)
{
	UInt128 div, mod;
	UInt128_DivMod(a, b, &div, &mod);
	return div;
}

Inline UInt128 UInt128_Mod(UInt128 a, UInt128 b)
{
	UInt128 div, mod;
	UInt128_DivMod(a, b, &div, &mod);
	return mod;
}

//---------------------------------------------------------------------------
// Stringification

Bool UInt128_TryParse(String str, UInt128 *result)
{

}

String UInt128_ToString(UInt128 value, Int minDigits)
{
	
}
