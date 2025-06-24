#include <Arduino.h>
#include "stdiowrapper.h"
#include "FileSystem.h"

wFILE* wfopen(const char* filename, const char* mode)
{
  wFILE* file = new wFILE();
  if (file) {
    return file;
  } else {
    return nullptr;
  }
}

void wfclose(wFILE* file)
{
  if (file) {
    delete file;
  }
}

int wfread(void* ptr, size_t size, size_t count, wFILE* file)
{
  if (!file || !ptr || size == 0 || count == 0) {
    return 0;
  }
  
  // Simulate reading data into ptr
  memset(ptr, 0, size * count); // Fill with zeros for demonstration
  return size * count; // Return number of bytes read
}
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
