#ifndef STDIOWRAPPER_H
#define STDIOWRAPPER_H

#include <stdio.h>

#define fopen wfopen
#define fclose wfclose
#define fread wfread
#define fwrite wfwrite
#define fseek wfseek
#define ftell wftell
#define fflush wfflush
#define fgetc wfgetc
#define fputc wfputc
#define fgets wfgets
#define fputs wfputs
#define fprintf wfprintf
#define fscanf wfscanf
#define ungetc wungetc
#define rewind wrewind
#define remove wremove
#define rename wrename
#define tmpfile wtmpfile
#define tmpnam wtmpnam
#define setvbuf wsetvbuf
#define clearerr wclearerr
#define feof wfeof
#define ferror wferror
#define perror wperror
#define fileno wfileno
#define FILE wFILE

struct wFILE {
};

wFILE* wfopen(const char* filename, const char* mode);
void wfclose(wFILE* file);
int wfread(void* ptr, size_t size, size_t count, wFILE* file);
int wfwrite(const void* ptr, size_t size, size_t count, wFILE* file);
int wfseek(wFILE* file, long offset, int whence);
long wftell(wFILE* file);
int wfflush(wFILE* file);
int wfgetc(wFILE* file);
int wfputc(int c, wFILE* file);
int wfgets(char* str, int num, wFILE* file);
int wfputs(const char* str, wFILE* file);
int wfprintf(wFILE* file, const char* format, ...);
int wfscanf(wFILE* file, const char* format, ...);
int wungetc(int c, wFILE* file);
void wrewind(wFILE* file);
int wremove(const char* filename);
int wrename(const char* oldname, const char* newname);
int wtmpnam(char* str);
int wsetvbuf(wFILE* file, char* buf, int mode, size_t size);
void wclearerr(wFILE* file);
int wfeof(wFILE* file);
int wferror(wFILE* file);
void wperror(const char* s);
int wfileno(wFILE* file);

#endif // STDIOWRAPPER_H
