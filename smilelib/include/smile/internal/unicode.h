
#ifndef __SMILE_INTERNAL_UNICODE_H__
#define __SMILE_INTERNAL_UNICODE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

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
