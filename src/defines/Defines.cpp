#include "defines/Defines.h"

void initializeSharedResources() {
    if (xMutex == nullptr) {
        xMutex = xSemaphoreCreateMutex();
    }
}