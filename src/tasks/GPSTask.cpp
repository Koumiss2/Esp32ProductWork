#include "tasks/GPSTask.h"
#include "sensors/GPS/GPS.h"
#include "defines/Defines.h"
#include "WebServer/WebServer.h"

void GPSTask::run(void* pvParameters) {
    GPS& gps = GPS::getInstance();
    gps.init();
    DebugTool& server = DebugTool::getInstance();
    static String nmeaBuffer;
    server.sendNMEAData("WebServer is begin");
    while (1) {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {  // Wait indefinitely for mutex
            bool isSpoofing = gps.getSpoofingState().detected;
            
            while (gps.getSerialGPS().available()) {
                char c = gps.getSerialGPS().read();
                gps.getGps().encode(c);
            
                if (!isSpoofing) {
                    Serial.write(c);  
                    nmeaBuffer += c;
                    
                    if (c == '\n') {
                        server.sendNMEAData(nmeaBuffer); 
                        nmeaBuffer = "";
                    }
                } else {
                    static uint32_t lastWarn = 0;
                    if(millis() - lastWarn > 1000) {
                        Serial.println("Спуфинг! Игнорируем GPS.");
                        server.sendNMEAData("Спуфинг! Игнорируем GPS.");
                        lastWarn = millis();
                    }
                }
            }
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(10));  // Smaller delay for more responsive handling
    }
}