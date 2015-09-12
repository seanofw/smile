#ifdef _WIN32

# define fprintf(...) _fprintf_w32(__VA_ARGS__)
# define printf(...) _fprintf_w32(stdout, __VA_ARGS__)
# define fputs(fp, x) _fprintf_w32(fp, x);
# define puts(x) _fprintf_w32(stdout, x);

extern int __write_w32(FILE* fp, const char* buf);
extern int _fprintf_w32(FILE* fp, const char* format, ...);
extern int _fputs_w32(FILE* fp, const char* s);

#endif