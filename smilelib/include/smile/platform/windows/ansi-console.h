#ifndef __SMILE_PLATFORM_WINDOWS_ANSI_CONSOLE_H__
#define __SMILE_PLATFORM_WINDOWS_ANSI_CONSOLE_H__

#ifdef _WIN32

#include <smile/types.h>
#include <stdarg.h>
#include <stdio.h>

#define vfprintf(...)	vfprintf_ansi_win32(__VA_ARGS__)
#define fprintf(...)	fprintf_ansi_win32(__VA_ARGS__)
#define vprintf(...)	vprintf_ansi_win32(__VA_ARGS__)
#define printf(...)		printf_ansi_win32(__VA_ARGS__)
#define fputs(fp, x)	fputs_ansi_win32(fp, x)
#define puts(x)			puts_ansi_win32(x)

SMILE_API_FUNC size_t fwrite_ansi_win32(const void *data, size_t size, size_t count, FILE *fp);
SMILE_API_FUNC int fprintf_ansi_win32(FILE *fp, const char *format, ...);
SMILE_API_FUNC int printf_ansi_win32(const char *format, ...);
SMILE_API_FUNC int vfprintf_ansi_win32(FILE *fp, const char *format, va_list v);
SMILE_API_FUNC int vprintf_ansi_win32(const char *format, va_list v);
SMILE_API_FUNC int fputs_ansi_win32(FILE *fp, const char *s);
SMILE_API_FUNC int puts_ansi_win32(const char *s);

#endif

#endif