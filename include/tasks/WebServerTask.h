#pragma once
#include "WebServer/WebServer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks/Task.h"

class WebServerTask: public Task{
public:
    static void run(void* pvParameters);
};