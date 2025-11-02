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
    // Сервоприводы управления полетом
    ServoGroup elevatorServo;     // Руль высоты
    ServoGroup rudderServo;       // Руль направления
    ServoGroup leftAileronServo;  // Левый элерон
    ServoGroup rightAileronServo; // Правый элерон
    
    bool isTesting = false;
    
    // Настройки для каждого сервопривода
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
    
    // Вспомогательные методы для управления элеронами
    void updateAilerons(int rollValue);
};