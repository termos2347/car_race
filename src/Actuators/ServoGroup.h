#pragma once
#include <ESP32Servo.h>
#include "Core/Types.h"

class ServoGroup {
public:
    ServoGroup(uint8_t pin, int minAngle, int maxAngle, int neutralAngle, const char* name, 
               int minPulse, int maxPulse);
    void begin();
    void write(int angle);
    void writeSmooth(int angle, int movementTime = 200);
    void testSequence();
    void testToNeutral();
    void testToMin();
    void testToMax();
    const char* getName() const { return name; }
    int getCurrentAngle() const { return currentAngle; }
    
private:
    Servo servo;
    uint8_t pin;
    int minAngle;
    int maxAngle;
    int neutralAngle;
    const char* name;
    bool isTesting = false;
    int currentAngle = 0;
    int minPulse;
    int maxPulse;
};