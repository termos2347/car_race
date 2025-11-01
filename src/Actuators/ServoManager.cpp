#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::begin() {
    Serial.println("🔧 Инициализация ServoManager...");
    
    // Инициализация таймеров для ESP32Servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // Инициализация сервопривода
    elevatorServo.setPeriodHertz(50);
    
    if (elevatorServo.attach(ELEVATOR_PIN, SG90_MIN_PULSE, SG90_MAX_PULSE)) {
        servoAttached = true;
        Serial.println("✅ Сервопривод SG90 инициализирован");
        
        // Быстрый тест
        quickTest();
        currentAngle = 90;
        
    } else {
        Serial.println("❌ ОШИБКА: Сервопривод не инициализирован!");
        servoAttached = false;
        return;
    }
    
    // Инициализация мотора
    ledcSetup(MOTOR_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttachPin(MOTOR_PIN, MOTOR_CHANNEL);
    ledcWrite(MOTOR_CHANNEL, 0);
    Serial.println("✅ Мотор инициализирован");
    
    Serial.println("🎯 ServoManager готов");
}

void ServoManager::safeServoWrite(int angle) {
    if (!servoAttached) return;
    
    // Гарантируем границы
    angle = constrain(angle, 0, 180);
    
    elevatorServo.write(angle);
    currentAngle = angle;
}

void ServoManager::update(const ControlData& data) {
    // ПРОСТАЯ И НАДЕЖНАЯ ЛОГИКА:
    // Учитываем реальное нейтральное положение джойстика (-33)
    
    int targetAngle = 90; // Всегда по умолчанию 90°
    
    // Вычисляем отклонение от реального нейтрального положения
    int yOffset = data.yAxis1 - JOYSTICK_NEUTRAL;
    
    // Применяем мертвую зону к ОТКЛОНЕНИЮ, а не к абсолютному значению
    if (abs(yOffset) > JOYSTICK_DEADZONE) {
        if (yOffset > 0) {
            // Джойстик ВВЕРХ от нейтрали
            targetAngle = map(yOffset, JOYSTICK_DEADZONE, 512 - JOYSTICK_NEUTRAL, 90, 180);
        } else {
            // Джойстик ВНИЗ от нейтрали  
            targetAngle = map(yOffset, -JOYSTICK_DEADZONE, -512 - JOYSTICK_NEUTRAL, 90, 0);
        }
    }
    
    // НЕМЕДЛЕННО устанавливаем угол (без сглаживания!)
    safeServoWrite(targetAngle);
    
    // Управление мотором
    int motorPWM = 0;
    int xOffset = data.xAxis1 - JOYSTICK_NEUTRAL; // Аналогично для оси X
    
    if (abs(xOffset) > JOYSTICK_DEADZONE && xOffset > 0) {
        motorPWM = map(xOffset, JOYSTICK_DEADZONE, 512 - JOYSTICK_NEUTRAL, 0, 255);
    }
    motorPWM = constrain(motorPWM, 0, 255);
    ledcWrite(MOTOR_CHANNEL, motorPWM);
}

void ServoManager::debugJoystick(const ControlData& data) {
    // Вычисляем отклонение от нейтрали
    int yOffset = data.yAxis1 - JOYSTICK_NEUTRAL;
    int xOffset = data.xAxis1 - JOYSTICK_NEUTRAL;
    
    const char* yDirection = "НЕЙТРАЛЬ";
    if (yOffset > JOYSTICK_DEADZONE) yDirection = "ВВЕРХ  ";
    else if (yOffset < -JOYSTICK_DEADZONE) yDirection = "ВНИЗ   ";
    
    const char* xDirection = "НЕЙТРАЛЬ";
    if (xOffset > JOYSTICK_DEADZONE) xDirection = "ВПРАВО";
    else if (xOffset < -JOYSTICK_DEADZONE) xDirection = "ВЛЕВО ";
    
    Serial.printf("🎮 Y1=%-4d(%+4d) %s | X1=%-4d(%+4d) %s | Servo=%d°\n", 
                 data.yAxis1, yOffset, yDirection,
                 data.xAxis1, xOffset, xDirection,
                 currentAngle);
}

void ServoManager::quickCalibrate() {
    Serial.println("🎯 Калибровка: серво=90°, мотор=0");
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
    delay(100);
}

void ServoManager::quickTest() {
    Serial.println("🧪 БЫСТРЫЙ ТЕСТ СЕРВОПРИВОДА...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен");
        return;
    }
    
    int testAngles[] = {0, 90, 180, 90};
    for (int i = 0; i < 4; i++) {
        safeServoWrite(testAngles[i]);
        Serial.printf("➡️  %d°\n", testAngles[i]);
        delay(300);
    }
    Serial.println("✅ Тест завершен");
}

void ServoManager::emergencyStop() {
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
}

void ServoManager::testSequence() {
    quickTest();
}

void ServoManager::setServoAngle(int angle) {
    if (!servoAttached) return;
    angle = constrain(angle, 0, 180);
    safeServoWrite(angle);
}

void ServoManager::setMotorSpeed(int speed) {
    speed = constrain(speed, 0, 255);
    ledcWrite(MOTOR_CHANNEL, speed);
}

bool ServoManager::isServoAttached() {
    return servoAttached;
}

void ServoManager::diagnostic() {
    Serial.println("📊 ДИАГНОСТИКА SERVOMANAGER:");
    Serial.printf("   Сервопривод: %s\n", servoAttached ? "ПОДКЛЮЧЕН" : "ОТКЛЮЧЕН");
    Serial.printf("   Текущий угол: %d°\n", currentAngle);
    Serial.printf("   Нейтральное положение джойстика: %d\n", JOYSTICK_NEUTRAL);
    Serial.printf("   Мертвая зона: %d\n", JOYSTICK_DEADZONE);
}