#pragma once
#include <cstdint>

// Обновленная структура данных для двух джойстиков (должна совпадать с передатчиком)
struct ControlData {
    int16_t xAxis1;     // Ось X первого джойстика (-512 до +512)
    int16_t yAxis1;     // Ось Y первого джойстика (-512 до +512)
    int16_t xAxis2;     // Ось X второго джойстика (-512 до +512)
    int16_t yAxis2;     // Ось Y второго джойстика (-512 до +512)
    bool button1;       // Кнопка первого джойстика
    bool button2;       // Кнопка второго джойстика
    uint8_t buttons;    // Дополнительные кнопки (битовая маска)
    uint16_t crc;       // Контрольная сумма
};

// Конфигурация пинов для приемника
struct HardwareConfig {
    // Сервоприводы для первого джойстика
    static const uint8_t SERVO_0_PIN = 12;  // Y ось
    static const uint8_t SERVO_1_PIN = 27;  // Y ось  
    static const uint8_t SERVO_2_PIN = 13;  // X ось
    static const uint8_t SERVO_3_PIN = 14;  // X ось
    
    // Сервоприводы для второго джойстика
    static const uint8_t SERVO_4_PIN = 15;  // Y ось второго джойстика
    static const uint8_t SERVO_5_PIN = 16;  // Y ось второго джойстика  
    static const uint8_t SERVO_6_PIN = 17;  // X ось второго джойстика
    static const uint8_t SERVO_7_PIN = 18;  // X ось второго джойстика
    
    static const uint8_t LED_PIN = 2;
};