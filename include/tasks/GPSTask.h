// GPSTask.h
#ifndef GPSTASK_H
#define GPSTASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class GPSTask {
public:
    static void run(void* pvParameters);
};

#endif