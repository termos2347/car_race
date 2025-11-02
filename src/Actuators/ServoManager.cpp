#include "ServoManager.h"
#include <Arduino.h>

// Инициализация сервоприводов с их настройками
ServoManager::ServoManager()
    : elevatorServo(HardwareConfig::ELEVATOR_PIN, ELEVATOR_MIN, ELEVATOR_MAX, ELEVATOR_NEUTRAL, "ELEVATOR"),
      rudderServo(HardwareConfig::RUDDER_PIN, RUDDER_MIN, RUDDER_MAX, RUDDER_NEUTRAL, "RUDDER"),
      leftAileronServo(HardwareConfig::LEFT_AILERON_PIN, AILERON_MIN, AILERON_MAX, AILERON_NEUTRAL, "LEFT_AILERON"),
      rightAileronServo(HardwareConfig::RIGHT_AILERON_PIN, AILERON_MIN, AILERON_MAX, AILERON_NEUTRAL, "RIGHT_AILERON") {
}

void ServoManager::begin() {
    Serial.println("🚀 START ServoManager");
    
    // Инициализация всех сервоприводов
    elevatorServo.begin();
    rudderServo.begin();
    leftAileronServo.begin();
    rightAileronServo.begin();
    
    // Запускаем тестовую последовательность
    testSequence();
    
    Serial.println("✅ ALL Servos INIT OK");
}

void ServoManager::testSequence() {
    Serial.println("🧪 START Complete Servo Test Sequence");
    isTesting = true;
    
    // Тестируем каждый сервопривод по очереди
    elevatorServo.testSequence();
    rudderServo.testSequence();
    leftAileronServo.testSequence();
    rightAileronServo.testSequence();
    
    // Тест работы элеронов в противофазе
    Serial.println("🔄 Testing aileron synchronization");
    
    // Левый крен - левый элерон вверх, правый вниз
    Serial.println("🔄 LEFT ROLL - Left up, Right down");
    leftAileronServo.write(AILERON_MAX);   // Левый вверх
    rightAileronServo.write(AILERON_MIN);  // Правый вниз
    delay(1000);
    
    // Правый крен - левый элерон вниз, правый вверх
    Serial.println("🔄 RIGHT ROLL - Left down, Right up");
    leftAileronServo.write(AILERON_MIN);   // Левый вниз
    rightAileronServo.write(AILERON_MAX);  // Правый вверх
    delay(1000);
    
    // Нейтральное положение
    Serial.println("🔄 NEUTRAL - Both centered");
    leftAileronServo.write(AILERON_NEUTRAL);
    rightAileronServo.write(AILERON_NEUTRAL);
    delay(1000);
    
    Serial.println("✅ ALL Servo Tests COMPLETE");
    isTesting = false;
}

void ServoManager::updateAilerons(int rollValue) {
    // Элероны работают в противофазе:
    // - При крене влево: левый поднимается, правый опускается
    // - При крене вправо: левый опускается, правый поднимается
    
    int leftAileronAngle = map(rollValue, -512, 512, AILERON_MAX, AILERON_MIN);
    int rightAileronAngle = map(rollValue, -512, 512, AILERON_MIN, AILERON_MAX);
    
    leftAileronServo.write(leftAileronAngle);
    rightAileronServo.write(rightAileronAngle);
}

void ServoManager::update(const ControlData& data) {
    if (isTesting) {
        Serial.println("⏸️ Servo update paused during testing");
        return;
    }
    
    // Управление рулем высоты (ось Y первого джойстика)
    int elevatorAngle = map(data.yAxis1, -512, 512, ELEVATOR_MIN, ELEVATOR_MAX);
    elevatorServo.write(elevatorAngle);
    
    // Управление рулем направления (ось X первого джойстика)
    int rudderAngle = map(data.xAxis1, -512, 512, RUDDER_MIN, RUDDER_MAX);
    rudderServo.write(rudderAngle);
    
    // Управление элеронами (ось X второго джойстика)
    updateAilerons(data.xAxis2);
    
    // Выводим в Serial для отладки
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        Serial.print("🎮 Elevator:");
        Serial.print(elevatorAngle);
        Serial.print("° Rudder:");
        Serial.print(rudderAngle);
        Serial.print("° Ailerons L:");
        
        // Вычисляем текущие углы элеронов для отладки
        int leftAileronAngle = map(data.xAxis2, -512, 512, AILERON_MAX, AILERON_MIN);
        int rightAileronAngle = map(data.xAxis2, -512, 512, AILERON_MIN, AILERON_MAX);
        
        Serial.print(leftAileronAngle);
        Serial.print("°/R:");
        Serial.print(rightAileronAngle);
        Serial.println("°");
        lastPrint = millis();
    }
}