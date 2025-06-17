#ifndef GPS_H
#define GPS_H

#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "sensors/GPS/Defines.h"
#include "SimpleKalmanFilter.h"

class GPS {
public:
    static GPS& getInstance() {
        static GPS instance;
        return instance;
    }
    
    void init();
    void checkSpoofing();
    void checkRecovery();
    void resetSpoofingState();

    void setBaudRate(uint16_t);
    void begin(uint16_t);
    void end();

    void update();
    String generateFiltredNMEA();

    HardwareSerial& getSerialGPS() { return SerialGPS; }
    TinyGPSPlus& getGps() { return gps; }
    SpoofingState& getSpoofingState() { return spoofingState; }

    float getLatitude() const { return filteredLat; }   // Отфильтрованная широта
    float getLongitude() const { return filteredLng; }  // Отфильтрованная долгота
    float getRawLatitude() { return gps.location.lat(); }  // Сырые данные
    float getRawLongitude() { return gps.location.lng(); }

private:
    GPS();
    ~GPS() = default;
    GPS(const GPS&) = delete;
    GPS& operator=(const GPS&) = delete;
    
    uint32_t safeMillisDelta(uint32_t current, uint32_t previous);
    void checkSignalAnomalies();       // Высший приоритет
    void checkMovementAnomalies();     // Высокий приоритет
    void checkDataConsistency();       // Средний приоритет  
    void checkAltitudeAnomalies();     // Низкий приоритет
    void checkTimestamps();            // Самый Низкий приоритет

    byte calculateCRC(const char *sentence);

    String generateFilteredGPGGA();

    const byte ubx5hz[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00, 0xDE, 0x6A};
    const byte ubxSaveCfg[21] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x31, 0xBF};
    
    TinyGPSPlus gps;
    HardwareSerial SerialGPS;
    SpoofingState spoofingState;

    SimpleKalmanFilter kalmanLat;
    SimpleKalmanFilter kalmanLng;

    // Отфильтрованные координаты
    float filteredLat = 0.0f;
    float filteredLng = 0.0f;
    uint16_t currentBaudRate = 9600; 
};

#endif // GPS_H