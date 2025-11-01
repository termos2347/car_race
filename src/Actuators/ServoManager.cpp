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
    static unsigned long lastDebug = 0;
    
    // ДИАГНОСТИКА: Всегда выводим сырые данные джойстика
    if (millis() - lastDebug > 150) {
        debugJoystick(data);
        lastDebug = millis();
    }
    
    // ПРОСТАЯ И ПОНЯТНАЯ ЛОГИКА:
    // Джойстик в нейтральном положении = 90°
    // Джойстик вверх = увеличиваем угол (до 180°)
    // Джойстик вниз = уменьшаем угол (до 0°)
    
    int targetAngle = 90; // По умолчанию нейтральное положение
    
    // Проверяем, находится ли джойстик вне мертвой зоны
    if (data.yAxis1 > JOYSTICK_DEADZONE) {
        // Джойстик ОТ себя (вверх) - увеличиваем угол
        targetAngle = map(data.yAxis1, JOYSTICK_DEADZONE, 512, 90, 180);
    } 
    else if (data.yAxis1 < -JOYSTICK_DEADZONE) {
        // Джойстик НА себя (вниз) - уменьшаем угол
        targetAngle = map(data.yAxis1, -JOYSTICK_DEADZONE, -512, 90, 0);
    }
    // Если в мертвой зоне - остаемся на 90°
    
    // Устанавливаем угол сразу
    safeServoWrite(targetAngle);
    
    // Управление мотором (упрощенное)
    int motorPWM = 0;
    if (data.xAxis1 > JOYSTICK_DEADZONE) {
        motorPWM = map(data.xAxis1, JOYSTICK_DEADZONE, 512, 0, 255);
    }
    motorPWM = constrain(motorPWM, 0, 255);
    ledcWrite(MOTOR_CHANNEL, motorPWM);
}

void ServoManager::debugJoystick(const ControlData& data) {
    // Определяем направление для понятного вывода
    const char* direction = "НЕЙТРАЛЬ";
    if (data.yAxis1 > JOYSTICK_DEADZONE) direction = "ВВЕРХ";
    else if (data.yAxis1 < -JOYSTICK_DEADZONE) direction = "ВНИЗ";
    
    int targetAngle = 90;
    if (data.yAxis1 > JOYSTICK_DEADZONE) {
        targetAngle = map(data.yAxis1, JOYSTICK_DEADZONE, 512, 90, 180);
    } else if (data.yAxis1 < -JOYSTICK_DEADZONE) {
        targetAngle = map(data.yAxis1, -JOYSTICK_DEADZONE, -512, 90, 0);
    }
    
    Serial.printf("🎮 Y1=%-4d %-10s -> Угол=%d° | Текущий=%d°\n", 
                 data.yAxis1, direction, targetAngle, currentAngle);
}

void ServoManager::quickCalibrate() {
    Serial.println("🎯 Калибровка: серво=90°, мотор=0");
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
    delay(500);
}

void ServoManager::quickTest() {
    Serial.println("🧪 БЫСТРЫЙ ТЕСТ СЕРВОПРИВОДА...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен");
        return;
    }
    
    Serial.println("➡️  0°");
    safeServoWrite(0);
    delay(500);
    
    Serial.println("➡️  90°");
    safeServoWrite(90);
    delay(500);
    
    Serial.println("➡️  180°");
    safeServoWrite(180);
    delay(500);
    
    Serial.println("➡️  90°");
    safeServoWrite(90);
    delay(500);
    
    Serial.println("✅ Тест завершен");
}

void ServoManager::emergencyStop() {
    Serial.println("🛑 АВАРИЙНАЯ ОСТАНОВКА!");
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
}

void ServoManager::testSequence() {
    Serial.println("🎯 ПОЛНЫЙ ТЕСТ...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен");
        return;
    }
    
    // Тест сервопривода
    int testAngles[] = {0, 45, 90, 135, 180, 90};
    for (int i = 0; i < 6; i++) {
        safeServoWrite(testAngles[i]);
        Serial.printf("📍 Угол: %d°\n", testAngles[i]);
        delay(400);
    }
    
    Serial.println("✅ Тест завершен");
}

void ServoManager::setServoAngle(int angle) {
    if (!servoAttached) return;
    angle = constrain(angle, 0, 180);
    safeServoWrite(angle);
    Serial.printf("🎯 Установлен угол: %d°\n", angle);
}

void ServoManager::setMotorSpeed(int speed) {
    speed = constrain(speed, 0, 255);
    ledcWrite(MOTOR_CHANNEL, speed);
    Serial.printf("🚀 Установлена скорость: %d/255\n", speed);
}

bool ServoManager::isServoAttached() {
    return servoAttached;
}

void ServoManager::diagnostic() {
    Serial.println("📊 ДИАГНОСТИКА SERVOMANAGER:");
    Serial.printf("   Сервопривод: %s\n", servoAttached ? "ПОДКЛЮЧЕН" : "ОТКЛЮЧЕН");
    Serial.printf("   Текущий угол: %d°\n", currentAngle);
    Serial.printf("   Deadzone: %d\n", JOYSTICK_DEADZONE);
}