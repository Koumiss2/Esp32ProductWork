#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "sensors/GPS/Defines.h"
#include "sensors/GPS/GPS.h"

class DebugTool {
public:
    // Удаляем конструкторы копирования и присваивания
    DebugTool(const DebugTool&) = delete;
    DebugTool& operator=(const DebugTool&) = delete;

    // Статический метод для получения экземпляра
    static DebugTool& getInstance(uint16_t port = 80) {
        static DebugTool instance(port);
        return instance;
    }

    AsyncWebSocket ws;

    // Публичные методы
    void sendNMEAData(const String& data);
    void init();

    // Доступ к серверу и WebSocket (при необходимости)
    AsyncWebServer& getServer() { return server; }
    AsyncWebSocket& getWebSocket() { return ws; }

private:
    // Приватный конструктор
    explicit DebugTool(uint16_t port);
    
    // Приватные методы
    void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);
    void handleRoot(AsyncWebServerRequest* request);
    void handleLED(AsyncWebServerRequest* request);
    void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                AwsEventType type, void* arg, uint8_t* data, size_t len);

    // Члены класса
    AsyncWebServer server;
    
    const char* ssid = "DebugTool";
    const char* password = "12345678";
    bool isConnected = false;
    uint32_t currentBaudRate = 9600;
    GPS& gps = GPS::getInstance();
    String html;
    
    // Настройки сети
    IPAddress apIP = {192, 168, 4, 1};
    IPAddress gateway = {192, 168, 4, 1};
    IPAddress subnet = {255, 255, 255, 0};
};