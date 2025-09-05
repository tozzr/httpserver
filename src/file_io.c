#include <stdio.h>

char* read_file(const char* filename, size_t* file_size) {
    FILE* file;
    char* buffer;
    long length;
    
    file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    
    /* Dateigröße ermitteln */
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (length < 0) {
        fclose(file);
        return NULL;
    }
    
    /* Speicher allokieren */
    buffer = malloc(length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    /* Datei lesen */
    *file_size = fread(buffer, 1, length, file);
    buffer[*file_size] = '\0';
    
    fclose(file);
    return buffer;
}
