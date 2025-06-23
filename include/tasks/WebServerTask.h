#ifndef WEB_SERVER_TASK_H
#define WEB_SERVER_TASK_H

#include "WebServer/WebServer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class WebServerTask{
public:
    static void run(void* pvParameters);
};

#endif