#include <string.h>
#include <stdlib.h>
#include "string_utils.h"

char* str_replace(char* original, char* old_part, char* new_part) {
    char* result;
    char* pos;
    int old_len = strlen(old_part);
    int new_len = strlen(new_part);
    
    pos = strstr(original, old_part);
    if (pos == NULL) return original;
    
    result = malloc(strlen(original) - old_len + new_len + 1);
    
    strncpy(result, original, pos - original);
    result[pos - original] = '\0';
    strcat(result, new_part);
    strcat(result, pos + old_len);
    
    return result;
}