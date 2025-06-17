#include "tasks/SpoofingCheckTask.h"
#include "tasks/GPSTask.h"
#include "sensors/GPS/GPS.h"
#include "sensors/GPS/Defines.h"

void SpoofingCheckTask::run(void* pvParameters) {
    GPS& gps = GPS::getInstance();

    while (1) {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
            bool previousState = gps.getSpoofingState().detected;
            gps.checkSpoofing();
            
            if (gps.getSpoofingState().detected != previousState) {
                if (gps.getSpoofingState().detected) {
                    Serial.println("Обнаружен спуфинг!");
                } else {
                    Serial.println("Спуфинг больше не обнаружен, возобновляем работу GPS");
                }
            }
            
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(TIME_INTERVAL));
    }
}