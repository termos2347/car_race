#include "ServoManager.h"
#include <Arduino.h>

ServoManager::ServoManager()
    : L_elevatorServo(HardwareConfig::L_ELEVATOR_PIN, L_ELEVATOR_MIN, L_ELEVATOR_MAX, L_ELEVATOR_NEUTRAL, "L_ELEVATOR"),
      R_elevatorServo(HardwareConfig::R_ELEVATOR_PIN, R_ELEVATOR_MIN, R_ELEVATOR_MAX, R_ELEVATOR_NEUTRAL, "R_ELEVATOR"),
      L_rudderServo(HardwareConfig::L_RUDDER_PIN, L_RUDDER_MIN, L_RUDDER_MAX, L_RUDDER_NEUTRAL, "L_RUDDER"),
      R_rudderServo(HardwareConfig::R_RUDDER_PIN, R_RUDDER_MIN, R_RUDDER_MAX, R_RUDDER_NEUTRAL, "R_RUDDER"),
      L_aileronServo(HardwareConfig::L_AILERON_PIN, L_AILERON_MIN, L_AILERON_MAX, L_AILERON_NEUTRAL, "L_LEFT_AILERON"),
      R_aileronServo(HardwareConfig::R_AILERON_PIN, R_AILERON_MIN, R_AILERON_MAX, R_AILERON_NEUTRAL, "R_RIGHT_AILERON"),
      L_flapServo(HardwareConfig::L_FLAPS_PIN, L_FLAPS_MIN, L_FLAPS_MAX, L_FLAPS_NEUTRAL, "L_FLAPS"),
      R_flapServo(HardwareConfig::R_FLAPS_PIN, R_FLAPS_MIN, R_FLAPS_MAX, R_FLAPS_NEUTRAL, "R_FLAPS"),
      motorServo(HardwareConfig::MOTOR_PIN, MOTOR_MIN, MOTOR_MAX, MOTOR_NEUTRAL, "MOTOR")
{
    isMotorArmed = false;
    firstMotorUpdate = true;
}

void ServoManager::begin() {
    Serial.println("🚀 START ServoManager");
    Serial.println("📌 Power Safety Configuration:");
    Serial.print("   - Smooth Movement: ");
    Serial.println(SMOOTH_SERVO_MOVEMENT ? "ENABLED" : "DISABLED");
    Serial.print("   - Safe Test Mode: ");
    Serial.println(SAFE_TEST_MODE ? "ENABLED" : "DISABLED");
    Serial.println("   - Test Type: SIMULTANEOUS (All servos together)");
    
    delay(3000);

    // Инициализация сервоприводов
    Serial.println("🎯 Initializing servos...");
    L_elevatorServo.begin();
    R_elevatorServo.begin();
    L_rudderServo.begin();
    R_rudderServo.begin();
    L_aileronServo.begin();
    R_aileronServo.begin();
    L_flapServo.begin();
    R_flapServo.begin();
    motorServo.begin();
    
    // Безопасный запуск двигателя
    safeMotorStart();
    
    // Запуск тестов в зависимости от настроек
    #if SAFE_TEST_MODE
        safeTestSequence();
    #else
        simultaneousTestSequence();
    #endif
    
    Serial.println("✅ ALL Servos INIT OK");
}

void ServoManager::safeMotorStart() {
    Serial.println("🔧 Motor Safe Start Sequence - SIMPLE");
    
    // Простая калибровка для реверса
    motorServo.write(MOTOR_MAX);
    Serial.println("   ⚡ MAX FORWARD");
    delay(2000);
    
    motorServo.write(MOTOR_MIN);
    Serial.println("   🔄 MAX REVERSE");
    delay(2000);
    
    motorServo.write(MOTOR_NEUTRAL);
    Serial.println("   ✅ NEUTRAL - READY");
    delay(2000);
    
    isMotorArmed = true;
    firstMotorUpdate = true;
    
    Serial.println("✅ Motor ARMED - Simple control active");
}

void ServoManager::testMotorSequence() {
    Serial.println("🎯 MOTOR Test Sequence");
    Serial.println("⚠️  WARNING: PROPELLER REMOVED?");
    
    if (!isMotorArmed) {
        Serial.println("❌ Motor NOT armed - skipping test");
        return;
    }
    
    // Тест 1: Нейтраль
    Serial.println("🎯 TEST 1: Motor NEUTRAL");
    motorServo.write(MOTOR_NEUTRAL);
    delay(2000);
    
    // Тест 2: Плавное увеличение до 25%
    Serial.println("🎯 TEST 2: Motor 25% power");
    for (int i = MOTOR_NEUTRAL; i <= MOTOR_NEUTRAL + 45; i += 5) {
        motorServo.write(i);
        Serial.print("   Power: ");
        Serial.print(i);
        Serial.println("/180");
        delay(300);
    }
    delay(2000);
    
    // Тест 3: Вернуться к 10%
    Serial.println("🎯 TEST 3: Motor 10% power");
    motorServo.write(MOTOR_NEUTRAL + 18);
    delay(2000);
    
    // Тест 4: Нейтраль
    Serial.println("🎯 TEST 4: Motor NEUTRAL");
    motorServo.write(MOTOR_NEUTRAL);
    delay(2000);
    
    Serial.println("✅ Motor test COMPLETE");
}

void ServoManager::moveAllServos(int L_elevator, int R_elevator, int L_rudder, int R_rudder, 
                                int L_aileron, int R_aileron, int L_flaps, int R_flaps, int motor) {
    // Сервоприводы
    L_elevatorServo.write(L_elevator);
    R_elevatorServo.write(R_elevator);
    L_rudderServo.write(L_rudder);
    R_rudderServo.write(R_rudder);
    L_aileronServo.write(L_aileron);
    R_aileronServo.write(R_aileron);
    L_flapServo.write(L_flaps);
    R_flapServo.write(R_flaps);
    
    // Двигатель - безопасное ограничение для тестов
    if (isMotorArmed) {
        int safeMotor = constrain(motor, MOTOR_NEUTRAL, MOTOR_NEUTRAL + 60); // Увеличил до 60
        motorServo.write(safeMotor);
        
        // Вывод для отладки
        Serial.print("   Motor: ");
        Serial.print(safeMotor);
        Serial.print("/180 (");
        Serial.print((safeMotor * 100) / 180);
        Serial.println("%)");
    } else {
        motorServo.write(MOTOR_NEUTRAL);
    }
}

void ServoManager::simultaneousTestSequence() {
    Serial.println("🧪 SIMULTANEOUS Servo Test Sequence");
    Serial.println("🎯 ALL servos moving TOGETHER at the same time!");
    Serial.println("⚠️  MOTOR LIMITED TO SAFE RANGE FOR TESTING");
    isTesting = true;
    
    // ТЕСТ 0: Отдельный тест двигателя
    testMotorSequence();

    // ТЕСТ 1: Все в нейтральное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 1: ALL SERVOS → NEUTRAL");
    moveAllServos(L_ELEVATOR_NEUTRAL, R_ELEVATOR_NEUTRAL, L_RUDDER_NEUTRAL, R_RUDDER_NEUTRAL,
                  L_AILERON_NEUTRAL, R_AILERON_NEUTRAL,
                  L_FLAPS_NEUTRAL, R_FLAPS_NEUTRAL, 
                  MOTOR_NEUTRAL);
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 2: Все в минимальное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 2: ALL SERVOS → MINIMUM");
    moveAllServos(L_ELEVATOR_MIN, R_ELEVATOR_MIN, 
                  L_RUDDER_MIN, R_RUDDER_MIN,
                  L_AILERON_MIN, R_AILERON_MIN,
                  L_FLAPS_MIN, R_FLAPS_MIN, 
                  MOTOR_NEUTRAL); // Двигатель остается в нейтрали
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 3: Все в максимальное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 3: ALL SERVOS → MAXIMUM");
    moveAllServos(L_ELEVATOR_MAX, R_ELEVATOR_MAX, 
                  L_RUDDER_MAX, R_RUDDER_MAX,
                  L_AILERON_MAX, R_AILERON_MAX,
                  L_FLAPS_MAX, R_FLAPS_MAX, 
                  MOTOR_NEUTRAL); // Двигатель остается в нейтрали
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 4: Элероны в противофазе
    Serial.println("🎯 TEST 4: AILERONS ANTI-PHASE");
    moveAllServos(L_ELEVATOR_NEUTRAL, R_ELEVATOR_NEUTRAL,
                  L_RUDDER_NEUTRAL, R_RUDDER_NEUTRAL,
                  L_AILERON_MAX, R_AILERON_MIN,
                  L_FLAPS_NEUTRAL, R_FLAPS_NEUTRAL,
                  MOTOR_NEUTRAL);
    delay(TEST_DELAY_SHORT);
    
    // ТЕСТ 5: Руль направления + закрылки
    Serial.println("🎯 TEST 5: RUDDER + FLAPS");
    moveAllServos(L_ELEVATOR_NEUTRAL, R_ELEVATOR_NEUTRAL,
                  L_RUDDER_MAX, R_RUDDER_MAX,
                  L_AILERON_NEUTRAL, R_AILERON_NEUTRAL,
                  L_FLAPS_MAX, R_FLAPS_MAX,
                  MOTOR_NEUTRAL);
    delay(TEST_DELAY_SHORT);
    
    // ФИНАЛ: Все обратно в нейтральное
    Serial.println("🎯 FINAL: ALL SERVOS → NEUTRAL");
    moveAllServos(L_ELEVATOR_NEUTRAL, R_ELEVATOR_NEUTRAL, 
                  L_RUDDER_NEUTRAL, R_RUDDER_NEUTRAL,
                  L_AILERON_NEUTRAL, R_AILERON_NEUTRAL,
                  L_FLAPS_NEUTRAL, R_FLAPS_NEUTRAL, 
                  MOTOR_NEUTRAL);
    delay(TEST_DELAY_SHORT);
    
    Serial.println("✅ SIMULTANEOUS Tests COMPLETE - All servos moved together!");
    isTesting = false;
}

void ServoManager::safeTestSequence() {
    Serial.println("🧪 SAFE Servo Test Sequence");
    Serial.println("🎯 Testing ONE servo at a time for power safety");
    isTesting = true;
    
    #if TEST_ELEVATOR
        Serial.println("🎯 Testing ELEVATOR");
        L_elevatorServo.testSequence();
        delay(TEST_DELAY_LONG);
        R_elevatorServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_RUDDER
        Serial.println("🎯 Testing RUDDER");
        L_rudderServo.testSequence();
        delay(TEST_DELAY_LONG);
        R_rudderServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_AILERONS
        Serial.println("🎯 Testing AILERONS");
        L_aileronServo.testSequence();
        delay(TEST_DELAY_SHORT);
        R_aileronServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_FLAPS
        Serial.println("🎯 Testing FLAPS");
        L_flapServo.testSequence();
        delay(TEST_DELAY_LONG);
        R_flapServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_MOTOR
        Serial.println("🎯 Testing MOTOR (Safe Mode)");
        Serial.println("⚠️  Motor test - SAFE RANGE ONLY");
        
        // Безопасный тест двигателя - только минимальные значения
        motorServo.write(MOTOR_NEUTRAL);
        delay(1000);
        
        // Небольшое увеличение тяги для теста
        motorServo.write(MOTOR_NEUTRAL + 10);
        delay(1000);
        
        // Еще небольшое увеличение
        motorServo.write(MOTOR_NEUTRAL + 20);
        delay(1000);
        
        // Возврат в нейтраль
        motorServo.write(MOTOR_NEUTRAL);
        delay(1000);
        
        Serial.println("✅ Motor test completed safely");
    #endif
    
    Serial.println("✅ SAFE Tests COMPLETE");
    isTesting = false;
}

void ServoManager::testSequence() {
    simultaneousTestSequence();
}

void ServoManager::applyDeadZone(int16_t& axisValue, int deadZone) {
    if (abs(axisValue) < deadZone) {
        axisValue = 0;
    }
}

void ServoManager::updateAilerons(int rollValue) {
    int leftAileronAngle = map(rollValue, -512, 512, L_AILERON_MAX, L_AILERON_MIN);
    int rightAileronAngle = map(rollValue, -512, 512, R_AILERON_MIN, R_AILERON_MAX);
    
    L_aileronServo.write(leftAileronAngle);
    R_aileronServo.write(rightAileronAngle);
}

void ServoManager::updateAileronsSmooth(int rollValue) {
    int L_aileronAngle = map(rollValue, -512, 512, L_AILERON_MAX, L_AILERON_MIN);
    int R_aileronAngle = map(rollValue, -512, 512, R_AILERON_MIN, R_AILERON_MAX);
    
    L_aileronServo.writeSmooth(L_aileronAngle, SERVO_SPEED_FAST);
    R_aileronServo.writeSmooth(R_aileronAngle, SERVO_SPEED_FAST);
}

void ServoManager::updateFlaps(int flapsValue) {
    int L_flapsAngle, R_flapsAngle;
    
    // Управление закрылками через кнопки
    if (flapsValue < -300) {
        // Закрылки убраны
        L_flapsAngle = L_FLAPS_MIN;
        R_flapsAngle = R_FLAPS_MIN;
    } else if (flapsValue > 300) {
        // Закрылки выпущены
        L_flapsAngle = L_FLAPS_MAX;
        R_flapsAngle = R_FLAPS_MAX;
    } else {
        // Нейтральное положение
        L_flapsAngle = L_FLAPS_NEUTRAL;
        R_flapsAngle = R_FLAPS_NEUTRAL;
    }
    
    L_flapServo.write(L_flapsAngle);
    R_flapServo.write(R_flapsAngle);
}

void ServoManager::updateFlapsSmooth(int flapsValue) {
    int L_flapsAngle, R_flapsAngle;
    
    // Управление закрылками через кнопки
    if (flapsValue < -300) {
        L_flapsAngle = L_FLAPS_MIN;
        R_flapsAngle = R_FLAPS_MIN;
    } else if (flapsValue > 300) {
        L_flapsAngle = L_FLAPS_MAX;
        R_flapsAngle = R_FLAPS_MAX;
    } else {
        L_flapsAngle = L_FLAPS_NEUTRAL;
        R_flapsAngle = R_FLAPS_NEUTRAL;
    }
    
    L_flapServo.writeSmooth(L_flapsAngle, SERVO_SPEED_SLOW);
    R_flapServo.writeSmooth(R_flapsAngle, SERVO_SPEED_SLOW);
}

void ServoManager::update(const ControlData& data) {
    if (isTesting) {
        return;
    }
    
    // 🔥 ПРИОРИТЕТНОЕ УПРАВЛЕНИЕ ДВИГАТЕЛЕМ (выполняется первым)
    if (isMotorArmed && !firstMotorUpdate) {
        // 🔧 ИСПОЛЬЗУЕМ СЫРЫЕ ДАННЫЕ ДЖОЙСТИКА БЕЗ ПРЕОБРАЗОВАНИЯ
        // Просто передаем значение оси джойстика напрямую в двигатель
        int motorSpeed = data.yAxis2;
        
        // 🔥 ОТПРАВЛЯЕМ КОМАНДУ ДВИГАТЕЛЮ СРАЗУ ЖЕ
        motorServo.write(motorSpeed);
    }
    
    // Первое обновление - гарантированная нейтраль
    if (firstMotorUpdate) {
        motorServo.write(MOTOR_NEUTRAL);
        firstMotorUpdate = false;
        Serial.println("🔄 First motor update - SAFETY NEUTRAL");
    }
    
    // ОБРАБОТКА СЕРВОПРИВОДОВ (выполняется после двигателя)
    ControlData processedData = data;
    applyDeadZone(processedData.xAxis1, DEADZONE_XAXIS1);
    applyDeadZone(processedData.yAxis1, DEADZONE_YAXIS1);
    applyDeadZone(processedData.xAxis2, DEADZONE_XAXIS2);
    
    // Основные органы управления
    int L_elevatorAngle = map(processedData.yAxis1, -512, 512, L_ELEVATOR_MIN, L_ELEVATOR_MAX);
    int R_elevatorAngle = map(processedData.yAxis1, -512, 512, R_ELEVATOR_MIN, R_ELEVATOR_MAX);
    int L_rudderAngle = map(processedData.xAxis1, -512, 512, L_RUDDER_MIN, L_RUDDER_MAX);
    int R_rudderAngle = map(processedData.xAxis1, -512, 512, R_RUDDER_MIN, R_RUDDER_MAX);
    
    // ЗАКРЫЛКИ: управление от кнопок
    int flapsValue = 0;
    if (processedData.button1) {
        flapsValue = 512;
    } else if (processedData.button2) {
        flapsValue = -512;
    }
    
    // Применяем управление сервоприводами
    #if SMOOTH_SERVO_MOVEMENT
        L_elevatorServo.writeSmooth(L_elevatorAngle, SERVO_SPEED_MEDIUM);
        R_elevatorServo.writeSmooth(R_elevatorAngle, SERVO_SPEED_MEDIUM);
        L_rudderServo.writeSmooth(L_rudderAngle, SERVO_SPEED_MEDIUM);
        R_rudderServo.writeSmooth(R_rudderAngle, SERVO_SPEED_MEDIUM);
        updateAileronsSmooth(processedData.xAxis2);
        updateFlapsSmooth(flapsValue);
    #else
        L_elevatorServo.write(L_elevatorAngle);
        R_elevatorServo.write(R_elevatorAngle);
        L_rudderServo.write(L_rudderAngle);
        R_rudderServo.write(R_rudderAngle);
        updateAilerons(processedData.xAxis2);
        updateFlaps(flapsValue);
    #endif
    
    // Упрощенная диагностика
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 300) {
        const char* motorDirection = "STOP";
        int currentMotorSpeed = data.yAxis2;
        
        if (currentMotorSpeed > 50) motorDirection = "FWD";
        else if (currentMotorSpeed < -50) motorDirection = "REV";
        
        Serial.print("⚡ Motor RAW:");
        Serial.print(currentMotorSpeed);
        Serial.print(" ");
        Serial.print(motorDirection);
        Serial.print(" Throttle:");
        Serial.print(data.yAxis2);
        Serial.println();
        
        lastPrint = millis();
    }
}