#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::begin() {
    #if DEBUG_MODE
        Serial.println("🔧 Инициализация ServoManager...");
        Serial.printf("   Сервопривод: пин %d\n", ELEVATOR_PIN);
        Serial.printf("   Двигатель: пин %d\n", MOTOR_PIN);
    #endif
    
    // Проверка пинов
    if (!validatePins()) {
        return;
    }
    
    // 1. СНАЧАЛА инициализация двигателя (чтобы избежать конфликтов)
    ledcSetup(MOTOR_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttachPin(MOTOR_PIN, MOTOR_CHANNEL);
    ledcWrite(MOTOR_CHANNEL, 0);
    
    // 2. ИНИЦИАЛИЗАЦИЯ SERVO С ПРОСТЫМ ПОДХОДОМ
    #if DEBUG_MODE
        Serial.println("🔧 Инициализация сервопривода...");
    #endif
    
    // Простая инициализация без сложных таймеров
    if (elevatorServo.attach(ELEVATOR_PIN, 500, 2500)) {
        #if DEBUG_MODE
            Serial.println("✅ Servo прикреплен с диапазоном 500-2500 мкс");
        #endif
    } else {
        // Пробуем стандартный диапазон
        if (elevatorServo.attach(ELEVATOR_PIN)) {
            #if DEBUG_MODE
                Serial.println("✅ Servo прикреплен со стандартным диапазоном");
            #endif
        } else {
            #if DEBUG_MODE
                Serial.println("❌ КРИТИЧЕСКАЯ ОШИБКА: Servo не прикреплен!");
            #endif
            return;
        }
    }
    
    // 3. НЕМЕДЛЕННЫЙ тест для проверки работы
    #if DEBUG_MODE
        Serial.println("🎯 Немедленный тест сервопривода...");
    #endif
    quickTest();
    
    // 4. Определение реального рабочего диапазона сервопривода
    #if DEBUG_MODE
        Serial.println("🎯 Определение рабочего диапазона сервопривода...");
    #endif
    findServoRange();
    
    #if DEBUG_MODE
        Serial.println("✅ ServoManager инициализирован");
    #endif
}

void ServoManager::findServoRange() {
    #if DEBUG_MODE
        Serial.println("🔍 Поиск рабочего диапазона сервопривода...");
    #endif
    
    // Тестируем широкий диапазон для поиска реальных границ
    int testPositions[] = {500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 
                          1400, 1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200, 2300, 2400, 2500};
    
    #if DEBUG_MODE
        Serial.println("   Тестируем позиции:");
    #endif
    
    for (int i = 0; i < sizeof(testPositions)/sizeof(testPositions[0]); i++) {
        elevatorServo.writeMicroseconds(testPositions[i]);
        #if DEBUG_MODE
            Serial.printf("      📍 %d мкс\n", testPositions[i]);
        #endif
        delay(300); // Даем время на движение
    }
    
    // Возвращаем в нейтраль
    elevatorServo.writeMicroseconds(1500);
    delay(500);
    
    #if DEBUG_MODE
        Serial.println("✅ Поиск диапазона завершен");
    #endif
}

void ServoManager::quickTest() {
    #if DEBUG_MODE
        Serial.println("🔧 Быстрый тест сервопривода...");
    #endif
    
    // НЕМЕДЛЕННО проверяем работу в полном диапазоне
    elevatorServo.writeMicroseconds(500);
    delay(500);
    elevatorServo.writeMicroseconds(1500);
    delay(500);
    elevatorServo.writeMicroseconds(2500);
    delay(500);
    elevatorServo.writeMicroseconds(1500);
    delay(500);
    
    #if DEBUG_MODE
        Serial.println("✅ Быстрый тест завершен");
    #endif
}

void ServoManager::update(const ControlData& data) {
    // Управление сервоприводом
    int pulseWidth = mapToPulse(data.xAxis1);
    elevatorServo.writeMicroseconds(pulseWidth);
    
    // Управление двигателем
    int targetMotorPWM = mapToMotorPWM(data.yAxis1);
    
    // Плавное изменение мощности
    if (targetMotorPWM > currentMotorPWM) {
        currentMotorPWM += MOTOR_RAMP_UP_SPEED;
        if (currentMotorPWM > targetMotorPWM) currentMotorPWM = targetMotorPWM;
    } else if (targetMotorPWM < currentMotorPWM) {
        currentMotorPWM -= MOTOR_RAMP_DOWN_SPEED;
        if (currentMotorPWM < targetMotorPWM) currentMotorPWM = targetMotorPWM;
    }
    
    currentMotorPWM = constrain(currentMotorPWM, 0, 255);
    ledcWrite(MOTOR_CHANNEL, currentMotorPWM);
    
    #if DEBUG_MODE
        static unsigned long lastDebug = 0;
        if (millis() - lastDebug > 500) {
            Serial.printf("🎮 X1:%-4d→серво:%-4dмкс | Y1:%-4d→двиг:PWM%-3d\n", 
                         data.xAxis1, pulseWidth, data.yAxis1, currentMotorPWM);
            lastDebug = millis();
        }
    #endif
}

void ServoManager::testSequence() {
    #if DEBUG_MODE
        Serial.println("🎯 ЗАПУСК ПОЛНОЙ ТЕСТОВОЙ ПОСЛЕДОВАТЕЛЬНОСТИ");
        Serial.println("==========================================");
    #endif
    
    // ТЕСТ 1: НЕМЕДЛЕННАЯ проверка крайних положений
    #if DEBUG_MODE
        Serial.println("🔋 Тест 1: Немедленная проверка крайних положений");
    #endif
    
    // Сразу проверяем полный диапазон
    elevatorServo.writeMicroseconds(500);
    #if DEBUG_MODE
        Serial.println("   📍 MIN: 500 мкс");
    #endif
    delay(2000);
    
    elevatorServo.writeMicroseconds(2500);
    #if DEBUG_MODE
        Serial.println("   📍 MAX: 2500 мкс");
    #endif
    delay(2000);
    
    elevatorServo.writeMicroseconds(1500);
    #if DEBUG_MODE
        Serial.println("   📍 NEUTRAL: 1500 мкс");
    #endif
    delay(1000);
    
    // ТЕСТ 2: Быстрое движение по всему диапазону
    #if DEBUG_MODE
        Serial.println("⚡ Тест 2: Быстрое движение по диапазону");
    #endif
    
    for (int i = 500; i <= 2500; i += 100) {
        elevatorServo.writeMicroseconds(i);
        #if DEBUG_MODE
            Serial.printf("   📍 Положение: %d мкс\n", i);
        #endif
        delay(200);
    }
    
    for (int i = 2500; i >= 500; i -= 100) {
        elevatorServo.writeMicroseconds(i);
        #if DEBUG_MODE
            Serial.printf("   📍 Положение: %d мкс\n", i);
        #endif
        delay(200);
    }
    
    // Возврат в нейтраль
    elevatorServo.writeMicroseconds(1500);
    delay(1000);
    
    // ТЕСТ 3: Резкие движения
    #if DEBUG_MODE
        Serial.println("⚡ Тест 3: Резкие движения");
    #endif
    
    int testPositions[] = {500, 1500, 2500, 1500, 500, 1500};
    const char* positionNames[] = {"MIN", "NEUTRAL", "MAX", "NEUTRAL", "MIN", "NEUTRAL"};
    
    for(int i = 0; i < 6; i++) {
        elevatorServo.writeMicroseconds(testPositions[i]);
        #if DEBUG_MODE
            Serial.printf("   🔄 %s: %d мкс\n", positionNames[i], testPositions[i]);
        #endif
        delay(1000);
    }
    
    // ТЕСТ 4: Двигатель
    #if DEBUG_MODE
        Serial.println("🚀 Тест 4: Двигатель");
    #endif
    
    for(int pwm = 0; pwm <= 255; pwm += 50) {
        ledcWrite(MOTOR_CHANNEL, pwm);
        #if DEBUG_MODE
            Serial.printf("   🔧 PWM двигателя: %d/255\n", pwm);
        #endif
        delay(800);
    }
    ledcWrite(MOTOR_CHANNEL, 0);
    
    // Финальная калибровка
    elevatorServo.writeMicroseconds(1500);
    
    #if DEBUG_MODE
        Serial.println("✅ ТЕСТЫ ЗАВЕРШЕНЫ");
        Serial.println("==========================================");
    #endif
}

// Остальные методы остаются без изменений
void ServoManager::applyMixer(const ControlData& data, int16_t& servoOutput, int16_t& motorOutput) {
    servoOutput = constrain(data.xAxis1, INPUT_MIN, INPUT_MAX);
    motorOutput = constrain(data.yAxis1, INPUT_MIN, INPUT_MAX);
    if (motorOutput < 0) motorOutput = 0;
}

int ServoManager::mapToPulse(int16_t value) {
    return map(value, INPUT_MIN, INPUT_MAX, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
}

int ServoManager::mapToMotorPWM(int16_t value) {
    if (value < MOTOR_DEAD_ZONE) return 0;
    return map(value, MOTOR_DEAD_ZONE, INPUT_MAX, MOTOR_MIN_PWM, 255);
}

void ServoManager::quickCalibrate() {
    elevatorServo.writeMicroseconds(SERVO_NEUTRAL_PULSE);
    ledcWrite(MOTOR_CHANNEL, 0);
    currentMotorPWM = 0;
    delay(100);
}

void ServoManager::emergencyStop() {
    elevatorServo.writeMicroseconds(SERVO_NEUTRAL_PULSE);
    ledcWrite(MOTOR_CHANNEL, 0);
    currentMotorPWM = 0;
}

bool ServoManager::validatePins() {
    if (ELEVATOR_PIN == MOTOR_PIN) {
        #if DEBUG_MODE
            Serial.println("❌ Конфликт пинов!");
        #endif
        return false;
    }
    #if DEBUG_MODE
        Serial.println("✅ Конфигурация пинов корректна");
    #endif
    return true;
}