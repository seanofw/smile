#include "stdafx.h"

extern void RunAllTests();

int main()
{
	char buffer[100];

	Smile_Init();

	RunAllTests();

	printf("Press Enter to exit. ");
	fgets(buffer, 99, stdin);

	Smile_End();

	return 0;
}

#include "testsuites.generated.inc"
