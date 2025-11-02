#pragma once
#include <cstdint>

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

struct HardwareConfig {
    // Основные пины для самолета
    static const uint8_t ELEVATOR_PIN = 12;  // Сервопривод руля высоты
    static const uint8_t MOTOR_PIN = 13;     // Двигатель (PWM)
    static const uint8_t LED_PIN = 2;        // Индикация
    
    // static const uint8_t RUDDER_PIN = 14;
    // static const uint8_t AILERON_LEFT_PIN = 15;
    // static const uint8_t AILERON_RIGHT_PIN = 16;
};