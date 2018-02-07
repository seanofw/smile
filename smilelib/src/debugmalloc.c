#include <smile.h>

#ifdef SMILE_SUPER_DETAILED_MEMORY_DEBUG

#include <stdio.h>
#include <malloc.h>

static Byte *_basePtr = NULL;
static Byte *_stackTop = NULL;
static Byte *_stackEnd = NULL;

// 48 bytes on all machines.
typedef struct HeaderStruct {
	Byte sig1[8];			// 0x8B AD F0 0D DE AD BE EF
	UInt32 physicalSize;	// Whole block, including header
	UInt32 logicalSize;		// Logical size (size from caller), to detect overruns
	UInt32 line;
	UInt32 reserved;		// Always zeros
	union {
		const char *fileName;
		Byte padding[8];
	} f;
	union {
		const char *typeName;
		Byte padding2[8];
	} t;
	Byte sig2[8];			// 0xFE EB DA ED D0 0F DA B8
} *Header;

void MemoryCheck(void)
{
	// Spin over the Headers, which should form a linked list, and check their sig1 and sig2;
	// also, check that each logical block is properly padded with 0xCC bytes.
	Header header;
	Byte *ptr = _basePtr;

	if (ptr == NULL) return;

	for (;;) {
		header = (Header)ptr;

		if (*(UInt64 *)header->sig1 != 0xEFBEADDE0DF0AD8BULL) {
			char fileName_buffer[64];
			char typeName_buffer[64];
			strcpy_s(fileName_buffer, 63, header->f.fileName);
			strcpy_s(typeName_buffer, 63, header->t.typeName);
			fileName_buffer[63] = '\0';
			typeName_buffer[63] = '\0';
			printf("Heap corruption detected: Block at %p is damaged.\n"
				"  (This is likely an overrun from the block before it.)\n"
				"  This block is of type \"%s\", allocated in file \"%s\" line %d.",
				header, typeName_buffer, fileName_buffer, (int)header->line);
		}

		if (*(UInt64 *)header->sig2 != 0xB8DA0FD0EDDAEBFEULL) {
			char fileName_buffer[64];
			char typeName_buffer[64];
			strcpy_s(fileName_buffer, 63, header->f.fileName);
			strcpy_s(typeName_buffer, 63, header->t.typeName);
			fileName_buffer[63] = '\0';
			typeName_buffer[63] = '\0';
			printf("Heap corruption detected: Block at %p is damaged.\n"
				"  (This is likely an underrun from its own block.)\n"
				"  This block is of type \"%s\", allocated in file \"%s\" line %d.",
				header, typeName_buffer, fileName_buffer, (int)header->line);
		}

		if (header->physicalSize == 0)
			break;

		ptr = ptr + header->physicalSize;
	}
}

void *SuperDetailedMemoryDebugAllocator(size_t size, const char *typeName, const char *fileName, Int line)
{
	Header header, tailHeader;
	Byte *physicalBlock;
	UInt32 physicalSize;
	UInt32 i;

	if (_basePtr == NULL) {
		const int HeapSize = 0x10000000; // The heap is exactly 256 megabytes and no more.

		_basePtr = malloc(HeapSize);
		_stackTop = _basePtr;
		_stackEnd = _stackTop + HeapSize;
	}

	// World's lamest allocator:  We allocate as a stack, and FAIL HARD as soon as we run
	// out of space.  This is very predictable, though, and makes debugging easier, and makes
	// it easy to insert (and check) guards on every allocated block.

	physicalSize = (sizeof(struct HeaderStruct) + size + 15) & ~15;	// Always end on a 16-byte page increment.
	if (_stackTop + physicalSize > _stackEnd)
		return NULL;	// Out of memory!

	// Should probably do this as an atomic operation or something >_>
	physicalBlock = (Byte *)_stackTop;
	_stackTop += physicalSize;

	// Make the header.
	header = (Header)physicalBlock;

	memset(header, 0, sizeof(struct HeaderStruct));

	header->sig1[0] = 0x8B;
	header->sig1[1] = 0xAD;
	header->sig1[2] = 0xF0;
	header->sig1[3] = 0x0D;
	header->sig1[4] = 0xDE;
	header->sig1[5] = 0xAD;
	header->sig1[6] = 0xBE;
	header->sig1[7] = 0xEF;

	header->sig2[0] = 0xFE;
	header->sig2[1] = 0xEB;
	header->sig2[2] = 0xDA;
	header->sig2[3] = 0xED;
	header->sig2[4] = 0xD0;
	header->sig2[5] = 0x0F;
	header->sig2[6] = 0xDA;
	header->sig2[7] = 0xB8;

	header->f.fileName = fileName;
	header->t.typeName = typeName;
	header->reserved = 0;
	header->line = line;
	header->physicalSize = physicalSize;
	header->logicalSize = size;

	// Zero out the part we're returning.
	memset(physicalBlock + sizeof(struct HeaderStruct), 0, size);

	// Fill the padding space with detectable useless fluff bytes.
	for (i = sizeof(struct HeaderStruct) + size; i < physicalSize; i++) {
		physicalBlock[i] = 0xCC;
	}

	// Make sure there's at least part of a header to follow this one.
	tailHeader = (Header)_stackTop;

	memset(tailHeader, 0, sizeof(struct HeaderStruct));

	tailHeader->sig1[0] = 0x8B;
	tailHeader->sig1[1] = 0xAD;
	tailHeader->sig1[2] = 0xF0;
	tailHeader->sig1[3] = 0x0D;
	tailHeader->sig1[4] = 0xDE;
	tailHeader->sig1[5] = 0xAD;
	tailHeader->sig1[6] = 0xBE;
	tailHeader->sig1[7] = 0xEF;

	tailHeader->sig2[0] = 0xFE;
	tailHeader->sig2[1] = 0xEB;
	tailHeader->sig2[2] = 0xDA;
	tailHeader->sig2[3] = 0xED;
	tailHeader->sig2[4] = 0xD0;
	tailHeader->sig2[5] = 0x0F;
	tailHeader->sig2[6] = 0xDA;
	tailHeader->sig2[7] = 0xB8;

	// Return the caller its memory.
	return physicalBlock + sizeof(struct HeaderStruct);
}

void *AnnoyinglyHaveToSupportGcRealloc(void *ptr, size_t newSize, const char *fileName, int line)
{
	Byte *physicalBlock = (Byte *)ptr - sizeof(struct HeaderStruct);
	Header header = (Header)physicalBlock;

	Byte *newBuffer = SuperDetailedMemoryDebugAllocator(newSize, header->t.typeName, fileName, line);
	MemCpy(newBuffer, ptr, header->logicalSize);

	return newBuffer;
}

#endif
