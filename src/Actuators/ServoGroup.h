#pragma once
#include <ESP32Servo.h>
#include "Core/Types.h"

class ServoGroup {
public:
    ServoGroup(uint8_t pin, int minAngle, int maxAngle, int neutralAngle, const char* name);
    void begin();
    void write(int angle);
    void testSequence();
    const char* getName() const { return name; }
    
private:
    Servo servo;
    uint8_t pin;
    int minAngle;
    int maxAngle;
    int neutralAngle;
    const char* name;
    bool isTesting = false;
    
    static const int SERVO_MIN_PULSE = 500;
    static const int SERVO_MAX_PULSE = 2400;
};