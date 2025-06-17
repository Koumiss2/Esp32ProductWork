// GPSTask.h
#ifndef GPSTASK_H
#define GPSTASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks/Task.h"

class GPSTask: public Task {
public:
    static void run(void* pvParameters) override;
};

#endif