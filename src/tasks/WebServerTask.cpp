#include "tasks/WebServerTask.h"
#include "sensors/GPS/Defines.h"

void WebServerTask::run(void* pvParameters) {
    DebugTool& server = DebugTool::getInstance(); 
    server.init();
    server.sendNMEAData("WebServer is begin");
    while(1) {
        server.ws.cleanupClients();
        
        // Optional: Add periodic status updates if needed
        static uint32_t lastUpdate = 0;
        if(millis() - lastUpdate > 5000) {
            server.sendNMEAData("System active, waiting for GPS data...");
            lastUpdate = millis();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}