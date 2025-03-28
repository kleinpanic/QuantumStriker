#include "debug.h"

/* By default, debugging is disabled. Only critical errors (severity 0) are printed. */
int g_debug_enabled = 0;
int g_debug_level = 1;  // default level if debugging is enabled
int g_always_print = 1;

