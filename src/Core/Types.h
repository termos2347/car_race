#pragma once
#include <cstdint>

struct ControlData {
    int16_t xAxis1;     // Ось X первого джойстика (-512 до +512) - РУЛЬ НАПРАВЛЕНИЯ
    int16_t yAxis1;     // Ось Y первого джойстика (-512 до +512) - РУЛЬ ВЫСОТЫ
    int16_t xAxis2;     // Ось X второго джойстика (-512 до +512) - ЭЛЕРОНЫ (КРЕН)
    int16_t yAxis2;     // Ось Y второго джойстика (-512 до +512) - НЕ ИСПОЛЬЗУЕТСЯ
    bool button1;       // Кнопка первого джойстика
    bool button2;       // Кнопка второго джойстика
    uint8_t buttons;    // Дополнительные кнопки (битовая маска)
    uint16_t crc;       // Контрольная сумма
};

struct HardwareConfig {
    // Основные пины для самолета
    static const uint8_t ELEVATOR_PIN = 12;  // Сервопривод руля высоты
    static const uint8_t RUDDER_PIN = 14;    // Сервопривод руля направления
    static const uint8_t LEFT_AILERON_PIN = 15;   // Левый элерон
    static const uint8_t RIGHT_AILERON_PIN = 16;  // Правый элерон
    static const uint8_t MOTOR_PIN = 13;     // Двигатель (PWM)
    static const uint8_t LED_PIN = 2;        // Индикация состояния связи
};