#include "ServoManager.h"
#include <Arduino.h>

ServoManager::ServoManager()
    : L_elevatorServo(HardwareConfig::L_ELEVATOR_PIN, L_ELEVATOR_MIN, L_ELEVATOR_MAX, L_ELEVATOR_NEUTRAL, "L_ELEVATOR", SERVO_MIN_PULSE, SERVO_MAX_PULSE),
      R_elevatorServo(HardwareConfig::R_ELEVATOR_PIN, R_ELEVATOR_MIN, R_ELEVATOR_MAX, R_ELEVATOR_NEUTRAL, "R_ELEVATOR", SERVO_MIN_PULSE, SERVO_MAX_PULSE),
      L_rudderServo(HardwareConfig::L_RUDDER_PIN, L_RUDDER_MIN, L_RUDDER_MAX, L_RUDDER_NEUTRAL, "L_RUDDER", SERVO_MIN_PULSE, SERVO_MAX_PULSE),
      R_rudderServo(HardwareConfig::R_RUDDER_PIN, R_RUDDER_MIN, R_RUDDER_MAX, R_RUDDER_NEUTRAL, "R_RUDDER", SERVO_MIN_PULSE, SERVO_MAX_PULSE),
      L_aileronServo(HardwareConfig::L_AILERON_PIN, L_AILERON_MIN, L_AILERON_MAX, L_AILERON_NEUTRAL, "L_LEFT_AILERON", SERVO_MIN_PULSE, SERVO_MAX_PULSE),
      R_aileronServo(HardwareConfig::R_AILERON_PIN, R_AILERON_MIN, R_AILERON_MAX, R_AILERON_NEUTRAL, "R_RIGHT_AILERON", SERVO_MIN_PULSE, SERVO_MAX_PULSE),
      L_flapServo(HardwareConfig::L_FLAPS_PIN, L_FLAPS_MIN, L_FLAPS_MAX, L_FLAPS_NEUTRAL, "L_FLAPS", SERVO_MIN_PULSE, SERVO_MAX_PULSE),
      R_flapServo(HardwareConfig::R_FLAPS_PIN, R_FLAPS_MIN, R_FLAPS_MAX, R_FLAPS_NEUTRAL, "R_FLAPS", SERVO_MIN_PULSE, SERVO_MAX_PULSE),
      motorServo(HardwareConfig::MOTOR_PIN, MOTOR_MIN, MOTOR_MAX, MOTOR_NEUTRAL, "MOTOR", MOTOR_MIN_PULSE, MOTOR_MAX_PULSE)
{
    motorArmed = false;
    firstMotorUpdate = true;
    testsEnabled = false;
}

void ServoManager::begin() {
    Serial.println("🚀 ServoManager - FLIGHT MODE");
    Serial.println("📌 Configuration:");
    Serial.print("   - Smooth Movement: ");
    Serial.println(SMOOTH_SERVO_MOVEMENT ? "ENABLED" : "DISABLED");
    
    delay(100);
    
    // Инициализация сервоприводов управления
    Serial.println("🎯 Initializing servos...");
    L_elevatorServo.begin();
    R_elevatorServo.begin();
    L_rudderServo.begin();
    R_rudderServo.begin();
    L_aileronServo.begin();
    R_aileronServo.begin();
    L_flapServo.begin();
    R_flapServo.begin();
    
    // 🔥 КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: ПРАВИЛЬНАЯ ИНИЦИАЛИЗАЦИЯ ESC ДЛЯ BLHeli
    Serial.println("\n🔧 ESC Initialization (BLHeli)");
    Serial.println("⚠️  IMPORTANT: Follow steps carefully!");
    Serial.println("1. PROPELLER REMOVED?");
    Serial.println("2. Battery DISCONNECTED from ESC");
    Serial.println("3. Wait for signal...");
    
    // 1. Инициализируем ESC
    motorServo.begin();  // Вызовет attach с импульсами 1000-2000μs
    
    // 2. Отправляем STOP сигнал (БАТАРЕЯ ОТКЛЮЧЕНА)
    Serial.println("\n🎯 STEP 1: Sending STOP signal (1000μs) - NO BATTERY");
    motorServo.writeMicroseconds(1000);
    delay(1000);
    
    // 3. Говорим подключить батарею
    Serial.println("\n⚠️  ⚠️  ⚠️  NOW: CONNECT BATTERY to ESC! ⚠️  ⚠️  ⚠️");
    Serial.println("   Wait for 3 beeps (cell count)...");
    delay(3000);  // Даем время подключить батарею
    
    // 4. Ждем завершения инициализации ESC
    Serial.println("\n🎯 STEP 2: Waiting for ESC initialization...");
    Serial.println("   You should hear 1 more beep (signal received)");
    delay(2000);
    
    // 5. BLHeli АКТИВАЦИЯ: максимум на 1 секунду
    Serial.println("\n🎯 STEP 3: BLHeli activation sequence");
    Serial.println("   Sending 2000μs (max) for 1 second...");
    motorServo.writeMicroseconds(2000);
    delay(1000);
    
    // 6. Возвращаем STOP
    Serial.println("   Sending 1000μs (stop)...");
    motorServo.writeMicroseconds(1000);
    delay(1000);
    
    // 7. Проверка работы
    Serial.println("\n🎯 STEP 4: Testing ESC (1200μs = 10% power)...");
    motorServo.writeMicroseconds(1200);
    delay(500);
    
    Serial.println("   Returning to STOP (1000μs)...");
    motorServo.writeMicroseconds(1000);
    delay(500);
    
    motorArmed = true;
    firstMotorUpdate = true;
    
    Serial.println("\n✅ ESC ARMED and READY for BLHeli");
    Serial.println("✅ All servos READY for flight");
    Serial.println("\n📝 Send 'h' for available commands");
    
    // Сбрасываем флаг BLHeli активации (уже сделали в begin)
    blheliFirstRun = false;
    blheliActivationStart = 0;
    blheliActivationStep = 0;
}

void ServoManager::runManualTests() {
    Serial.println("🧪 MANUAL TEST SEQUENCE");
    Serial.println("⚠️  WARNING: Ensure propeller is removed!");
    Serial.println("Send 'y' to confirm or any key to cancel...");
    
    // Ждем подтверждения 5 секунд
    unsigned long start = millis();
    while (millis() - start < 5000) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == 'y' || c == 'Y') {
                Serial.println("✅ Starting full test sequence...");
                simultaneousTestSequence();
                return;
            } else {
                Serial.println("❌ Test cancelled");
                return;
            }
        }
    }
    Serial.println("⏰ Timeout - test cancelled");
}

void ServoManager::calibrateESC() {
    Serial.println("\n🎛️ ESC CALIBRATION MODE");
    Serial.println("⚠️  ⚠️  ⚠️  WARNING: REMOVE PROPELLER! ⚠️  ⚠️  ⚠️");
    Serial.println("\n📋 Procedure:");
    Serial.println("1. Disconnect battery from ESC");
    Serial.println("2. Send 'y' to start calibration");
    Serial.println("3. Follow instructions");
    
    while (!Serial.available()) delay(100);
    if (Serial.read() != 'y') {
        Serial.println("❌ Calibration cancelled");
        return;
    }
    
    Serial.println("\n🔧 Starting calibration...");
    
    // ШАГ 1: Подготовка
    Serial.println("\n🎯 STEP 1: Disconnect battery from ESC");
    Serial.println("   Ensure battery is DISCONNECTED");
    Serial.println("   Press any key when ready...");
    while (!Serial.available()) delay(100);
    Serial.read();
    
    // ШАГ 2: Максимальный газ
    Serial.println("\n🎯 STEP 2: Sending MAX signal (2000μs)");
    motorServo.writeMicroseconds(2000);
    
    Serial.println("⚠️  NOW: Connect battery to ESC!");
    Serial.println("   Wait for beeps (2-3 beeps)");
    delay(8000);
    
    // ШАГ 3: Минимальный газ
    Serial.println("\n🎯 STEP 3: Sending MIN signal (1000μs)");
    motorServo.writeMicroseconds(1000);
    Serial.println("   Wait for confirmation beeps (1 long beep)");
    delay(8000);
    
    // ШАГ 4: Готово
    Serial.println("\n✅ Calibration complete!");
    Serial.println("✅ ESC is now calibrated to 1000-2000μs range");
    
    motorArmed = true;
    firstMotorUpdate = true;
    
    Serial.println("\n🔧 Testing calibration...");
    Serial.println("   Sending 1500μs (50% power)");
    motorServo.writeMicroseconds(1500);
    delay(3000);
    
    Serial.println("   Returning to STOP (1000μs)");
    motorServo.writeMicroseconds(1000);
    delay(1000);
    
    Serial.println("✅ ESC calibrated and ready!");
}

void ServoManager::safeStartSequence() {
    Serial.println("\n🔒 SAFE START SEQUENCE");
    Serial.println("📋 Follow these steps:");
    
    // 1. Проверка пропеллера
    Serial.println("\n1. ⚠️  PROPELLER REMOVED?");
    Serial.println("   Type 'y' to confirm or any key to cancel");
    
    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == 'y' || c == 'Y') {
                break;
            } else {
                Serial.println("❌ Cancelled - safety first!");
                return;
            }
        }
    }
    
    // 2. Отключение батареи
    Serial.println("\n2. 🔋 Disconnect battery from ESC");
    Serial.println("   Type 'y' when battery is disconnected");
    
    while (!Serial.available()) delay(100);
    if (Serial.read() != 'y') {
        Serial.println("❌ Cancelled");
        return;
    }
    
    // 3. Инициализация ESC
    Serial.println("\n3. 🔧 Initializing ESC...");
    motorServo.writeMicroseconds(1000);
    delay(1000);
    
    // 4. Подключение батареи
    Serial.println("\n4. 🔋 NOW: Connect battery to ESC");
    Serial.println("   Wait for beeps...");
    delay(5000);
    
    // 5. Тест
    Serial.println("\n5. 🎯 Testing ESC...");
    Serial.println("   Sending 1200μs (10% power)");
    motorServo.writeMicroseconds(1200);
    delay(2000);
    
    Serial.println("   Sending 1000μs (STOP)");
    motorServo.writeMicroseconds(1000);
    delay(1000);
    
    motorArmed = true;
    firstMotorUpdate = true;
    
    Serial.println("\n✅ SAFE START COMPLETE");
    Serial.println("✅ ESC armed and ready");
}

void ServoManager::escTestSimple() {
    Serial.println("🎯 SIMPLE ESC TEST (using microseconds)");
    
    if (!motorArmed) {
        Serial.println("⚠️  Arming ESC first...");
        motorServo.writeMicroseconds(1000);
        delay(2000);
        motorArmed = true;
    }
    
    int testValues[] = {1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000};
    const char* labels[] = {"STOP", "5%", "10%", "15%", "20%", "25%", "30%", "35%", "40%", "45%", "50%"};
    
    for (int i = 0; i < 11; i++) {
        Serial.print("🎯 ");
        Serial.print(labels[i]);
        Serial.print(" (");
        Serial.print(testValues[i]);
        Serial.println("μs)");
        
        motorServo.writeMicroseconds(testValues[i]);
        delay(2000);
    }
    
    // Возврат в STOP
    motorServo.writeMicroseconds(1000);
    Serial.println("✅ Test complete - ESC STOPPED");
}

void ServoManager::safeMotorStart() {
    Serial.println("🔧 Motor Safe Start - FULL RANGE -512 to +512");
    
    // Калибровка с полным диапазоном
    motorServo.write(180);
    Serial.println("   ⚡ MAX FORWARD (180)");
    delay(2000);
    
    motorServo.write(0);
    Serial.println("   🔄 MAX REVERSE (0)");
    delay(2000);
    
    motorServo.write(0);
    Serial.println("   ✅ NEUTRAL - READY");
    delay(2000);
    
    motorArmed = true;
    firstMotorUpdate = true;
    
    Serial.println("✅ Motor ARMED - Full range mapping active");
}

void ServoManager::testMotorSequence() {
    Serial.println("🎯 MOTOR Test Sequence");
    Serial.println("⚠️  WARNING: PROPELLER REMOVED?");
    
    if (!motorArmed) {
        Serial.println("❌ Motor NOT armed - arming now...");
        motorServo.write(0);  // Минимальный газ
        delay(2000);
        motorArmed = true;
        firstMotorUpdate = true;
    }
    
    // Тест 1: Нейтраль
    Serial.println("🎯 TEST 1: Motor NEUTRAL (0%)");
    motorServo.write(0);
    delay(2000);
    
    // Тест 2: Плавное увеличение до 25%
    Serial.println("🎯 TEST 2: Motor 25% power");
    for (int i = 0; i <= 45; i += 5) {
        motorServo.write(i);
        Serial.print("   Power: ");
        Serial.print(i);
        Serial.print("° (");
        Serial.print(map(i, 0, 180, 0, 100));
        Serial.println("%)");
        delay(500);  // Увеличил задержку для ESC
    }
    delay(2000);
    
    // Тест 3: Плавное увеличение до 50%
    Serial.println("🎯 TEST 3: Motor 50% power");
    for (int i = 45; i <= 90; i += 5) {
        motorServo.write(i);
        Serial.print("   Power: ");
        Serial.print(i);
        Serial.println("/180");
        delay(300);
    }
    delay(2000);
    
    // Тест 4: Плавное уменьшение до 10%
    Serial.println("🎯 TEST 4: Motor 10% power");
    for (int i = 90; i >= 18; i -= 5) {
        motorServo.write(i);
        Serial.print("   Power: ");
        Serial.print(i);
        Serial.println("/180");
        delay(300);
    }
    delay(2000);
    
    // Тест 5: Нейтраль
    Serial.println("🎯 TEST 5: Motor NEUTRAL");
    motorServo.write(0);
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
    int safeMotor = constrain(motor, 0, MOTOR_TEST_MAX);
    motorServo.write(safeMotor);
    
    // Вывод для отладки
    Serial.print("   Motor: ");
    Serial.print(safeMotor);
    Serial.print("/180 (");
    Serial.print((safeMotor * 100) / 180);
    Serial.println("%)");
}

void ServoManager::simultaneousTestSequence() {
    Serial.println("🧪 SIMULTANEOUS Servo Test Sequence");
    Serial.println("🎯 ALL servos moving TOGETHER at the same time!");
    Serial.println("⚠️  MOTOR LIMITED TO 33% FOR SAFETY TESTING");
    
    // Включаем тестовый режим, но не блокируем двигатель
    isTesting = true;
    
    // ТЕСТ 0: Отдельный тест двигателя
    Serial.println("🔧 Testing MOTOR separately first...");
    testMotorSequence();

    // ТЕСТ 1: Все в нейтральное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 1: ALL SERVOS → NEUTRAL");
    moveAllServos(L_ELEVATOR_NEUTRAL, R_ELEVATOR_NEUTRAL, L_RUDDER_NEUTRAL, R_RUDDER_NEUTRAL,
                  L_AILERON_NEUTRAL, R_AILERON_NEUTRAL,
                  L_FLAPS_NEUTRAL, R_FLAPS_NEUTRAL, 
                  0);
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 2: Все в минимальное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 2: ALL SERVOS → MINIMUM");
    moveAllServos(L_ELEVATOR_MIN, R_ELEVATOR_MIN, 
                  L_RUDDER_MIN, R_RUDDER_MIN,
                  L_AILERON_MIN, R_AILERON_MIN,
                  L_FLAPS_MIN, R_FLAPS_MIN, 
                  0);
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 3: Все в максимальное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 3: ALL SERVOS → MAXIMUM");
    moveAllServos(L_ELEVATOR_MAX, R_ELEVATOR_MAX, 
                  L_RUDDER_MAX, R_RUDDER_MAX,
                  L_AILERON_MAX, R_AILERON_MAX,
                  L_FLAPS_MAX, R_FLAPS_MAX, 
                  30); // Мотор на 30% одновременно с сервоприводами
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 4: Элероны в противофазе
    Serial.println("🎯 TEST 4: AILERONS ANTI-PHASE");
    moveAllServos(L_ELEVATOR_NEUTRAL, R_ELEVATOR_NEUTRAL,
                  L_RUDDER_NEUTRAL, R_RUDDER_NEUTRAL,
                  L_AILERON_MAX, R_AILERON_MIN,
                  L_FLAPS_NEUTRAL, R_FLAPS_NEUTRAL,
                  20);
    delay(TEST_DELAY_SHORT);
    
    // ТЕСТ 5: Руль направления + закрылки
    Serial.println("🎯 TEST 5: RUDDER + FLAPS");
    moveAllServos(L_ELEVATOR_NEUTRAL, R_ELEVATOR_NEUTRAL,
                  L_RUDDER_MAX, R_RUDDER_MAX,
                  L_AILERON_NEUTRAL, R_AILERON_NEUTRAL,
                  L_FLAPS_MAX, R_FLAPS_MAX,
                  25);
    delay(TEST_DELAY_SHORT);
    
    // ТЕСТ 6: Все сервоприводы + мотор плавно
    Serial.println("🎯 TEST 6: ALL SERVOS + MOTOR SMOOTH");
    for (int i = 0; i <= 30; i += 5) {
        moveAllServos(
            map(i, 0, 30, L_ELEVATOR_NEUTRAL, L_ELEVATOR_MAX),
            map(i, 0, 30, R_ELEVATOR_NEUTRAL, R_ELEVATOR_MAX),
            map(i, 0, 30, L_RUDDER_NEUTRAL, L_RUDDER_MAX),
            map(i, 0, 30, R_RUDDER_NEUTRAL, R_RUDDER_MAX),
            map(i, 0, 30, L_AILERON_NEUTRAL, L_AILERON_MAX),
            map(i, 0, 30, R_AILERON_NEUTRAL, R_AILERON_MAX),
            map(i, 0, 30, L_FLAPS_NEUTRAL, L_FLAPS_MAX),
            map(i, 0, 30, R_FLAPS_NEUTRAL, R_FLAPS_MAX),
            i
        );
        delay(200);
    }
    delay(1000);
    
    // ФИНАЛ: Все обратно в нейтральное
    Serial.println("🎯 FINAL: ALL SERVOS → NEUTRAL");
    moveAllServos(L_ELEVATOR_NEUTRAL, R_ELEVATOR_NEUTRAL, 
                  L_RUDDER_NEUTRAL, R_RUDDER_NEUTRAL,
                  L_AILERON_NEUTRAL, R_AILERON_NEUTRAL,
                  L_FLAPS_NEUTRAL, R_FLAPS_NEUTRAL, 
                  0);
    delay(TEST_DELAY_SHORT);
    
    Serial.println("✅ SIMULTANEOUS Tests COMPLETE - All servos moved together!");
    isTesting = false;
}

void ServoManager::safeTestSequence() {
    Serial.println("🧪 SAFE Servo Test Sequence");
    Serial.println("🎯 Testing ONE servo at a time for power safety");
    
    isTesting = true;
    
    Serial.println("🎯 Testing ELEVATOR");
    L_elevatorServo.testSequence();
    delay(TEST_DELAY_LONG);
    R_elevatorServo.testSequence();
    delay(TEST_DELAY_LONG);
    
    Serial.println("🎯 Testing RUDDER");
    L_rudderServo.testSequence();
    delay(TEST_DELAY_LONG);
    R_rudderServo.testSequence();
    delay(TEST_DELAY_LONG);
    
    Serial.println("🎯 Testing AILERONS");
    L_aileronServo.testSequence();
    delay(TEST_DELAY_SHORT);
    R_aileronServo.testSequence();
    delay(TEST_DELAY_LONG);
    
    Serial.println("🎯 Testing FLAPS");
    L_flapServo.testSequence();
    delay(TEST_DELAY_LONG);
    R_flapServo.testSequence();
    delay(TEST_DELAY_LONG);
    
    Serial.println("🎯 Testing MOTOR (Safe Mode)");
    Serial.println("⚠️  Motor test - SAFE RANGE ONLY");
    
    // Безопасный тест двигателя
    motorServo.write(0);
    delay(1000);
    
    for (int i = 0; i <= 30; i += 5) {
        motorServo.write(i);
        Serial.print("   Motor: ");
        Serial.print(i);
        Serial.println("/180");
        delay(500);
    }
    
    delay(1000);
    
    for (int i = 30; i >= 0; i -= 5) {
        motorServo.write(i);
        delay(300);
    }
    
    motorServo.write(0);
    delay(1000);
    
    Serial.println("✅ Motor test completed safely");
    
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
    
    L_flapServo.write(L_flapsAngle);
    R_flapServo.write(R_flapsAngle);
}

void ServoManager::updateFlapsSmooth(int flapsValue) {
    int L_flapsAngle, R_flapsAngle;
    
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

void ServoManager::testMotorDirect() {
    Serial.println("🔧 DIRECT MOTOR TEST (using microseconds)");
    
    // Принудительно вооружаем двигатель
    motorArmed = true;
    firstMotorUpdate = false;
    
    // Плавный разгон как в работающем тесте
    Serial.println("⚡ Smooth acceleration 1000-1500μs...");
    for (int us = 1000; us <= 1500; us += 10) {
        motorServo.writeMicroseconds(us);
        Serial.print("  Setting: ");
        Serial.print(us);
        Serial.println("μs");
        delay(100);
    }
    
    delay(2000);
    
    // Плавное торможение
    Serial.println("⚡ Smooth deceleration 1500-1000μs...");
    for (int us = 1500; us >= 1000; us -= 10) {
        motorServo.writeMicroseconds(us);
        delay(100);
    }
    
    Serial.println("✅ Direct motor test complete");
}

void ServoManager::directMotorTest(int powerPercent) {
    if (!motorArmed) {
        Serial.println("⚠️  Arming motor first...");
        motorServo.writeMicroseconds(1000);  // STOP
        delay(2000);
        motorArmed = true;
    }
    
    // Преобразуем проценты в микросекунды
    int us = map(powerPercent, 0, 100, 1000, 2000);
    us = constrain(us, 1000, 2000);
    
    Serial.print("🔧 Direct motor test: ");
    Serial.print(powerPercent);
    Serial.print("% = ");
    Serial.print(us);
    Serial.println("μs");
    
    motorServo.writeMicroseconds(us);
}

void ServoGroup::writeMicroseconds(int us) {
    servo.writeMicroseconds(us);
}

void ServoManager::blheliArmingSequence() {
    Serial.println("🔐 BLHeli ARMING SEQUENCE");
    Serial.println("⚠️  This is REQUIRED for BLHeli ESCs");
    
    // 1. Убедитесь, что батарея отключена
    Serial.println("\n1. Disconnect battery from ESC");
    Serial.println("   Press any key when ready...");
    while(!Serial.available());
    Serial.read();
    
    // 2. Инициализация ESC
    motorServo.begin();
    delay(100);
    
    // 3. Отправляем минимальный сигнал
    Serial.println("\n2. Sending 1000μs (min)");
    motorServo.writeMicroseconds(1000);
    delay(100);
    
    // 4. Подключаем батарею
    Serial.println("\n3. ⚡ NOW: Connect battery to ESC!");
    Serial.println("   Wait for 3 beeps (cell count)...");
    delay(5000);
    
    // 5. Специальная последовательность для BLHeli
    Serial.println("\n4. BLHeli arming sequence:");
    
    // 5a. Минимум 2 секунды
    Serial.println("   a. 1000μs for 2 seconds");
    motorServo.writeMicroseconds(1000);
    delay(2000);
    
    // 5b. Максимум 1 секунда
    Serial.println("   b. 2000μs for 1 second");
    motorServo.writeMicroseconds(2000);
    delay(1000);
    
    // 5c. Возврат к минимуму
    Serial.println("   c. 1000μs (armed)");
    motorServo.writeMicroseconds(1000);
    delay(1000);
    
    // 6. Проверка
    Serial.println("\n5. Testing...");
    Serial.println("   Sending 1200μs (10%)");
    motorServo.writeMicroseconds(1200);
    delay(2000);
    
    Serial.println("   Sending 1000μs (stop)");
    motorServo.writeMicroseconds(1000);
    delay(1000);
    
    motorArmed = true;
    firstMotorUpdate = true;
    
    Serial.println("\n✅ BLHeli ESC ARMED and READY!");
}

void ServoManager::update(const ControlData& data) {
    // 🔥 BLHeli АКТИВАЦИЯ - ТОЛЬКО ПЕРВЫЙ РАЗ (без блокировки)
    static bool blheliFirstRun = true;
    static unsigned long blheliActivationStart = 0;
    static int blheliActivationStep = 0;
    
    if (blheliFirstRun && motorArmed) {
        if (blheliActivationStep == 0) {
            Serial.println("\n⚡ BLHeli ACTIVATION: Starting in update()");
            Serial.println("   Sending 2000μs for 1 second...");
            motorServo.writeMicroseconds(2000);
            blheliActivationStart = millis();
            blheliActivationStep = 1;
        } 
        else if (blheliActivationStep == 1 && millis() - blheliActivationStart > 1000) {
            Serial.println("   Sending 1000μs (armed)...");
            motorServo.writeMicroseconds(1000);
            blheliActivationStart = millis();
            blheliActivationStep = 2;
        }
        else if (blheliActivationStep == 2 && millis() - blheliActivationStart > 1000) {
            blheliFirstRun = false;
            Serial.println("✅ BLHeli activation COMPLETE in update()");
            Serial.println("   ESC ready for normal operation!");
        }
        
        // Не обрабатываем обычное управление во время активации
        // Но сервоприводы будут работать (код ниже)
    }
    
    // ============================================================================
    // 🔥 УПРАВЛЕНИЕ ДВИГАТЕЛЕМ ЧЕРЕЗ МИКРОСЕКУНДЫ
    // ============================================================================
    
    // Если BLHeli активация завершена, управляем двигателем нормально
    if (motorArmed && !blheliFirstRun) {
        int motorMicroseconds = 1000;  // По умолчанию STOP
        
        // Преобразуем значение джойстика в микросекунды
        // yAxis2: от -512 (низ) до +512 (верх)
        
        if (data.yAxis2 > 10) {  // Добавляем мертвую зону 10
            // От 10 до 512 -> от 1100 до 2000 мкс
            motorMicroseconds = map(data.yAxis2, 10, 512, 1100, 2000);
            motorMicroseconds = constrain(motorMicroseconds, 1100, 2000);
            
            // Диагностика (раз в 500мс)
            static unsigned long lastMotorLog = 0;
            if (millis() - lastMotorLog > 500) {
                Serial.print("🎮 Motor: ");
                Serial.print(motorMicroseconds);
                Serial.print("μs (");
                Serial.print(map(motorMicroseconds, 1000, 2000, 0, 100));
                Serial.print("%), Joy: ");
                Serial.println(data.yAxis2);
                lastMotorLog = millis();
            }
        } else {
            // Джойстик в нейтрали или внизу -> STOP (1000μs)
            motorMicroseconds = 1000;
        }
        
        // 🔒 БЕЗОПАСНОСТЬ: Первое обновление всегда STOP
        if (firstMotorUpdate) {
            motorMicroseconds = 1000;
            firstMotorUpdate = false;
            Serial.println("🛡️ First motor update - SAFETY STOP (1000μs)");
        }
        
        // 🔧 Отправляем команду ESC
        motorServo.writeMicroseconds(motorMicroseconds);
        
    } else if (motorArmed && blheliFirstRun) {
        // Во время BLHeli активации двигатель управляется выше
        // Просто логируем состояние
        static unsigned long lastActivationLog = 0;
        if (millis() - lastActivationLog > 1000) {
            Serial.print("⏳ BLHeli activation: step ");
            Serial.print(blheliActivationStep);
            Serial.print("/2, time: ");
            Serial.print((millis() - blheliActivationStart) / 1000.0, 1);
            Serial.println("s");
            lastActivationLog = millis();
        }
    } else {
        // Двигатель не вооружен
        motorServo.writeMicroseconds(1000);  // STOP
        
        static unsigned long lastWarning = 0;
        if (millis() - lastWarning > 3000) {
            Serial.println("⚠️  Motor NOT armed! Send 'c' to calibrate or wait for BLHeli activation");
            lastWarning = millis();
        }
    }
    
    // ============================================================================
    // ⚠️ ЕСЛИ ТЕСТИРОВАНИЕ АКТИВНО - ВЫХОДИМ
    // ============================================================================
    if (isTesting) {
        // ❌ В тестовом режиме НЕ управляем сервоприводами от пульта
        // Но двигатель работает (управляется выше)
        return;
    }
    
    // ============================================================================
    // 🎮 НОРМАЛЬНОЕ УПРАВЛЕНИЕ СЕРВОПРИВОДАМИ
    // ============================================================================
    
    // Применяем мертвые зоны
    ControlData processedData = data;
    applyDeadZone(processedData.xAxis1, DEADZONE_XAXIS1);
    applyDeadZone(processedData.yAxis1, DEADZONE_YAXIS1);
    applyDeadZone(processedData.xAxis2, DEADZONE_XAXIS2);
    
    // Руль высоты
    int L_elevatorAngle = map(processedData.yAxis1, -512, 512, L_ELEVATOR_MIN, L_ELEVATOR_MAX);
    int R_elevatorAngle = map(processedData.yAxis1, -512, 512, R_ELEVATOR_MIN, R_ELEVATOR_MAX);
    
    // Руль направления
    int L_rudderAngle = map(processedData.xAxis1, -512, 512, L_RUDDER_MIN, L_RUDDER_MAX);
    int R_rudderAngle = map(processedData.xAxis1, -512, 512, R_RUDDER_MIN, R_RUDDER_MAX);
    
    // Элероны
    int L_aileronAngle, R_aileronAngle;
    if (processedData.xAxis2 >= 0) {
        L_aileronAngle = map(processedData.xAxis2, 0, 512, L_AILERON_NEUTRAL, L_AILERON_MIN);
        R_aileronAngle = map(processedData.xAxis2, 0, 512, R_AILERON_NEUTRAL, R_AILERON_MAX);
    } else {
        L_aileronAngle = map(processedData.xAxis2, -512, 0, L_AILERON_MAX, L_AILERON_NEUTRAL);
        R_aileronAngle = map(processedData.xAxis2, -512, 0, R_AILERON_MIN, R_AILERON_NEUTRAL);
    }
    
    // Закрылки
    int L_flapsAngle = L_FLAPS_NEUTRAL;
    int R_flapsAngle = R_FLAPS_NEUTRAL;
    if (processedData.button1) {
        L_flapsAngle = L_FLAPS_MIN;
        R_flapsAngle = R_FLAPS_MIN;
    } else if (processedData.button2) {
        L_flapsAngle = L_FLAPS_MAX;
        R_flapsAngle = R_FLAPS_MAX;
    }
    
    // Применяем управление
    #if SMOOTH_SERVO_MOVEMENT
        // Плавное движение сервоприводов
        L_elevatorServo.writeSmooth(L_elevatorAngle, SERVO_SPEED_MEDIUM);
        R_elevatorServo.writeSmooth(R_elevatorAngle, SERVO_SPEED_MEDIUM);
        L_rudderServo.writeSmooth(L_rudderAngle, SERVO_SPEED_MEDIUM);
        R_rudderServo.writeSmooth(R_rudderAngle, SERVO_SPEED_MEDIUM);
        
        // Элероны и закрылки с отдельными методами плавного движения
        updateAileronsSmooth(processedData.xAxis2);
        updateFlapsSmooth(processedData.button1 ? 512 : (processedData.button2 ? -512 : 0));
    #else
        // Прямое управление сервоприводами
        L_elevatorServo.write(L_elevatorAngle);
        R_elevatorServo.write(R_elevatorAngle);
        L_rudderServo.write(L_rudderAngle);
        R_rudderServo.write(R_rudderAngle);
        
        // Элероны и закрылки
        updateAilerons(processedData.xAxis2);
        updateFlaps(processedData.button1 ? 512 : (processedData.button2 ? -512 : 0));
    #endif
    
    // 📊 ДИАГНОСТИКА ПОЛОЖЕНИЙ СЕРВОПРИВОДОВ (раз в 2 секунды)
    static unsigned long lastServoDebug = 0;
    if (millis() - lastServoDebug > 2000 && !blheliFirstRun) {
        // Проверяем, были ли изменения в управлении
        static int lastElevator = 0, lastRudder = 0, lastAileron = 0;
        static bool lastFlaps = false;
        
        bool shouldPrint = false;
        
        if (abs(L_elevatorAngle - lastElevator) > 5) {
            lastElevator = L_elevatorAngle;
            shouldPrint = true;
        }
        if (abs(L_rudderAngle - lastRudder) > 5) {
            lastRudder = L_rudderAngle;
            shouldPrint = true;
        }
        if (abs(L_aileronAngle - lastAileron) > 5) {
            lastAileron = L_aileronAngle;
            shouldPrint = true;
        }
        bool currentFlaps = (processedData.button1 || processedData.button2);
        if (currentFlaps != lastFlaps) {
            lastFlaps = currentFlaps;
            shouldPrint = true;
        }
        
        if (shouldPrint) {
            Serial.print("🎮 SERVO Positions: ");
            Serial.print("Elev=");
            Serial.print(L_elevatorAngle);
            Serial.print("°, Rud=");
            Serial.print(L_rudderAngle);
            Serial.print("°, Ail=");
            Serial.print(L_aileronAngle);
            Serial.print("°, Flaps=");
            Serial.print(L_flapsAngle);
            Serial.print("°, MotorArmed=");
            Serial.print(motorArmed ? "YES" : "NO");
            Serial.print(", BLHeliActive=");
            Serial.println(blheliFirstRun ? "NO" : "YES");
        }
        
        lastServoDebug = millis();
    }
    
    // ============================================================================
    // 🔄 ОБНОВЛЕНИЕ СОСТОЯНИЯ ТЕСТОВ (если включены)
    // ============================================================================
    
    if (testsEnabled && !isTesting && !blheliFirstRun) {
        // Проверяем условия для автоматического запуска тестов
        // Например, если все оси в нейтрали и нажата комбинация кнопок
        if (abs(data.yAxis1) < 50 && abs(data.xAxis1) < 50 && 
            abs(data.xAxis2) < 50 && abs(data.yAxis2) < 50 &&
            data.button1 && data.button2) {
            Serial.println("🧪 AUTO-TEST triggered by button combo!");
            simultaneousTestSequence();
        }
    }
}