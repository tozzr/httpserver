#include "file_io.h"

#include <stdlib.h>

char* read_file(const char* filename, size_t* file_size) {
    FILE* file;
    char* buffer;
    long length;
    
    file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (length < 0) {
        fclose(file);
        return NULL;
    }
    
    buffer = malloc(length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    *file_size = fread(buffer, 1, length, file);
    buffer[*file_size] = '\0';
    
    fclose(file);
    return buffer;
}
