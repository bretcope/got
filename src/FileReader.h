#ifndef GOT_FILE_H
#define GOT_FILE_H


#include <cstdint>

const uint32_t MAX_FILENAME_SIZE = 600;     // Windows limit is 260 UTF-16 characters. This more than covers that limit.

bool LoadFile(const char* filename, uint32_t maxSize, FILE* errStream, char** out_content, uint32_t* out_size);


#endif //GOT_FILE_H
