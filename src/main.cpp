#include "tasks/GPSTask.h"
#include "tasks/SpoofingCheckTask.h"
#include "HardwareSerial.h"
#include "sensors/GPS/Defines.h"
#include "sensors/GPS/GPS.h"
#include "WebServer/WebServer.h"
#include "tasks/WebServerTask.h"
SemaphoreHandle_t xMutex = nullptr;
EventGroupHandle_t gpsEventGroup = nullptr;


void setup() {
    Serial.begin(115200, SERIAL_8N1);
    initializeSharedResources(); 
    if (gpsEventGroup == NULL || xMutex == NULL) {
        Serial.println("Ошибка инициализации ресурсов!");
        while(1);
    }

    xTaskCreate(GPSTask::run, "GPS", 4096, NULL, 2, NULL);
    xTaskCreate(SpoofingCheckTask::run, "SpoofCheck", 4096, NULL, 1, NULL);
    xTaskCreate(WebServerTask::run, "HTTPdebug", 8196, NULL, 0, NULL);
}

void loop() {vTaskDelete(NULL); }
