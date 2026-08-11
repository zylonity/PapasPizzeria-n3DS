#pragma once
// Error code every framework/scene call passes back up
typedef int PapasError;

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <3ds.h>

// Print + debug log, then bail out; gives you time to read the console
#define ASSERT(condition, message)                                           \
    do {                                                                     \
        if (!(condition)) {                                                  \
            printf("Assertion failed: %s\nFile: %s\nLine: %d\n",             \
                   message, __FILE__, __LINE__);                             \
            svcOutputDebugString(message, strlen(message));                         \
            printf("Exiting in 3 seconds...\n");                           \
            svcSleepThread(3 * 1000 * 1000 * 1000ULL); /* Pause for 3 seconds. */ \
            exit(1);                                                        \
        }                                                                    \
    } while (0)

#define PAPAS_OK 0
#define PAPAS_NOT_OK 1
#define PAPAS_EXIT_REQUESTED 2

#define SCREEN_WIDTH_TOP  400
#define SCREEN_HEIGHT_TOP 240

#define SCREEN_WIDTH_BOTTOM  320
#define SCREEN_HEIGHT_BOTTOM 240

// Original timings, straight off GameData.as and BakingScreen.as
#define COOK_TIME_MS            180000.0f           // a full 360 degree bake, 3 minutes
#define MS_PER_COOK_DEGREE      (COOK_TIME_MS / 360.0f)
#define PREP_TIME_PER_ORDER_MS  45000.0f            // prepTimePerOrder
#define IDEAL_LINE_WAIT_MS      25000.0f            // Customer.idealLineWait
