// debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

// Global variable to hold current debug level (0 means no debug output).
extern int g_debug_level;

// A macro to print debug messages.
// It will only print if the current debug level is greater than or equal to 'level'.
#define DEBUG_PRINT(level, fmt, ...)         \
    do {                                     \
        if (g_debug_level >= (level)) {      \
            fprintf(stderr, "[DEBUG][L%d] " fmt "\n", level, ##__VA_ARGS__); \
        }                                    \
    } while (0)

#endif // DEBUG_H

