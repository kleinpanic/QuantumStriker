#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

/* Global flag and level – set from main */
extern int g_debug_enabled;   // 0 = off, 1 = on
extern int g_debug_level;     // expected values 1, 2, or 3

/* ANSI color codes for printing */
#define DEBUG_COLOR_ERROR   "\033[31m"  // red
#define DEBUG_COLOR_WARNING "\033[33m"  // orange/yellow
#define DEBUG_COLOR_DEBUG   "\033[34m"  // blue
#define DEBUG_COLOR_SUCCESS "\033[32m"  // green
#define DEBUG_COLOR_RESET   "\033[0m"

/*
 * DEBUG_PRINT(detail, severity, fmt, ...)
 *
 * detail: an integer (1-3) that tags the “depth” of the debug statement.
 *         Only debug statements whose detail equals g_debug_level will print.
 *
 * severity: a status code:
 *    0 = error/critical (always printed, in red)
 *    1 = warning (printed in orange)
 *    2 = debug (printed in blue)
 *    3 = success (printed in green)
 *
 * fmt, ...: standard printf-style format and arguments.
 *
 * By default (when g_debug_enabled==0) only severity 0 messages are printed.
 */
#define DEBUG_PRINT(detail, severity, fmt, ...)                \
    do {                                                       \
        if ((severity) == 0 || (g_debug_enabled && (g_debug_level) == (detail))) { \
            const char* color = "";                            \
            if ((severity) == 0) { color = DEBUG_COLOR_ERROR; }\
            else if ((severity) == 1) { color = DEBUG_COLOR_WARNING; }\
            else if ((severity) == 2) { color = DEBUG_COLOR_DEBUG; }\
            else if ((severity) == 3) { color = DEBUG_COLOR_SUCCESS; }\
            fprintf(stderr, "%s", color);                      \
            fprintf(stderr, fmt, ##__VA_ARGS__);               \
            fprintf(stderr, "%s\n", DEBUG_COLOR_RESET);        \
        }                                                      \
    } while(0)

#endif /* DEBUG_H */

