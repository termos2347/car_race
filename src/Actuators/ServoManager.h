#pragma once
#include <ESP32Servo.h>
#include "Core/Types.h"
#include "ServoGroup.h"

class ServoManager {
public:
    ServoManager();
    void begin();
    void update(const ControlData& data);
    void testSequence();
    
private:
    // Основные сервоприводы управления полетом
    ServoGroup elevatorServo;     // Руль высоты
    ServoGroup rudderServo;       // Руль направления
    ServoGroup leftAileronServo;  // Левый элерон
    ServoGroup rightAileronServo; // Правый элерон
    
    // Дополнительные сервоприводы
    ServoGroup flapsServo;        // Закрылки
    ServoGroup aux1Servo;         // Дополнительный сервопривод 1
    ServoGroup aux2Servo;         // Дополнительный сервопривод 2
    
    bool isTesting = false;
    
    // Настройки для основных сервоприводов
    static const int ELEVATOR_MIN = 0;
    static const int ELEVATOR_MAX = 180;
    static const int ELEVATOR_NEUTRAL = 90;
    
    static const int RUDDER_MIN = 0;
    static const int RUDDER_MAX = 180;
    static const int RUDDER_NEUTRAL = 90;
    
    // Элероны работают в противофазе
    static const int AILERON_MIN = 0;
    static const int AILERON_MAX = 180;
    static const int AILERON_NEUTRAL = 90;
    
    // Настройки для дополнительных сервоприводов
    static const int FLAPS_MIN = 0;
    static const int FLAPS_MAX = 90;
    static const int FLAPS_NEUTRAL = 0;
    
    static const int AUX1_MIN = 0;
    static const int AUX1_MAX = 180;
    static const int AUX1_NEUTRAL = 90;
    
    static const int AUX2_MIN = 0;
    static const int AUX2_MAX = 180;
    static const int AUX2_NEUTRAL = 90;
    
    // Вспомогательные методы
    void updateAilerons(int rollValue);
    void updateFlaps(int flapsValue);
    void updateAuxServos(const ControlData& data);
};