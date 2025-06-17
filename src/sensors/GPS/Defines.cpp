#include "sensors/GPS/Defines.h"

void initializeSharedResources() {
    if (xMutex == nullptr) {
        xMutex = xSemaphoreCreateMutex();
    }
    if (gpsEventGroup == nullptr) {
        gpsEventGroup = xEventGroupCreate();
    }
}