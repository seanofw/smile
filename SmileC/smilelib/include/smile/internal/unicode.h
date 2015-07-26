
#ifndef __SMILE_INTERNAL_UNICODE_H__
#define __SMILE_INTERNAL_UNICODE_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

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

#endif
