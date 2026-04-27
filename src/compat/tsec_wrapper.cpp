#include <cstdlib>
#include <cstring>

extern "C" {
    int TSecOpenChannel(void) { return 1; }
    int TSecCloseChannel(int handle) { return 0; }
    void* TSecMemAlloc(unsigned int size) { return malloc(size); }
    int TSecMemWrite(void* dst, const void* src, unsigned int size) { memcpy(dst, src, size); return 0; }
    int TSecMemFree(void* ptr) { free(ptr); return 0; }
    int TSecInitStream(int handle, void* params) { return 0; }
    int TSecStartStreaming(int handle) { return 0; }
    int TSecWriteToStream(int handle, void* buf, unsigned int size) { return 0; }
    int TSecWriteToStreamReloc(int handle, void* buf, unsigned int size, unsigned int reloc) { return 0; }
    int TSecStopStreamingAndFlush(int handle) { return 0; }
    int TSecStreamWait(int handle, int timeout) { return 0; }
    int TSecCloseStream(int handle) { return 0; }
} 
