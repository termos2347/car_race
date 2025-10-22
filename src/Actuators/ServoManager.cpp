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
    
    // ТЕСТ СИГНАЛА НА ПИНАХ БЕЗ БИБЛИОТЕКИ
    #if DEBUG_MODE
        Serial.println("🎯 Тест прямого управления пинами...");
    #endif
    
    // Тест 1: Прямое управление PWM без библиотеки
    testDirectPWM();
    
    // Тест 2: Инициализация через библиотеку Servo
    #if DEBUG_MODE
        Serial.println("🔧 Инициализация через библиотеку Servo...");
    #endif
    
    // Пробуем разные таймеры
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    
    elevatorServo.setPeriodHertz(50);
    
    // Пробуем разные диапазоны импульсов
    bool attached = false;
    int ranges[][2] = {{500, 2500}, {1000, 2000}, {800, 2200}};
    
    for(int i = 0; i < 3; i++) {
        attached = elevatorServo.attach(ELEVATOR_PIN, ranges[i][0], ranges[i][1]);
        if(attached) {
            #if DEBUG_MODE
                Serial.printf("✅ Servo прикреплен с диапазоном %d-%d мкс\n", ranges[i][0], ranges[i][1]);
            #endif
            break;
        } else {
            #if DEBUG_MODE
                Serial.printf("❌ Не удалось прикрепить с диапазоном %d-%d мкс\n", ranges[i][0], ranges[i][1]);
            #endif
        }
    }
    
    if(!attached) {
        #if DEBUG_MODE
            Serial.println("❌ КРИТИЧЕСКАЯ ОШИБКА: Servo не прикреплен!");
        #endif
        return;
    }
    
    // Инициализация двигателя
    ledcSetup(MOTOR_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttachPin(MOTOR_PIN, MOTOR_CHANNEL);
    ledcWrite(MOTOR_CHANNEL, 0);
    
    #if DEBUG_MODE
        Serial.println("✅ ServoManager инициализирован");
    #endif
}

void ServoManager::testDirectPWM() {
    #if DEBUG_MODE
        Serial.println("🔌 Тест прямого PWM на пине сервопривода...");
    #endif
    
    // Настройка PWM вручную
    ledcSetup(8, 50, 16); // Канал 8, 50Hz, 16 бит
    ledcAttachPin(ELEVATOR_PIN, 8);
    
    // Тест ручного PWM (аналогично Servo библиотеке)
    int positions[] = {500, 1500, 2500, 1500};
    const char* positionNames[] = {"MIN", "NEUTRAL", "MAX", "NEUTRAL"};
    
    for(int i = 0; i < 4; i++) {
        int pulseWidth = positions[i];
        // Конвертация микросекунд в duty cycle для 50Hz
        uint32_t duty = (pulseWidth * 65535) / 20000;
        
        ledcWrite(8, duty);
        #if DEBUG_MODE
            Serial.printf("   📍 %s: %d мкс (duty: %d)\n", positionNames[i], pulseWidth, duty);
        #endif
        delay(2000);
    }
    
    // Отключаем ручной PWM
    ledcDetachPin(ELEVATOR_PIN);
    #if DEBUG_MODE
        Serial.println("✅ Тест прямого PWM завершен");
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
    
    // Тест 1: Проверка питания - минимальное движение
    #if DEBUG_MODE
        Serial.println("🔋 Тест 1: Проверка питания (медленное движение)");
    #endif
    
    for (int i = 1500; i >= 1000; i -= 10) {
        elevatorServo.writeMicroseconds(i);
        #if DEBUG_MODE
            Serial.printf("   📍 Положение: %d мкс\n", i);
        #endif
        delay(100);
    }
    delay(1000);
    
    for (int i = 1000; i <= 2000; i += 10) {
        elevatorServo.writeMicroseconds(i);
        #if DEBUG_MODE
            Serial.printf("   📍 Положение: %d мкс\n", i);
        #endif
        delay(100);
    }
    delay(1000);
    
    // Тест 2: Резкие движения (проверка мощности)
    #if DEBUG_MODE
        Serial.println("⚡ Тест 2: Резкие движения (проверка мощности)");
    #endif
    
    int testPositions[] = {1000, 1500, 2000, 1500, 1000, 1500};
    
    for(int i = 0; i < 6; i++) {
        elevatorServo.writeMicroseconds(testPositions[i]);
        #if DEBUG_MODE
            Serial.printf("   🔄 Резкое движение: %d мкс\n", testPositions[i]);
        #endif
        delay(1000);
    }
    
    // Тест 3: Двигатель
    #if DEBUG_MODE
        Serial.println("🚀 Тест 3: Двигатель");
    #endif
    
    for(int pwm = 0; pwm <= 255; pwm += 50) {
        ledcWrite(MOTOR_CHANNEL, pwm);
        #if DEBUG_MODE
            Serial.printf("   🔧 PWM двигателя: %d/255\n", pwm);
        #endif
        delay(1000);
    }
    ledcWrite(MOTOR_CHANNEL, 0);
    
    // Возврат в нейтраль
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