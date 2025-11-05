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
      aux2Servo(HardwareConfig::AUX2_PIN, AUX2_MIN, AUX2_MAX, AUX2_NEUTRAL, "AUX2") {
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
    
    // Инициализация всех сервоприводов
    elevatorServo.begin();
    rudderServo.begin();
    leftAileronServo.begin();
    rightAileronServo.begin();
    flapsServo.begin();
    aux1Servo.begin();
    aux2Servo.begin();
    
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
    flapsServo.testSequence();
    aux1Servo.testSequence();
    aux2Servo.testSequence();
    
    // Тест работы элеронов в противофазе
    Serial.println("🔄 Testing aileron synchronization");
    
    // Левый крен
    Serial.println("🔄 LEFT ROLL - Left up, Right down");
    leftAileronServo.write(AILERON_MAX);
    rightAileronServo.write(AILERON_MIN);
    delay(1000);
    
    // Правый крен
    Serial.println("🔄 RIGHT ROLL - Left down, Right up");
    leftAileronServo.write(AILERON_MIN);
    rightAileronServo.write(AILERON_MAX);
    delay(1000);
    
    // Нейтральное положение всех сервоприводов
    Serial.println("🔄 NEUTRAL - All servos centered");
    leftAileronServo.write(AILERON_NEUTRAL);
    rightAileronServo.write(AILERON_NEUTRAL);
    elevatorServo.write(ELEVATOR_NEUTRAL);
    rudderServo.write(RUDDER_NEUTRAL);
    flapsServo.write(FLAPS_NEUTRAL);
    aux1Servo.write(AUX1_NEUTRAL);
    aux2Servo.write(AUX2_NEUTRAL);
    delay(1000);
    
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
    // Управление закрылками (можно привязать к кнопке или отдельной оси)
    // Пока просто демонстрация - можно настроить под свои нужды
    if (flapsValue < -300) {
        flapsServo.write(FLAPS_MIN);      // Закрылки убраны
    } else if (flapsValue > 300) {
        flapsServo.write(FLAPS_MAX);      // Закрылки выпущены
    } else {
        flapsServo.write(FLAPS_NEUTRAL);  // Промежуточное положение
    }
}

void ServoManager::updateAuxServos(const ControlData& data) {
    // Дополнительные сервоприводы - можно настроить под конкретные задачи
    // Например: управление шасси, триммером, камерой и т.д.
    
    // AUX1 - пример: управление по кнопке 1
    if (data.button1) {
        aux1Servo.write(AUX1_MAX);
    } else {
        aux1Servo.write(AUX1_MIN);
    }
    
    // AUX2 - пример: управление по кнопке 2
    if (data.button2) {
        aux2Servo.write(AUX2_MAX);
    } else {
        aux2Servo.write(AUX2_MIN);
    }
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
    
    updateAilerons(data.xAxis2);
    updateFlaps(data.yAxis2);
    updateAuxServos(data);
    
    // Выводим в Serial для отладки (редко, чтобы не спамить)
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        // Вычисляем текущие углы для отладки
        int leftAileronAngle = map(data.xAxis2, -512, 512, AILERON_MAX, AILERON_MIN);
        int rightAileronAngle = map(data.xAxis2, -512, 512, AILERON_MIN, AILERON_MAX);
        
        // Определяем статус закрылков на основе значения оси Y второго джойстика
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
        Serial.print(" B1:");
        Serial.print(data.button1 ? "ON" : "OFF");
        Serial.print(" B2:");
        Serial.print(data.button2 ? "ON" : "OFF");
        Serial.println();
        
        lastPrint = millis();
    }
}