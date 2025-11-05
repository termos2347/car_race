#include "ServoManager.h"
#include <Arduino.h>

// Инициализация всех сервоприводов с новыми пинами
ServoManager::ServoManager()
    : elevatorServo(HardwareConfig::ELEVATOR_PIN, ELEVATOR_MIN, ELEVATOR_MAX, ELEVATOR_NEUTRAL, "ELEVATOR"),
      rudderServo(HardwareConfig::RUDDER_PIN, RUDDER_MIN, RUDDER_MAX, RUDDER_NEUTRAL, "RUDDER"),
      leftAileronServo(HardwareConfig::LEFT_AILERON_PIN, AILERON_MIN, AILERON_MAX, AILERON_NEUTRAL, "LEFT_AILERON"),
      rightAileronServo(HardwareConfig::RIGHT_AILERON_PIN, AILERON_MIN, AILERON_MAX, AILERON_NEUTRAL, "RIGHT_AILERON"),
      flapsServo(HardwareConfig::FLAPS_PIN, FLAPS_MIN, FLAPS_MAX, FLAPS_NEUTRAL, "FLAPS"),
      aux1Servo(HardwareConfig::AUX1_PIN, AUX1_MIN, AUX1_MAX, AUX1_NEUTRAL, "AUX1"),
      aux2Servo(HardwareConfig::AUX2_PIN, AUX2_MIN, AUX2_MAX, AUX2_NEUTRAL, "AUX2"),
      aux3Servo(HardwareConfig::AUX3_PIN, AUX3_MIN, AUX3_MAX, AUX3_NEUTRAL, "AUX3"),
      motorServo(HardwareConfig::MOTOR_PIN, MOTOR_MIN, MOTOR_MAX, MOTOR_NEUTRAL, "MOTOR")
{
}

void ServoManager::begin() {
    Serial.println("🚀 START ServoManager");
    Serial.println("📌 Pin configuration:");
    Serial.println("   - ELEVATOR: 13");
    Serial.println("   - RUDDER: 14");
    Serial.println("   - LEFT_AILERON: 27");
    Serial.println("   - RIGHT_AILERON: 26");
    Serial.println("   - FLAPS: 33");
    Serial.println("   - AUX1: 32");
    Serial.println("   - AUX2: 16");
    Serial.println("   - AUX3: 25");
    Serial.println("   - MOTOR: 17");
    
    delay(1000);

    // Инициализация всех сервоприводов
    elevatorServo.begin();
    rudderServo.begin();
    leftAileronServo.begin();
    rightAileronServo.begin();
    flapsServo.begin();
    aux1Servo.begin();
    aux2Servo.begin();
    aux3Servo.begin();
    motorServo.begin();
    
    // Запускаем тестовую последовательность
    testSequence();
    
    Serial.println("✅ ALL Servos INIT OK");
}

void ServoManager::testSequence() {
    Serial.println("🧪 START Complete Servo Test Sequence");
    isTesting = true;
    
    // === ТЕСТ 1: Все в нейтральное положение ===
    Serial.println("🎯 TEST 1: ALL SERVOS TO NEUTRAL");
    elevatorServo.testToNeutral();
    rudderServo.testToNeutral();
    leftAileronServo.testToNeutral();
    rightAileronServo.testToNeutral();
    flapsServo.testToNeutral();
    aux1Servo.testToNeutral();
    aux2Servo.testToNeutral();
    aux3Servo.testToNeutral();
    motorServo.testToNeutral();
    delay(2000);
    
    // === ТЕСТ 2: Все в минимальное положение ===
    Serial.println("🎯 TEST 2: ALL SERVOS TO MINIMUM");
    elevatorServo.testToMin();
    rudderServo.testToMin();
    leftAileronServo.testToMin();
    rightAileronServo.testToMin();
    flapsServo.testToMin();
    aux1Servo.testToMin();
    aux2Servo.testToMin();
    aux3Servo.testToMin();
    motorServo.testToMin();
    delay(2000);
    
    // === ТЕСТ 3: Все в максимальное положение ===
    Serial.println("🎯 TEST 3: ALL SERVOS TO MAXIMUM");
    elevatorServo.testToMax();
    rudderServo.testToMax();
    leftAileronServo.testToMax();
    rightAileronServo.testToMax();
    flapsServo.testToMax();
    aux1Servo.testToMax();
    aux2Servo.testToMax();
    aux3Servo.testToMax();
    motorServo.testToMax();
    delay(2000);
    
    // === ТЕСТ 4: Возврат в нейтральное ===
    Serial.println("🎯 TEST 4: ALL SERVOS BACK TO NEUTRAL");
    elevatorServo.testToNeutral();
    rudderServo.testToNeutral();
    leftAileronServo.testToNeutral();
    rightAileronServo.testToNeutral();
    flapsServo.testToNeutral();
    aux1Servo.testToNeutral();
    aux2Servo.testToNeutral();
    aux3Servo.testToNeutral();
    motorServo.testToNeutral();
    delay(2000);
    
    Serial.println("✅ ALL Servo Tests COMPLETE");
    isTesting = false;
}

void ServoManager::updateAilerons(int rollValue) {
    // Элероны работают в противофазе
    int leftAileronAngle = map(rollValue, -512, 512, AILERON_MAX, AILERON_MIN);
    int rightAileronAngle = map(rollValue, -512, 512, AILERON_MIN, AILERON_MAX);
    
    leftAileronServo.write(leftAileronAngle);
    rightAileronServo.write(rightAileronAngle);
}

void ServoManager::updateFlaps(int flapsValue) {
    if (flapsValue < -300) {
        flapsServo.write(FLAPS_MIN);      // Закрылки убраны
    } else if (flapsValue > 300) {
        flapsServo.write(FLAPS_MAX);      // Закрылки выпущены
    } else {
        flapsServo.write(FLAPS_NEUTRAL);  // Промежуточное положение
    }
}

void ServoManager::updateAuxServos(const ControlData& data) {
    // AUX1 - управление по кнопке 1
    if (data.button1) {
        aux1Servo.write(AUX1_MAX);
    } else {
        aux1Servo.write(AUX1_MIN);
    }
    
    // AUX2 - управление по кнопке 2
    if (data.button2) {
        aux2Servo.write(AUX2_MAX);
    } else {
        aux2Servo.write(AUX2_MIN);
    }
    
    // AUX3 - аналоговое управление (ось X второго джойстика)
    int aux3Angle = map(data.xAxis2, -512, 512, AUX3_MIN, AUX3_MAX);
    aux3Servo.write(aux3Angle);
}

void ServoManager::update(const ControlData& data) {
    if (isTesting) {
        Serial.println("⏸️ Servo update paused during testing");
        return;
    }
    
    // Основные органы управления
    int elevatorAngle = map(data.yAxis1, -512, 512, ELEVATOR_MIN, ELEVATOR_MAX);
    elevatorServo.write(elevatorAngle);
    
    int rudderAngle = map(data.xAxis1, -512, 512, RUDDER_MIN, RUDDER_MAX);
    rudderServo.write(rudderAngle);

    // Управление мотором (ось Y второго джойстика)
    int motorSpeed = map(data.yAxis2, -512, 512, MOTOR_MIN, MOTOR_MAX);
    motorServo.write(motorSpeed);
    
    updateAilerons(data.xAxis2);
    updateFlaps(data.yAxis2);
    updateAuxServos(data);
    
    // Выводим в Serial для отладки
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        // Вычисляем текущие углы для отладки
        int leftAileronAngle = map(data.xAxis2, -512, 512, AILERON_MAX, AILERON_MIN);
        int rightAileronAngle = map(data.xAxis2, -512, 512, AILERON_MIN, AILERON_MAX);
        int aux3Angle = map(data.xAxis2, -512, 512, AUX3_MIN, AUX3_MAX);
        
        // Определяем статус закрылков
        const char* flapsStatus = "MID";
        if (data.yAxis2 < -300) {
            flapsStatus = "UP";
        } else if (data.yAxis2 > 300) {
            flapsStatus = "DOWN";
        }
        
        Serial.print("🎮 Elev:");
        Serial.print(elevatorAngle);
        Serial.print("° Rud:");
        Serial.print(rudderAngle);
        Serial.print("° Ail L:");
        Serial.print(leftAileronAngle);
        Serial.print("°/R:");
        Serial.print(rightAileronAngle);
        Serial.print("° Flaps:");
        Serial.print(flapsStatus);
        Serial.print(" AUX3:");
        Serial.print(aux3Angle);
        Serial.print("° B1:");
        Serial.print(data.button1 ? "ON" : "OFF");
        Serial.print(" B2:");
        Serial.print(data.button2 ? "ON" : "OFF");
        Serial.println();
        
        lastPrint = millis();
    }
}