#include "sensors/GPS/GPS.h"

GPS::GPS() : SerialGPS(1), kalmanLat(1.0f, 1.0f, 0.01f),
    kalmanLng(1.0f, 1.0f, 0.01f) {}

void GPS::init() {
    SerialGPS.begin(currentBaudRate, SERIAL_8N1, RXD2, TXD2);
    if (!SerialGPS) {
        Serial.println("Error!");
        return;}
    else { Serial.println("UART1 подключился GPIO17(TX) | GPIO18(RX)"); }
    spoofingState.firstTimeCheck = true;
    spoofingState.detected = false;
    spoofingState.risk = 0;
    spoofingState.triggeredChecks = 0;
}

void GPS::setBaudRate(uint16_t newBaud){
  currentBaudRate = newBaud;
}

void GPS::begin(uint16_t newBaud){
    SerialGPS.begin(currentBaudRate, SERIAL_8N1, RXD2, TXD2);
}

void GPS::end(){
  SerialGPS.end();
}

uint32_t GPS::safeMillisDelta(uint32_t current, uint32_t previous) {
    return (current >= previous) ? (current - previous) : (UINT32_MAX - previous + current);
}

byte GPS::calculateCRC(const char *sentence){
    byte crc = 0;
    for (int i = 1; i < strlen(sentence); i++) {
        if (sentence[i] == '*') break;
        crc ^= sentence[i];
    }
    return crc;
}

String GPS::generateFiltredNMEA(){
    String nmea;
    update();
    nmea = generateFilteredGPGGA();
    return nmea;
}

void GPS::update() {
    while (SerialGPS.available()) {
        char c = SerialGPS.read();
        if (gps.encode(c)) {  
            if (gps.location.isUpdated()) {
                // Обновляем фильтр Калмана
                filteredLat = kalmanLat.updateEstimate(gps.location.lat());
                filteredLng = kalmanLng.updateEstimate(gps.location.lng());
            }
        }
    }
}

String GPS::generateFilteredGPGGA() {
    // Получаем отфильтрованные координаты
    float lat = filteredLat;
    float lng = filteredLng;

    // Форматируем координаты в NMEA-стиль
    char latDir = (lat >= 0) ? 'N' : 'S';
    char lngDir = (lng >= 0) ? 'E' : 'W';
    lat = fabs(lat);
    lng = fabs(lng);

    // Форматируем время (берём из сырых данных GPS)
    uint32_t time = gps.time.value();
    // Добавляем ведущие нули, если нужно (чтобы было HHMMSS.sss)
    String timeStr = String(time);
    while (timeStr.length() < 6) {
        timeStr = "0" + timeStr;
    }

    // Форматируем координаты в DDMM.mmmmmm
    int latDeg = (int)lat;
    float latMin = (lat - latDeg) * 60.0;
    int lngDeg = (int)lng;
    float lngMin = (lng - lngDeg) * 60.0;

    String nmea = "$GPGGA,";
    nmea += timeStr + ",";
    nmea += String(latDeg) + String(latMin, 6).substring(1) + "," + latDir + ",";  // Формат DDMM.mmmmmm
    nmea += String(lngDeg) + String(lngMin, 6).substring(1) + "," + lngDir + ",";
    nmea += "1,";  // Fix quality (1 = GPS fix)
    nmea += String(gps.satellites.value()) + ",";
    nmea += String(gps.hdop.value() / 100.0, 1) + ",";  // HDOP
    nmea += String(gps.altitude.meters(), 1) + ",M,";    // Высота
    nmea += "0.0,M,,";  // Geoid separation 

    // Рассчитываем и добавляем CRC
    byte crc = calculateCRC(nmea.c_str() + 1);  // Пропускаем '$'
    char crcStr[3];
    sprintf(crcStr, "%02X", crc);
    nmea += "*";
    nmea += crcStr;

    // Проверяем, есть ли валидные данные
    // if (!gps.location.isValid()) {
    //   String notValidNMEA = "$GPGGA,,,,,,0,,,,,,,,*";
    //   notValidNMEA += crcStr;
    //     // Возвращаем строку с нулевыми координатами, если нет фиксации
    //     return notValidNMEA;  // Стандартная строка "нет данных"
    // }

    return nmea;
}
void GPS::checkSpoofing() {
    spoofingState.triggeredChecks = 0;
    memset(spoofingState.flags, 0, sizeof(spoofingState.flags));
    
    checkSignalAnomalies();
    checkMovementAnomalies();
    checkDataConsistency();
    checkAltitudeAnomalies();
    checkTimestamps();
    checkHDOP();

    if(spoofingState.flags[SIGNAL_ANOMALY]) spoofingState.triggeredChecks += 2;
    if(spoofingState.flags[MOVEMENT_ANOMALY]) spoofingState.triggeredChecks += 1;
    if(spoofingState.flags[DATA_INCONSISTENCY]) spoofingState.triggeredChecks += 1;
    if(spoofingState.flags[ALTITUDE_ANOMALY]) spoofingState.triggeredChecks += 1;
    if(spoofingState.flags[TIMESTAMP_ANOMALY]) spoofingState.triggeredChecks += 1;
    if(spoofingState.flags[HDOP_ANOMALY]) spoofingState.triggeredChecks += 1;


     if(spoofingState.triggeredChecks >= REQUIRED_TRIGGERS) {
        spoofingState.risk = min(70, spoofingState.risk + RISK_INCREMENT);
        
        if(!spoofingState.detected && spoofingState.risk >= RISK_THRESHOLD) {
            spoofingState.detected = true;
            spoofingState.startTime = millis();
        }
    } else {
        spoofingState.risk = max(0, spoofingState.risk - RISK_DECREMENT);
        bool anyFlagActive = false;
        for(int i = 0; i < 5; i++) {
            if(spoofingState.flags[i]) {
                anyFlagActive = true;
                break;
            }
        }
        // Добавлено: автоматический сброс при низком риске
        if(!anyFlagActive && spoofingState.risk < UNRISK_THRESHOLD) {
            spoofingState.detected = false;
            spoofingState.triggeredChecks = 0;
             spoofingState.risk = 0;
        }
    }
}

// В классе GPS добавить метод:
void GPS::resetSpoofingState() {
    spoofingState.detected = false;
    spoofingState.risk = 0;
    spoofingState.triggeredChecks = 0;
    memset(spoofingState.flags, 0, sizeof(spoofingState.flags));
}

void GPS:: checkSignalAnomalies() {
  spoofingState.flags[SIGNAL_ANOMALY] = 
    (gps.satellites.value() < MIN_SATELLITES);
}

void GPS::checkMovementAnomalies() {
  if(gps.location.isValid() && gps.speed.isValid()) {
    float currentSpeed = gps.speed.mps();
    
    // Проверка максимальной скорости
    if(currentSpeed > MAX_SPEED_M_S) {
      spoofingState.flags[MOVEMENT_ANOMALY] = true;
    }
    
    // Проверка ускорения
    if(spoofingState.lastCheckTime != 0) {
      uint32_t timeDiffMs = safeMillisDelta(millis(), spoofingState.lastCheckTime);
      float timeDiff = timeDiffMs / 1000.0f;
      
      if(timeDiff > 0.1f) {  // Минимальный интервал для проверки ускорения
        float acceleration = abs(currentSpeed - spoofingState.lastSpeed) / timeDiff;
        if(acceleration > MAX_ACCEL_M_S2) {
          spoofingState.flags[MOVEMENT_ANOMALY] = true;
        }
      }
    }
    
    // Проверка "прыгающих" координат
    if(spoofingState.lastCheckTime != 0) {
      float distance = TinyGPSPlus::distanceBetween(
        spoofingState.lastLat, spoofingState.lastLon,
        gps.location.lat(), gps.location.lng());
      
      if(distance > MIN_MOVEMENT_DISTANCE && currentSpeed < 1.0) {
        spoofingState.flags[MOVEMENT_ANOMALY] = true;
      }
    }
    
    // Обновление данных
    spoofingState.lastSpeed = currentSpeed;
    spoofingState.lastLat = gps.location.lat();
    spoofingState.lastLon = gps.location.lng();
    spoofingState.lastCheckTime = millis();
  } else {
    spoofingState.flags[MOVEMENT_ANOMALY] = false;
  }
}

void GPS::checkAltitudeAnomalies() {
  if(gps.altitude.isValid()) {
    if(spoofingState.lastAltTime != 0) {
      uint32_t timeDiffMs = safeMillisDelta(millis(), spoofingState.lastAltTime);
      float timeDiff = timeDiffMs / 1000.0f;
      
      if(timeDiff > 0.1f) {
        float altDiff = abs(gps.altitude.meters() - spoofingState.lastAltitude);
        float rate = altDiff / timeDiff;
        
        if(rate > MAX_ALTITUDE_RATE) {
          spoofingState.flags[ALTITUDE_ANOMALY] = true;
        }
      }
    }
    
    spoofingState.lastAltitude = gps.altitude.meters();
    spoofingState.lastAltTime = millis();
  } else {
    spoofingState.flags[ALTITUDE_ANOMALY] = false;
  }
}

void GPS::checkDataConsistency() {
  if(gps.location.isValid() && gps.speed.isValid()) {
    if(spoofingState.lastConsistencyTime != 0) {
      uint32_t timeDiffMs = safeMillisDelta(millis(), spoofingState.lastConsistencyTime);
      float timeDiff = timeDiffMs / 1000.0f;
      
      if(timeDiff > 0.5f) {  // Минимальный интервал для проверки
        float distance = TinyGPSPlus::distanceBetween(
          spoofingState.lastConsistencyLat, spoofingState.lastConsistencyLon,
          gps.location.lat(), gps.location.lng());
        float calculatedSpeed = distance / timeDiff;
        float reportedSpeed = gps.speed.mps();
        
        if(reportedSpeed > 1.0 && abs(calculatedSpeed - reportedSpeed) > reportedSpeed * 0.2) {
          spoofingState.flags[DATA_INCONSISTENCY] = true;
        }
      }
    }
    
    spoofingState.lastConsistencyLat = gps.location.lat();
    spoofingState.lastConsistencyLon = gps.location.lng();
    spoofingState.lastConsistencyTime = millis();
  } else {
    spoofingState.flags[DATA_INCONSISTENCY] = false;
  }
}

void GPS::checkTimestamps() {
  if(gps.time.isValid() && gps.date.isValid()) {
    uint32_t currentGpsTime = gps.date.value() * 86400UL + gps.time.value(); // Конвертация в секунды
    
    if(!spoofingState.firstTimeCheck) {
      int32_t timeDiff = (int32_t)(currentGpsTime - spoofingState.lastGpsTime);
      int32_t systemDiff = (int32_t)(safeMillisDelta(millis(), spoofingState.lastSystemTime) / 1000);
      int32_t delta = abs(timeDiff - systemDiff);
      
      if(delta > (GPS_TIME_TOLERANCE / 1000)) {
        spoofingState.flags[TIMESTAMP_ANOMALY] = true;
      }
    } else {
      spoofingState.firstTimeCheck = false;
    }
    
    spoofingState.lastGpsTime = currentGpsTime;
    spoofingState.lastSystemTime = millis();
  } else {
    spoofingState.flags[TIMESTAMP_ANOMALY] = false;
  }
}

void GPS::checkHDOP(){
  if (gps.location.isValid() && gps.hdop.isValid()){
    float currentHDOP = gps.hdop.hdop();

    uint32_t timeDiffms = safeMillisDelta(millis(), spoofingState.lastHDOPTime);
    float timeDiff = timeDiffms / 1000.0f;

     if (spoofingState.lastHDOP < 0) {
            spoofingState.lastHDOP = currentHDOP;
            spoofingState.lastHDOPTime = millis();
            spoofingState.flags[HDOP_ANOMALY] = false;
            return;
        }

    if (spoofingState.lastHDOP >= 5.0 && timeDiff> 0.1){
      float hdopRate = abs(currentHDOP - spoofingState.lastHDOP) / timeDiff;
      if (hdopRate >= 10){
          spoofingState.flags[HDOP_ANOMALY] = true;
      }
    }
    spoofingState.lastHDOP = currentHDOP;
    spoofingState.lastHDOPTime = millis();
  }
  else {
    spoofingState.flags[HDOP_ANOMALY] = false;
  }
}