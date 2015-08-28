
#ifndef __SMILE_INTERNAL_UNICODE_H__
#define __SMILE_INTERNAL_UNICODE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

//---------------------------------------------------------------------------
// Core Unicode support: Casing, composition, and normalization.

extern const Int32 *UnicodeTables_LowercaseTable[];
extern const Int32 **UnicodeTables_LowercaseTableExtended[];
extern const Int32 UnicodeTables_LowercaseTableCount;

extern const Int32 *UnicodeTables_UppercaseTable[];
extern const Int32 **UnicodeTables_UppercaseTableExtended[];
extern const Int32 UnicodeTables_UppercaseTableCount;

extern const Int32 *UnicodeTables_TitlecaseTable[];
extern const Int32 **UnicodeTables_TitlecaseTableExtended[];
extern const Int32 UnicodeTables_TitlecaseTableCount;

extern const Int32 *UnicodeTables_CaseFoldingTable[];
extern const Int32 **UnicodeTables_CaseFoldingTableExtended[];
extern const Int32 UnicodeTables_CaseFoldingTableCount;

extern const Int32 *UnicodeTables_DecompositionTable[];
extern const Int32 **UnicodeTables_DecompositionTableExtended[];
extern const Int32 UnicodeTables_DecompositionTableCount;

extern const Byte *UnicodeTables_CanonicalCombiningClassTable[];
extern const Int UnicodeTables_CanonicalCombiningClassLookupCount;
extern const Int UnicodeTables_CanonicalCombiningClassTableCount;
extern const Int UnicodeTables_CanonicalCombiningClassByteCount;

extern Int32 Unicode_Compose(Int32 a, Int32 b, Int32 c, Int32 d);

//---------------------------------------------------------------------------
// Legacy IBM code pages.

extern const UInt16 UnicodeTables_Cp437ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToCp437Table[];
extern const Int UnicodeTables_UnicodeToCp437TableCount;

//---------------------------------------------------------------------------
// Legacy Microsoft code pages.

extern const UInt16 UnicodeTables_Windows1250ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1250Table[];
extern const Int UnicodeTables_UnicodeToWindows1250TableCount;

extern const UInt16 UnicodeTables_Windows1251ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1251Table[];
extern const Int UnicodeTables_UnicodeToWindows1251TableCount;

extern const UInt16 UnicodeTables_Windows1252ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1252Table[];
extern const Int UnicodeTables_UnicodeToWindows1252TableCount;

extern const UInt16 UnicodeTables_Windows1253ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1253Table[];
extern const Int UnicodeTables_UnicodeToWindows1253TableCount;

extern const UInt16 UnicodeTables_Windows1254ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1254Table[];
extern const Int UnicodeTables_UnicodeToWindows1254TableCount;

extern const UInt16 UnicodeTables_Windows1255ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1255Table[];
extern const Int UnicodeTables_UnicodeToWindows1255TableCount;

extern const UInt16 UnicodeTables_Windows1256ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1256Table[];
extern const Int UnicodeTables_UnicodeToWindows1256TableCount;

extern const UInt16 UnicodeTables_Windows1257ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1257Table[];
extern const Int UnicodeTables_UnicodeToWindows1257TableCount;

extern const UInt16 UnicodeTables_Windows1258ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToWindows1258Table[];
extern const Int UnicodeTables_UnicodeToWindows1258TableCount;

//---------------------------------------------------------------------------
// Legacy ISO code pages.

extern const UInt16 UnicodeTables_Iso_8859_1ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_1Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_1TableCount;

extern const UInt16 UnicodeTables_Iso_8859_2ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_2Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_2TableCount;

extern const UInt16 UnicodeTables_Iso_8859_3ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_3Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_3TableCount;

extern const UInt16 UnicodeTables_Iso_8859_4ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_4Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_4TableCount;

extern const UInt16 UnicodeTables_Iso_8859_5ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_5Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_5TableCount;

extern const UInt16 UnicodeTables_Iso_8859_6ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_6Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_6TableCount;

extern const UInt16 UnicodeTables_Iso_8859_7ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_7Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_7TableCount;

extern const UInt16 UnicodeTables_Iso_8859_8ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_8Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_8TableCount;

extern const UInt16 UnicodeTables_Iso_8859_9ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_9Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_9TableCount;

extern const UInt16 UnicodeTables_Iso_8859_10ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_10Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_10TableCount;

extern const UInt16 UnicodeTables_Iso_8859_11ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_11Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_11TableCount;

// For historical reasons, there's no such thing as ISO 8859-12.

extern const UInt16 UnicodeTables_Iso_8859_13ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_13Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_13TableCount;

extern const UInt16 UnicodeTables_Iso_8859_14ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_14Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_14TableCount;

extern const UInt16 UnicodeTables_Iso_8859_15ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_15Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_15TableCount;

extern const UInt16 UnicodeTables_Iso_8859_16ToUnicodeTable[];
extern const Byte *UnicodeTables_UnicodeToIso_8859_16Table[];
extern const Int UnicodeTables_UnicodeToIso_8859_16TableCount;

#endif
