#ifndef DEFINES_H
#define DEFINES_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#define RXD2 16
#define TXD2 17                     // UART 2
#define MAX_SPEED_M_S 138.0         // Максимальная скорость
#define MAX_ACCEL_M_S2 98.0         // максимальная акселерация
#define MIN_SATELLITES 4            // Минимальное количество спутников 
#define TIME_INTERVAL 200           // Интервал между обновлением инфы о триггерах
#define REQUIRED_TRIGGERS 2         // Количество сработанных триггеров для выявления спуфинга
#define MIN_SPOOFING_DURATION 2000  // Минимальное время для принятия решения о спуфинге
#define RISK_INCREMENT 5            // Накопление риска
#define RISK_DECREMENT 10           // Убавление риска
#define RISK_THRESHOLD 55           // Уровень недоверия для старта триггера
#define UNRISK_THRESHOLD 40         // Уровень недоверия для восстановления
#define MAX_ALTITUDE_RATE 100.0     // Максимальная скорость изменения высоты (м/с)
#define GPS_TIME_TOLERANCE 1000     // Допустимое расхождение временных меток (мс)
#define MIN_MOVEMENT_DISTANCE 5.0   // Минимальное расстояние для проверки движения (м)
#define RECOVERY_TIME 200           // Время восстановления (мс)

// Определяем биты для событий
#define SPOOFING_DETECTED_BIT  (1 << 0)
#define SPOOFING_RECOVERED_BIT (1 << 1)

extern SemaphoreHandle_t xMutex;
extern EventGroupHandle_t gpsEventGroup;

void initializeSharedResources();

enum SpoofingCheck {
    SIGNAL_ANOMALY,
    MOVEMENT_ANOMALY,
    ALTITUDE_ANOMALY,
    DATA_INCONSISTENCY,
    TIMESTAMP_ANOMALY,
    NUM_CHECKS
};

struct SpoofingState {
    bool detected = false;
    bool flags[NUM_CHECKS] = {false};
    uint32_t startTime = 0;
    int risk = 0;
    int triggeredChecks = 0;

    float lastLat = 0, lastLon = 0;
    float lastSpeed = 0;
    uint32_t lastCheckTime = 0;
    
    float lastAltitude = 0;
    uint32_t lastAltTime = 0;
    
    float lastConsistencyLat = 0, lastConsistencyLon = 0;
    uint32_t lastConsistencyTime = 0;
    
    uint32_t lastGpsTime = 0;
    uint32_t lastSystemTime = 0;
    bool firstTimeCheck = true;
};

#endif