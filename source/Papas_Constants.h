#pragma once
typedef int PapasError;

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <3ds.h>

#define ASSERT(condition, message)                                           \
    do {                                                                     \
        if (!(condition)) {                                                  \
            printf("Assertion failed: %s\nFile: %s\nLine: %d\n",             \
                   message, __FILE__, __LINE__);                             \
            svcOutputDebugString(message, strlen(message));                         \
            printf("Exiting in 3 seconds...\n");                           \
            svcSleepThread(3 * 1000 * 1000 * 1000ULL); /* Sleep for 10 seconds */ \
            exit(1);                                                        \
        }                                                                    \
    } while (0)

#define PAPAS_OK 0
#define PAPAS_NOT_OK 1

#define SCREEN_WIDTH_TOP  400
#define SCREEN_HEIGHT_TOP 240

#define SCREEN_WIDTH_BOTTOM  320
#define SCREEN_HEIGHT_BOTTOM 240