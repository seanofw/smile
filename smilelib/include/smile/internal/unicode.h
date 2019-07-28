
#ifndef __SMILE_INTERNAL_UNICODE_H__
#define __SMILE_INTERNAL_UNICODE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

//---------------------------------------------------------------------------
// Core Unicode support: General Category Lookup.

enum {
	// Normative Categories
	GeneralCategory_Cn = 0x00,
	GeneralCategory_Cc = 0x01,
	GeneralCategory_Cf = 0x02,
	GeneralCategory_Cs = 0x03,
	GeneralCategory_Co = 0x04,
	GeneralCategory_Lu = 0x11,
	GeneralCategory_Ll = 0x12,
	GeneralCategory_Lt = 0x13,
	GeneralCategory_Mn = 0x21,
	GeneralCategory_Mc = 0x22,
	GeneralCategory_Me = 0x23,
	GeneralCategory_Nd = 0x31,
	GeneralCategory_Nl = 0x32,
	GeneralCategory_No = 0x33,
	GeneralCategory_Zs = 0x41,
	GeneralCategory_Zl = 0x42,
	GeneralCategory_Zp = 0x43,

	// Informative Categories
	GeneralCategory_Lm = 0x19,
	GeneralCategory_Lo = 0x1A,
	GeneralCategory_Pc = 0x59,
	GeneralCategory_Pd = 0x5A,
	GeneralCategory_Ps = 0x5B,
	GeneralCategory_Pe = 0x5C,
	GeneralCategory_Pi = 0x5D,
	GeneralCategory_Pf = 0x5E,
	GeneralCategory_Po = 0x5F,
	GeneralCategory_Sm = 0x69,
	GeneralCategory_Sc = 0x6A,
	GeneralCategory_Sk = 0x6B,
	GeneralCategory_So = 0x6C,

	// 00 family: Others
	// 1x family: Letters
	// 2x family: Marks
	// 3x family: Numbers
	// 4x family: Separators
	// 5x family: Punctuation
	// 6x family: Spacing

	// 0..7 range: Normative
	// 8..F range: Informative
};

typedef struct {
	UInt16 paragraphID;
	UInt16 offset;
} Unicode_ExtendedTuple;

SMILE_INTERNAL_DATA extern const SByte Unicode_GeneralCategoryData[];
SMILE_INTERNAL_DATA extern const UInt16 Unicode_GeneralCategoryBmpLookup[];
SMILE_INTERNAL_DATA extern const Unicode_ExtendedTuple Unicode_GeneralCategoryExtendedLookup[];
SMILE_INTERNAL_DATA extern const Int Unicode_GeneralCategoryExtendedLookupCount;

SMILE_API_FUNC Byte Unicode_GetGeneralCategoryExtended(UInt32 codePoint);

/// <summary>
/// Given a Unicode code point in the Basic Multilingual Plane, this uses fast lookup tables
/// to find its General Category assignment (in O(1) time).
/// </summary>
/// <param name="codePoint">A Unicode code point that must be less than or equal to U+FFFF.  Code points above U+FFFF
/// will result in reading outside known arrays (i.e., corrupt memory accesses).</param>
/// <returns>The General Category assignment for that code point.</returns>
Inline Byte Unicode_GetGeneralCategoryBmp(UInt32 codePoint)
{
	return Unicode_GeneralCategoryData[Unicode_GeneralCategoryBmpLookup[codePoint >> 4] + (codePoint & 0xF)];
}

/// <summary>
/// Given a Unicode code point, this finds its General Category assignment.  Code points in the
/// Basic Multilingual Plane will be found in O(1) time; code points outside the BMP will be found
/// in O(lg n) time.
/// </summary>
/// <param name="codePoint">A Unicode code point.</param>
/// <returns>The General Category assignment for that code point.</returns>
Inline Byte Unicode_GetGeneralCategory(UInt32 codePoint)
{
	if (codePoint <= 0x10000)
		return Unicode_GetGeneralCategoryBmp(codePoint);
	else
		return Unicode_GetGeneralCategoryExtended(codePoint);
}

//---------------------------------------------------------------------------
// Core Unicode support: Casing, composition, and normalization.

SMILE_INTERNAL_DATA extern const Int32 *UnicodeTables_LowercaseTable[];
SMILE_INTERNAL_DATA extern const Int32 **UnicodeTables_LowercaseTableExtended[];
SMILE_INTERNAL_DATA extern const Int32 UnicodeTables_LowercaseTableCount;

SMILE_INTERNAL_DATA extern const Int32 *UnicodeTables_UppercaseTable[];
SMILE_INTERNAL_DATA extern const Int32 **UnicodeTables_UppercaseTableExtended[];
SMILE_INTERNAL_DATA extern const Int32 UnicodeTables_UppercaseTableCount;

SMILE_INTERNAL_DATA extern const Int32 *UnicodeTables_TitlecaseTable[];
SMILE_INTERNAL_DATA extern const Int32 **UnicodeTables_TitlecaseTableExtended[];
SMILE_INTERNAL_DATA extern const Int32 UnicodeTables_TitlecaseTableCount;

SMILE_INTERNAL_DATA extern const Int32 *UnicodeTables_CaseFoldingTable[];
SMILE_INTERNAL_DATA extern const Int32 **UnicodeTables_CaseFoldingTableExtended[];
SMILE_INTERNAL_DATA extern const Int32 UnicodeTables_CaseFoldingTableCount;

SMILE_INTERNAL_DATA extern const Int32 *UnicodeTables_DecompositionTable[];
SMILE_INTERNAL_DATA extern const Int32 **UnicodeTables_DecompositionTableExtended[];
SMILE_INTERNAL_DATA extern const Int32 UnicodeTables_DecompositionTableCount;

SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_CanonicalCombiningClassTable[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_CanonicalCombiningClassLookupCount;
SMILE_INTERNAL_DATA extern const Int UnicodeTables_CanonicalCombiningClassTableCount;
SMILE_INTERNAL_DATA extern const Int UnicodeTables_CanonicalCombiningClassByteCount;

SMILE_INTERNAL_FUNC extern Int32 Unicode_Compose(Int32 a, Int32 b, Int32 c, Int32 d);

//---------------------------------------------------------------------------
// Legacy IBM code pages.

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Cp437ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToCp437Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToCp437TableCount;

//---------------------------------------------------------------------------
// Legacy Microsoft code pages.

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1250ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1250Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1250TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1251ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1251Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1251TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1252ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1252Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1252TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1253ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1253Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1253TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1254ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1254Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1254TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1255ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1255Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1255TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1256ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1256Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1256TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1257ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1257Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1257TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Windows1258ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToWindows1258Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToWindows1258TableCount;

//---------------------------------------------------------------------------
// Legacy ISO code pages.

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_1ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_1Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_1TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_2ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_2Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_2TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_3ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_3Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_3TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_4ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_4Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_4TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_5ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_5Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_5TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_6ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_6Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_6TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_7ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_7Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_7TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_8ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_8Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_8TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_9ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_9Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_9TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_10ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_10Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_10TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_11ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_11Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_11TableCount;

// For historical reasons, there's no such thing as ISO 8859-12.

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_13ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_13Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_13TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_14ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_14Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_14TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_15ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_15Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_15TableCount;

SMILE_INTERNAL_DATA extern const UInt16 UnicodeTables_Iso_8859_16ToUnicodeTable[];
SMILE_INTERNAL_DATA extern const Byte *UnicodeTables_UnicodeToIso_8859_16Table[];
SMILE_INTERNAL_DATA extern const Int UnicodeTables_UnicodeToIso_8859_16TableCount;

#endif
