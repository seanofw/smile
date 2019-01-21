
//---------------------------------------------------------------------------------------
//  Int32 functions

Int32 Atomic_LoadInt32(const Int32 *src)
{
	Int32 ret;
	__atomic_load(src, &ret, __ATOMIC_SEQ_CST);
	return ret;
}

void Atomic_StoreInt32(Int32 *dest, Int32 value)
{
	__atomic_store(dest, &value, __ATOMIC_SEQ_CST);
}

Int32 Atomic_AddInt32(Int32 *value, Int32 delta)
{
	return __atomic_add_fetch(value, delta, __ATOMIC_SEQ_CST);
}

Bool Atomic_CompareAndSwapInt32(Int32 *value, Int32 comparator, Int32 replacement)
{
	return (Bool)__atomic_compare_exchange_n(value, &comparator, replacement, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

Int32 Atomic_SwapInt32(Int32 *value, Int32 replacement)
{
	return __atomic_exchange_n(value, replacement, __ATOMIC_SEQ_CST);
}

//---------------------------------------------------------------------------------------
//  Int64 functions

Int64 Atomic_LoadInt64(const Int64 *src)
{
	Int64 ret;
	__atomic_load(src, &ret, __ATOMIC_SEQ_CST);
	return ret;
}

void Atomic_StoreInt64(Int64 *dest, Int64 value)
{
	__atomic_store(dest, &value, __ATOMIC_SEQ_CST);
}

Int64 Atomic_AddInt64(Int64 *value, Int64 delta)
{
	return __atomic_add_fetch(value, delta, __ATOMIC_SEQ_CST);
}

Bool Atomic_CompareAndSwapInt64(Int64 *value, Int64 comparator, Int64 replacement)
{
	return (Bool)__atomic_compare_exchange_n(value, &comparator, replacement, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

Int64 Atomic_SwapInt64(Int64 *value, Int64 replacement)
{
	return __atomic_exchange_n(value, replacement, __ATOMIC_SEQ_CST);
}

//---------------------------------------------------------------------------------------
//  Pointer functions

void *Atomic_LoadPointer(const void **src)
{
	void *ret;
	__atomic_load(src, &ret, __ATOMIC_SEQ_CST);
	return ret;
}

void Atomic_StorePointer(void **dest, const void *value)
{
	__atomic_store(dest, &value, __ATOMIC_SEQ_CST);
}

void *Atomic_AddPointer(void **value, Int delta)
{
	return __atomic_add_fetch(value, delta, __ATOMIC_SEQ_CST);
}

Bool Atomic_CompareAndSwapPointer(void **value, const void *comparator, const void *replacement)
{
	return (Bool)__atomic_compare_exchange_n(value, &comparator, replacement, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

void *Atomic_SwapPointer(void **value, const void *replacement)
{
	return __atomic_exchange_n(value, replacement, __ATOMIC_SEQ_CST);
}
