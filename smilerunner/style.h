#ifndef __SMILERUNNER_STYLE_H__
#define __SMILERUNNER_STYLE_H__

#include <stdio.h>

size_t fwrite_styled(const char *string, size_t size, size_t count, FILE *fp);
size_t fprintf_styled(FILE *fp, const char *format, ...);
size_t vfprintf_styled(FILE *fp, const char *format, va_list v);
size_t printf_styled(const char *format, ...);
size_t vprintf_styled(const char *format, va_list v);

#endif
