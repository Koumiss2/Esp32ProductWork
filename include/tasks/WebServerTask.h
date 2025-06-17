#pragma once
#include "WebServer/WebServer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class WebServerTask{
public:
    static void run(void* pvParameters);
};