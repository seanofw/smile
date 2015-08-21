#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
	#include <mpir.h>
	typedef long Int32;
	typedef unsigned long UInt32;
	typedef __int64 Int64;
	typedef unsigned __int64 UInt64;
	typedef float Real32;
	typedef double Real64;
#elif defined(__GNUC__)
	#include <gmp.h>
	typedef long Int32;
	typedef unsigned long UInt32;
	typedef long long Int64;
	typedef unsigned long long UInt64;
	typedef float Real32;
	typedef double Real64;
#endif

void PrintMultipliers(int numericBase)
{
	char digitBuffer[1024];
	int power = 0;
	mpz_t fullValue;
	double d;

	mpz_init(fullValue);
	mpz_set_ui(fullValue, 1);

	while (power <= 64) {
		if ((power & 3) == 0) {
			printf("\n\t");
		}
		mpz_mul_ui(fullValue, fullValue, numericBase);
		d = mpz_get_d(fullValue);
		power++;
		printf("0x%016lXULL, ", *(UInt64 *)&d);
	}

	mpz_clear(fullValue);

	printf("\n");
}

int main(int argc, char **argv)
{
	int i;

	printf("// This file was automatically generated.  Do not edit!\n\n");

	for (i = 1; i <= 36; i++) {
		printf("static const UInt64 _realMultipliers%d[] = {", i);
		if (i > 1) {
			PrintMultipliers(i);
		}
		else {
			printf("\n\t0\n");
		}
		printf("};\n\n");
	}

	printf("static const Real64 *_realMultipliersByBase[37] = {\n\tNULL,\n");

	for (i = 1; i <= 36; i += 4) {
		printf("\t(Real64 *)_realMultipliers%d, (Real64 *)_realMultipliers%d, (Real64 *)_realMultipliers%d, (Real64 *)_realMultipliers%d,\n", i, i+1, i+2, i+3);
	}

	printf("};\n");

	return 0;
}
