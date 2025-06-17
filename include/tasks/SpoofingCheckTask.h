#ifndef SPOOFING_CHECK_TASK_H
#define SPOOFING_CHECK_TASK_H
#include "GPSTask.h"

class SpoofingCheckTask {
public:
    static void run(void* pvParameters);
};

#endif // SPOOFING_CHECK_TASK_H