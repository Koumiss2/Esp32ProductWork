#include "WebServer/WebServer.h"
DebugTool::DebugTool(uint16_t port) 
    : server(port), 
      ws("/ws"),
      currentBaudRate(115200) {
    // Инициализация HTML и других членов
    html = R"rawliteral(<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GNSS Приёмник - NMEA Monitor</title>
    <style>
        :root {
            --primary-color: #3498db;
            --primary-hover: #2980b9;
            --success-color: #27ae60;
            --error-color: #e74c3c;
            --bg-color: #f0f0f0;
            --card-color: #fff;
            --text-color: #333;
            --dark-bg: #2c3e50;
            --light-text: #ecf0f1;
            --disabled-color: #95a5a6;
            --border-radius: 8px;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: var(--bg-color);
            color: var(--text-color);
            line-height: 1.6;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: 0 2px 15px rgba(0,0,0,0.1);
        }
        
        h1 {
            color: var(--dark-bg);
            text-align: center;
            margin-bottom: 25px;
            font-weight: 600;
        }
        
        .control-panel {
            background-color: #f8f9fa;
            padding: 18px;
            border-radius: var(--border-radius);
            margin-bottom: 25px;
            display: flex;
            flex-wrap: wrap;
            gap: 20px;
            align-items: center;
            border: 1px solid #e0e0e0;
        }
        
        .control-group {
            display: flex;
            align-items: center;
            gap: 12px;
        }
        
        label {
            font-weight: 600;
            font-size: 15px;
        }
        
        select, button {
            padding: 10px 15px;
            border-radius: 4px;
            border: 1px solid #ddd;
            font-size: 15px;
            transition: all 0.2s;
        }
        
        select {
            min-width: 120px;
            background-color: white;
        }
        
        select:focus, button:focus {
            outline: none;
            box-shadow: 0 0 0 3px rgba(52,152,219,0.3);
        }
        
        button {
            background-color: var(--primary-color);
            color: white;
            border: none;
            cursor: pointer;
            font-weight: 500;
        }
        
        button:hover {
            background-color: var(--primary-hover);
            transform: translateY(-1px);
        }
        
        button:active {
            transform: translateY(0);
        }
        
        button:disabled {
            background-color: var(--disabled-color);
            cursor: not-allowed;
            transform: none;
        }
        
        .data-display {
            background-color: var(--dark-bg);
            color: var(--light-text);
            padding: 20px;
            border-radius: var(--border-radius);
            font-family: 'Consolas', 'Monaco', monospace;
            height: 500px;
            overflow-y: auto;
            white-space: pre-wrap;
            word-break: break-word;
            line-height: 1.5;
            font-size: 14px;
            border: 1px solid #3d5166;
        }
        
        .status-bar {
            display: flex;
            justify-content: space-between;
            margin: 15px 0;
            padding: 12px 15px;
            background-color: #f8f9fa;
            border-radius: var(--border-radius);
            font-size: 14px;
            border: 1px solid #e0e0e0;
        }
        
        .status-item {
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .status-indicator {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            display: inline-block;
        }
        
        .connected {
            color: var(--success-color);
            font-weight: 600;
        }
        
        .disconnected {
            color: var(--error-color);
            font-weight: 600;
        }
        
        .connected-indicator {
            background-color: var(--success-color);
        }
        
        .disconnected-indicator {
            background-color: var(--error-color);
        }
        
        .timestamp {
            color: #7f8c8d;
            font-size: 13px;
        }
        
        @media (max-width: 768px) {
            .container {
                padding: 15px;
            }
            
            .control-panel {
                flex-direction: column;
                align-items: flex-start;
                gap: 15px;
            }
            
            .data-display {
                height: 400px;
                font-size: 13px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>GNSS Приёмник - NMEA Monitor</h1>
        
        <div class="control-panel">
            <div class="control-group">
                <label for="baud-rate">Скорость:</label>
                <select id="baud-rate">
                    <option value="4800">4800 бод</option>
                    <option value="9600">9600 бод</option>
                    <option value="19200">19200 бод</option>
                    <option value="38400">38400 бод</option>
                    <option value="57600">57600 бод</option>
                    <option value="115200" selected>115200 бод</option>
                </select>
            </div>
            
            <button id="apply-btn" class="btn-primary">Применить</button>
            <button id="clear-btn">Очистить</button>
            <button id="export-btn">Экспорт</button>
        </div>
        
        <div class="status-bar">
            <div class="status-item">
                <span class="status-indicator connected-indicator" id="status-indicator"></span>
                <span>Статус: <span id="status-text" class="connected">Подключение...</span></span>
            </div>
            <div class="status-item">
                <span>Битрейт: <span id="baud-text">115200</span> бод</span>
            </div>
            <div class="status-item">
                <span>Получено: <span id="bytes-received">0</span> байт</span>
            </div>
            <div class="status-item timestamp" id="last-update">
                Последнее обновление: --
            </div>
        </div>
        
        <div class="data-display" id="nmea-output">
            // Ожидание данных GNSS...
        </div>
    </div>

    <script>
        const wsProtocol = window.location.protocol === 'https:' ? 'wss://' : 'ws://';
        const wsUrl = wsProtocol + window.location.hostname + ':' + window.location.port + '/ws';
        
        let ws;
        let reconnectAttempts = 0;
        let totalBytesReceived = 0;
        let lastMessageTime = null;
        let isFirstMessage = true;
        
        // DOM elements
        const nmeaOutput = document.getElementById('nmea-output');
        const statusText = document.getElementById('status-text');
        const statusIndicator = document.getElementById('status-indicator');
        const baudText = document.getElementById('baud-text');
        const bytesReceived = document.getElementById('bytes-received');
        const lastUpdate = document.getElementById('last-update');
        const applyBtn = document.getElementById('apply-btn');
        const baudRateSelect = document.getElementById('baud-rate');
        const clearBtn = document.getElementById('clear-btn');
        const exportBtn = document.getElementById('export-btn');
        
        function updateStatusTime() {
            const now = new Date();
            lastUpdate.textContent = `Последнее обновление: ${now.toLocaleTimeString()}`;
            lastMessageTime = now;
        }
        
        function initWebSocket() {
            statusText.textContent = 'Подключение...';
            statusIndicator.className = 'status-indicator';
            
            // Add cache buster to prevent connection issues
            const cacheBuster = '?_=' + Date.now();
            ws = new WebSocket(wsUrl + cacheBuster);
            
            ws.onopen = function() {
                reconnectAttempts = 0;
                statusText.textContent = 'Активно';
                statusText.className = 'connected';
                statusIndicator.className = 'status-indicator connected-indicator';
                console.log('WebSocket connected');
                
                // Request initial data
                ws.send('INIT');
            };
            
            ws.onclose = function(e) {
                if (e.code === 1000) {
                    statusText.textContent = 'Отключено';
                    console.log('WebSocket closed normally');
                    return;
                }
                
                statusText.textContent = 'Неактивно';
                statusText.className = 'disconnected';
                statusIndicator.className = 'status-indicator disconnected-indicator';
                
                const delay = Math.min(5000, (reconnectAttempts + 1) * 1000);
                reconnectAttempts++;
                console.log(`WebSocket disconnected. Reconnecting in ${delay/1000} seconds...`);
                
                setTimeout(initWebSocket, delay);
            };
            
            ws.onerror = function(error) {
                console.error('WebSocket error:', error);
                statusText.textContent = 'Ошибка подключения';
                statusText.className = 'disconnected';
                statusIndicator.className = 'status-indicator disconnected-indicator';
            };
            
            ws.onmessage = function(event) {
                try {
                    const message = event.data;
                    console.log('Received message:', message); // Debug log
                    
                    const messageSize = new Blob([message]).size;
                    totalBytesReceived += messageSize;
                    bytesReceived.textContent = totalBytesReceived.toLocaleString();
                    
                    if (message.startsWith('BAUDRATE:')) {
                        const baud = message.substring(9);
                        baudText.textContent = baud;
                        baudRateSelect.value = baud;
                    } else {
                        // Clear "waiting for data" message on first real data
                        if (isFirstMessage) {
                            nmeaOutput.textContent = '';
                            isFirstMessage = false;
                        }
                        
                        // Ensure proper line endings
                        const formattedMessage = message.endsWith('\n') ? message : message + '\n';
                        const now = new Date();
                        const timestamp = `[${now.toLocaleTimeString()}] `;
                        
                        // Prevent memory issues with large logs
                        if (nmeaOutput.textContent.length > 100000) {
                            nmeaOutput.textContent = nmeaOutput.textContent.substring(50000);
                        }
                        
                        nmeaOutput.textContent += timestamp + formattedMessage;
                        nmeaOutput.scrollTop = nmeaOutput.scrollHeight;
                    }
                    
                    updateStatusTime();
                } catch (e) {
                    console.error('Error processing message:', e);
                }
            };
        }
        
        function exportData() {
            try {
                const blob = new Blob([nmeaOutput.textContent], { type: 'text/plain' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = `nmea_data_${new Date().toISOString().slice(0,10)}.log`;
                document.body.appendChild(a);
                a.click();
                setTimeout(() => {
                    document.body.removeChild(a);
                    URL.revokeObjectURL(url);
                }, 100);
            } catch (e) {
                console.error('Export failed:', e);
                alert('Ошибка при экспорте данных');
            }
        }
        
        // Event listeners
        applyBtn.addEventListener('click', function() {
            const selectedBaud = baudRateSelect.value;
            if (ws && ws.readyState === WebSocket.OPEN) {
                try {
                    ws.send('BAUD:' + selectedBaud);
                } catch (e) {
                    console.error('Error sending baud rate:', e);
                }
            }
        });
        
        clearBtn.addEventListener('click', function() {
            nmeaOutput.textContent = '// Лог очищен\n';
            totalBytesReceived = 0;
            bytesReceived.textContent = '0';
        });
        
        exportBtn.addEventListener('click', exportData);
        
        // Initialize
        document.addEventListener('DOMContentLoaded', function() {
            initWebSocket();
            updateStatusTime();
            
            // Connection health check
            setInterval(() => {
                if (lastMessageTime && (new Date() - lastMessageTime) > 10000) {
                    statusText.textContent = 'Нет данных';
                    statusIndicator.className = 'status-indicator disconnected-indicator';
                    
                    // Attempt reconnect if no data for 10 seconds
                    if (ws.readyState !== WebSocket.CONNECTING && ws.readyState !== WebSocket.OPEN) {
                        initWebSocket();
                    }
                }
            }, 5000);
        });
    </script>
</body>
</html>
)rawliteral";  
}

void DebugTool::init() {
    Serial.begin(currentBaudRate);
    delay(1000);
    
    // Настройка WiFi
    if(!WiFi.softAPConfig(apIP, gateway, subnet)) {
        Serial.println("Failed to configure AP");
        return;
    }

    if(!WiFi.softAP(ssid, password)) {
        Serial.println("Failed to start AP");
        return;
    }

    Serial.print("AP запущен, SSID: ");
    Serial.println(ssid);
    Serial.print("IP адрес: ");
    Serial.println(WiFi.softAPIP());

    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
        this->onEvent(server, client, type, arg, data, len);
    });
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleRoot(request);
    });

    server.on("/led", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleLED(request);
    });

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    // Инициализация GNSS с дефолтным битрейтом
    gps.begin(currentBaudRate);
    server.begin();
    Serial.println("HTTP сервер запущен");
}

void DebugTool::sendNMEAData(const String& data) {
    if (data.length() > 0) {
        ws.textAll(data);
    }
}

void DebugTool::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
    switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      // Отправляем текущий битрейт при подключении
      client->text("BAUDRATE:" + String(currentBaudRate));
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_ERROR:
      Serial.printf("WebSocket error\n");
      break;
  }
}

void DebugTool::handleWebSocketMessage(void *arg, uint8_t *data, size_t len){
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        String message = (char*)data;
        
        if (message.startsWith("BAUD:")) {
            // Изменение скорости передачи
            int newBaud = message.substring(5).toInt();
            if (newBaud != currentBaudRate) {
                Serial.print("Changing baud rate to: ");
                Serial.println(newBaud);
                
                // Закрываем текущее соединение
                gps.end();
                
                // Устанавливаем новую скорость
                currentBaudRate = newBaud;
                gps.begin(currentBaudRate);
                
                // Уведомляем всех клиентов о новом битрейте
                ws.textAll("BAUDRATE:" + String(currentBaudRate));
            }
        }
    }
}

void DebugTool::handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", html);
}

void DebugTool::handleLED(AsyncWebServerRequest *request) {
    if (request->hasParam("state")) {
        String state = request->getParam("state")->value();
        
        if (state == "on") {
            digitalWrite(LED_BUILTIN, HIGH);
            request->send(200, "text/plain", "LED включен");
            Serial.println("LED включен через веб-интерфейс");
        } 
        else if (state == "off") {
            digitalWrite(LED_BUILTIN, LOW);
            request->send(200, "text/plain", "LED выключен");
            Serial.println("LED выключен через веб-интерфейс");
        } 
        else {
            request->send(400, "text/plain", "Неверный параметр state");
        }
    } 
    else {
        request->send(400, "text/plain", "Отсутствует параметр state");
    }
}