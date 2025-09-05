#include "logger.h"

#include <time.h>
#include <stdio.h>

void logger(const char* message) {
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    char timestamp[64];
    
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("%s: %s\n", timestamp, message);
}