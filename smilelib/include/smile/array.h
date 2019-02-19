
#ifndef __SMILE_ARRAY_H__
#define __SMILE_ARRAY_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_GC_H__
#include <smile/gc.h>
#endif

#ifndef __SMILE_MEM_H__
#include <smile/mem.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// The implementation of a "managed" array, which is a bit smarter than just a pointer to data.
/// </summary>
typedef struct ArrayStruct
{
	void *data;	// The buffer that actually stores each item.
	UInt16 itemSize;	// The uniform size of each item in the data[] buffer.
	Bool isAtomic;	// Whether this uses atomic allocation for the data[] buffer.
	Int length;	// The current apparent length of the data[] buffer.
	Int _max;	// Always the size of the actual data[] buffer, as a count of itemSize.
} *Array;

#define ARRAY_AT(__array__, __index__, __type__) ((__type__ *)((Byte *)(__array__)->data + (__array__)->itemSize * (__index__)))

Inline void Array_Init(Array array, UInt16 itemSize, Int max, Bool isAtomic)
{
	array->data = isAtomic ? GC_MALLOC_ATOMIC(itemSize * max) : GC_MALLOC(itemSize * max);
	if (array->data == NULL) Smile_Abort_OutOfMemory();
	array->itemSize = itemSize;
	array->isAtomic = isAtomic;
	array->length = 0;
	array->_max = max;
}

Inline Array Array_Create(UInt16 itemSize, Int max, Bool isAtomic)
{
	Array array = (Array)GC_MALLOC_STRUCT(struct ArrayStruct);
	if (array == NULL) Smile_Abort_OutOfMemory();
	Array_Init(array, itemSize, max, isAtomic);
	return array;
}

Inline void Array_Resize(Array array, Int newMax)
{
	Int copyMax = newMax < array->_max ? newMax : array->_max;
	void *newData = array->isAtomic ? GC_MALLOC_ATOMIC(array->itemSize * newMax) : GC_MALLOC(array->itemSize * newMax);
	if (newData == NULL) Smile_Abort_OutOfMemory();
	MemCpy(newData, array->data, array->itemSize * copyMax);
	array->data = newData;
	array->_max = newMax;
}

Inline void *Array_At(Array array, Int index)
{
	return ((Byte *)array->data) + index * array->itemSize;
}

Inline Int Array_Length(Array array)
{
	return array->length;
}

Inline void *Array_Push(Array array)
{
	if (array->length >= array->_max)
		Array_Resize(array, array->_max * 2);
	return Array_At(array, array->length++);
}

Inline void Array_Pop(Array array)
{
	if (array->length > 0) array->length--;
}

#endif
