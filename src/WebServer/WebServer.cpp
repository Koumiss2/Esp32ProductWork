#include "WebServer/WebServer.h"
DebugTool::DebugTool(uint16_t port) 
    : server(port), 
      ws("/ws"),
      currentBaudRate(9600) {
    // Инициализация HTML и других членов
    html = R"rawliteral(<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GNSS Monitor - Режим отладки</title>
    <style>
        :root {
            --primary: #4361ee;
            --primary-dark: #3a56d4;
            --secondary: #3f37c9;
            --success: #4cc9f0;
            --danger: #f72585;
            --warning: #f8961e;
            --dark: #212529;
            --light: #f8f9fa;
            --gray: #6c757d;
            --border-radius: 0.375rem;
            --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Inter', system-ui, -apple-system, sans-serif;
        }

        body {
            background-color: #f5f7fa;
            color: var(--dark);
            line-height: 1.6;
            padding: 1rem;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            overflow: hidden;
        }

        header {
            background: var(--primary);
            color: white;
            padding: 1.5rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        h1 {
            font-size: 1.5rem;
            font-weight: 600;
        }

        .control-panel {
            display: flex;
            gap: 1rem;
            padding: 1rem;
            background: #f1f5f9;
            border-bottom: 1px solid #e2e8f0;
            flex-wrap: wrap;
        }

        .control-group {
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        label {
            font-weight: 500;
            font-size: 0.875rem;
        }

        select, button {
            padding: 0.5rem 0.75rem;
            border-radius: var(--border-radius);
            border: 1px solid #ced4da;
            font-size: 0.875rem;
            transition: all 0.15s ease;
        }

        select {
            background-color: white;
            min-width: 120px;
        }

        select:focus, button:focus {
            outline: none;
            box-shadow: 0 0 0 3px rgba(67, 97, 238, 0.3);
            border-color: var(--primary);
        }

        button {
            background-color: var(--primary);
            color: white;
            border: none;
            cursor: pointer;
            font-weight: 500;
        }

        button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-1px);
        }

        button:active {
            transform: translateY(0);
        }

        button:disabled {
            background-color: var(--gray);
            cursor: not-allowed;
            opacity: 0.7;
        }

        .btn-danger {
            background-color: var(--danger);
        }

        .btn-success {
            background-color: var(--success);
        }

        .data-container {
            display: flex;
            flex-direction: column;
            height: calc(100vh - 200px);
        }

        .status-bar {
            display: flex;
            justify-content: space-between;
            padding: 0.75rem 1rem;
            background: #f1f5f9;
            border-bottom: 1px solid #e2e8f0;
            font-size: 0.875rem;
        }

        .status-item {
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .status-indicator {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            display: inline-block;
        }

        .connected {
            color: var(--success);
        }

        .disconnected {
            color: var(--danger);
        }

        .data-display {
            flex-grow: 1;
            padding: 1rem;
            background: #1e293b;
            color: #e2e8f0;
            font-family: 'Fira Code', monospace;
            font-size: 0.875rem;
            overflow-y: auto;
            white-space: pre-wrap;
            line-height: 1.5;
        }

        .timestamp {
            color: #94a3b8;
            font-size: 0.75rem;
        }

        @media (max-width: 768px) {
            .control-panel {
                flex-direction: column;
            }
            
            .status-bar {
                flex-wrap: wrap;
                gap: 0.5rem;
            }
        }
    </style>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600&family=Fira+Code&display=swap" rel="stylesheet">
</head>
<body>
    <div class="container">
        <header>
            <h1>GNSS Monitor</h1>
            <div class="status-item">
                <span class="status-indicator" id="status-indicator"></span>
                <span id="status-text">Подключение...</span>
            </div>
        </header>

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
            
            <button id="apply-btn">Применить</button>
            <button id="clear-btn" class="btn-danger">Очистить</button>
            <button id="export-btn" class="btn-success">Экспорт</button>
        </div>

        <div class="status-bar">
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

        <div class="data-container">
            <div class="data-display" id="nmea-output">
// Ожидание данных GNSS...
            </div>
        </div>
    </div>

    <script>
        class GNSSMonitor {
            constructor() {
                this.ws = null;
                this.reconnectAttempts = 0;
                this.totalBytesReceived = 0;
                this.lastMessageTime = null;
                this.isFirstMessage = true;
                this.connectionTimeout = null;

                this.initElements();
                this.initEventListeners();
                this.initWebSocket();
                this.initConnectionMonitor();
            }

            initElements() {
                this.nmeaOutput = document.getElementById('nmea-output');
                this.statusText = document.getElementById('status-text');
                this.statusIndicator = document.getElementById('status-indicator');
                this.baudText = document.getElementById('baud-text');
                this.bytesReceived = document.getElementById('bytes-received');
                this.lastUpdate = document.getElementById('last-update');
                this.applyBtn = document.getElementById('apply-btn');
                this.baudRateSelect = document.getElementById('baud-rate');
                this.clearBtn = document.getElementById('clear-btn');
                this.exportBtn = document.getElementById('export-btn');
            }

            initEventListeners() {
                this.applyBtn.addEventListener('click', () => this.handleBaudRateChange());
                this.clearBtn.addEventListener('click', () => this.clearData());
                this.exportBtn.addEventListener('click', () => this.exportData());
            }

            initWebSocket() {
                const protocol = window.location.protocol === 'https:' ? 'wss://' : 'ws://';
                const url = protocol + window.location.hostname + ':' + window.location.port + '/ws';
                
                this.updateConnectionStatus('Подключение...', false);
                
                if (this.ws) {
                    this.ws.close();
                }

                this.ws = new WebSocket(url + '?_=' + Date.now());

                this.ws.onopen = () => {
                    this.reconnectAttempts = 0;
                    this.updateConnectionStatus('Активно', true);
                    this.ws.send('INIT');
                };

                this.ws.onclose = (e) => {
                    if (e.code === 1000) {
                        this.updateConnectionStatus('Отключено', false);
                        return;
                    }

                    this.updateConnectionStatus('Неактивно', false);
                    const delay = Math.min(5000, (this.reconnectAttempts + 1) * 1000);
                    this.reconnectAttempts++;
                    setTimeout(() => this.initWebSocket(), delay);
                };

                this.ws.onerror = (error) => {
                    console.error('WebSocket error:', error);
                    this.updateConnectionStatus('Ошибка подключения', false);
                };

                this.ws.onmessage = (event) => this.handleMessage(event.data);
            }

            handleMessage(message) {
                try {
                    if (!message) return;

                    const messageSize = new Blob([message]).size;
                    this.totalBytesReceived += messageSize;
                    this.bytesReceived.textContent = this.totalBytesReceived.toLocaleString();

                    if (message.startsWith('BAUDRATE:')) {
                        const baud = message.substring(9);
                        this.baudText.textContent = baud;
                        this.baudRateSelect.value = baud;
                    } else {
                        this.processDataMessage(message);
                    }

                    this.updateStatusTime();
                } catch (e) {
                    console.error('Error processing message:', e);
                }
            }

            processDataMessage(message) {
                if (this.isFirstMessage) {
                    this.nmeaOutput.textContent = '';
                    this.isFirstMessage = false;
                }

                const now = new Date();
                const timestamp = `[${now.toLocaleTimeString()}] `;
                const formattedMessage = message.endsWith('\n') ? message : message + '\n';

                // Prevent memory issues
                if (this.nmeaOutput.textContent.length > 100000) {
                    this.nmeaOutput.textContent = this.nmeaOutput.textContent.substring(50000);
                }

                this.nmeaOutput.textContent += timestamp + formattedMessage;
                this.nmeaOutput.scrollTop = this.nmeaOutput.scrollHeight;
            }

            handleBaudRateChange() {
                const selectedBaud = this.baudRateSelect.value;
                if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                    this.ws.send('BAUD:' + selectedBaud);
                }
            }

            clearData() {
                this.nmeaOutput.textContent = '// Лог очищен\n';
                this.totalBytesReceived = 0;
                this.bytesReceived.textContent = '0';
            }

            exportData() {
                if (!this.nmeaOutput.textContent.trim() || 
                    this.nmeaOutput.textContent === '// Ожидание данных GNSS...') {
                    alert('Нет данных для экспорта');
                    return;
                }

                try {
                    const blob = new Blob([this.nmeaOutput.textContent], { type: 'text/plain' });
                    const url = URL.createObjectURL(blob);
                    const a = document.createElement('a');
                    a.href = url;
                    a.download = `gnss_data_${new Date().toISOString().slice(0, 10)}.log`;
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

            updateConnectionStatus(text, isConnected) {
                this.statusText.textContent = text;
                this.statusText.className = isConnected ? 'connected' : 'disconnected';
                this.statusIndicator.className = 'status-indicator ' + 
                    (isConnected ? 'connected' : 'disconnected');
            }

            updateStatusTime() {
                const now = new Date();
                this.lastUpdate.textContent = `Последнее обновление: ${now.toLocaleTimeString()}`;
                this.lastMessageTime = now;
                
                // Reset connection status if data is flowing again
                if (this.statusText.textContent === 'Нет данных') {
                    this.updateConnectionStatus('Активно', true);
                }
            }

            initConnectionMonitor() {
                setInterval(() => {
                    if (this.lastMessageTime && (new Date() - this.lastMessageTime) > 10000) {
                        this.updateConnectionStatus('Нет данных', false);
                        
                        if (this.ws.readyState !== WebSocket.CONNECTING && 
                            this.ws.readyState !== WebSocket.OPEN) {
                            this.initWebSocket();
                        }
                    }
                }, 5000);
            }
        }

        // Initialize application when DOM is loaded
        document.addEventListener('DOMContentLoaded', () => {
            new GNSSMonitor();
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