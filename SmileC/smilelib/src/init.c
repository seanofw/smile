
#include <smile/internal/types.h>
#include <smile/gc.h>

extern void puts(const char *);
extern void exit(int);

/// <summary>
/// Whether or not the Smile runtime has been initialized yet.
/// </summary>
static Bool Smile_IsInitialized = False;

/// <summary>
/// The global hash oracle, which ensures that successive runs of the same program yield
/// different hash codes.  This ensures that people do not attempt to make assumptions about
/// the order or values of their hash codes.
/// </summary>
UInt32 Smile_HashOracle;

/// <summary>
/// Initialize the Smile runtime.  This must be performed at least once on startup.
/// </summary>
void Smile_Init(void)
{
	if (Smile_IsInitialized) return;

	GC_INIT();

	Smile_HashOracle = (UInt32)GetBaselineEntropy();
}

/// <summary>
/// Shut down the Smile runtime.  This must be performed at least once on program shutdown.
/// </summary>
void Smile_End(void)
{
}

/// <summary>
/// Abort execution of the program because the runtime has run out of memory.
/// </summary>
void Smile_Abort_OutOfMemory(void)
{
	puts("Fatal error:  Out of memory!  Aborting program.");
	exit(-1);
}