#pragma once
#include <cstdint>

struct ControlData {
    int16_t xAxis1;     // Ось X первого джойстика (-512 до +512) - РУЛЬ НАПРАВЛЕНИЯ
    int16_t yAxis1;     // Ось Y первого джойстика (-512 до +512) - РУЛЬ ВЫСОТЫ
    int16_t xAxis2;     // Ось X второго джойстика (-512 до +512) - ЭЛЕРОНЫ (КРЕН)
    int16_t yAxis2;     // Ось Y второго джойстика (-512 до +512) - МОТОР/ЗАКРЫЛКИ
    bool button1;       // Кнопка первого джойстика
    bool button2;       // Кнопка второго джойстика
    uint8_t buttons;    // Дополнительные кнопки (битовая маска)
    uint16_t crc;       // Контрольная сумма
};

struct HardwareConfig {
    // Основные пины для самолета
    static const uint8_t L_ELEVATOR_PIN = 13;
    static const uint8_t R_ELEVATOR_PIN = 32;       // Сервопривод руля высоты
    static const uint8_t L_RUDDER_PIN = 14;         // Сервопривод руля направления
    static const uint8_t R_RUDDER_PIN = 16;
    static const uint8_t L_AILERON_PIN = 27;        // Левый элерон
    static const uint8_t R_AILERON_PIN = 26;        // Правый элерон
    static const uint8_t L_FLAPS_PIN = 33;          // Закрылки (дополнительный сервопривод)
    static const uint8_t R_FLAPS_PIN = 25;          // НОВЫЙ сервопривод на пине 25
    static const uint8_t MOTOR_PIN = 17;            // Двигатель (PWM)
    static const uint8_t LED_PIN = 2;               // Индикация состояния связи
};