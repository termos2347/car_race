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
    
    // Запуск тестов в зависимости от настроек
    #if SAFE_TEST_MODE
        safeTestSequence();
    #else
        simultaneousTestSequence();
    #endif
    
    Serial.println("✅ ALL Servos INIT OK");
}

void ServoManager::moveAllServos(int L_elevator, int R_elevator, int L_rudder, int R_rudder, int L_aileron, int R_aileron, 
                                int L_flaps, int R_flaps, int motor) {
    // ВСЕ сервоприводы двигаются ОДНОВРЕМЕННО
    L_elevatorServo.write(L_elevator);
    R_elevatorServo.write(R_elevator);
    L_rudderServo.write(L_rudder);
    R_rudderServo.write(R_rudder);
    L_aileronServo.write(L_aileron);
    R_aileronServo.write(R_aileron);
    L_flapServo.write(L_flaps);
    R_flapServo.write(R_flaps);
    motorServo.write(motor);
}

void ServoManager::simultaneousTestSequence() {
    Serial.println("🧪 SIMULTANEOUS Servo Test Sequence");
    Serial.println("🎯 ALL servos moving TOGETHER at the same time!");
    isTesting = true;
    
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
                  MOTOR_MIN);
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 3: Все в максимальное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 3: ALL SERVOS → MAXIMUM");
    moveAllServos(L_ELEVATOR_MAX, R_ELEVATOR_MAX, 
                  L_RUDDER_MAX, R_RUDDER_MAX,
                  L_AILERON_MAX, R_AILERON_MAX,
                  L_FLAPS_MAX, R_FLAPS_MAX, 
                  MOTOR_MAX);
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ: Элероны в противофазе + другие нейтральные
    // Serial.println("🎯 TEST 4: AILERONS ANTI-PHASE + OTHERS NEUTRAL");
    // moveAllServos(ELEVATOR_NEUTRAL, RUDDER_NEUTRAL, AILERON_MAX, AILERON_MIN,
    //              FLAPS_NEUTRAL, MOTOR_NEUTRAL, AUX1_NEUTRAL, AUX2_NEUTRAL, AUX3_NEUTRAL);
    // delay(TEST_DELAY_SHORT);
    
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
        Serial.println("🎯 Testing MOTOR");
        motorServo.testSequence();
        delay(TEST_DELAY_LONG);
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

void ServoManager::update(const ControlData& data) {
    if (isTesting) {
        return;
    }
    
    // Применяем мертвые зоны к осям
    ControlData processedData = data;
    applyDeadZone(processedData.xAxis1, DEADZONE_XAXIS1);
    applyDeadZone(processedData.yAxis1, DEADZONE_YAXIS1);
    applyDeadZone(processedData.xAxis2, DEADZONE_XAXIS2);
    applyDeadZone(processedData.yAxis2, DEADZONE_YAXIS2);
    
    // Основные органы управления
    int L_elevatorAngle = map(processedData.yAxis1, -512, 512, L_ELEVATOR_MIN, L_ELEVATOR_MAX);
    int R_elevatorAngle = map(processedData.yAxis1, -512, 512, R_ELEVATOR_MIN, R_ELEVATOR_MAX);
    int L_rudderAngle = map(processedData.xAxis1, -512, 512, L_RUDDER_MIN, L_RUDDER_MAX);
    int R_rudderAngle = map(processedData.xAxis1, -512, 512, R_RUDDER_MIN, R_RUDDER_MAX);
    int motorSpeed = map(processedData.yAxis2, -512, 512, MOTOR_MIN, MOTOR_MAX);
    
    #if SMOOTH_SERVO_MOVEMENT
        L_elevatorServo.writeSmooth(L_elevatorAngle, SERVO_SPEED_MEDIUM);
        R_elevatorServo.writeSmooth(R_elevatorAngle, SERVO_SPEED_MEDIUM);
        L_rudderServo.writeSmooth(L_rudderAngle, SERVO_SPEED_MEDIUM);
        R_rudderServo.writeSmooth(R_rudderAngle, SERVO_SPEED_MEDIUM);
        motorServo.write(motorSpeed); // Мотору не нужна плавность
        updateAileronsSmooth(processedData.xAxis2);
        updateFlapsSmooth(processedData.yAxis2);
    #else
        L_elevatorServo.write(L_elevatorAngle);
        R_elevatorServo.write(R_elevatorAngle);
        L_rudderServo.write(L_rudderAngle);
        R_rudderServo.write(R_rudderAngle);
        motorServo.write(motorSpeed);
        updateAilerons(processedData.xAxis2);
        updateFlaps(processedData.yAxis2);
    #endif
    
    // Вывод отладочной информации
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        int L_aileronAngle = map(processedData.xAxis2, -512, 512, L_AILERON_MAX, L_AILERON_MIN);
        int R_aileronAngle = map(processedData.xAxis2, -512, 512, R_AILERON_MIN, R_AILERON_MAX);
        
        Serial.print("🎮 Elev L:");
        Serial.print(L_elevatorAngle);
        Serial.print("/R");
        Serial.print(R_elevatorAngle);
        Serial.print("° Rud L:");
        Serial.print(L_rudderAngle);
        Serial.print("/R");
        Serial.print(L_rudderAngle);
        Serial.print("° Ail L:");
        Serial.print(L_aileronAngle);
        Serial.print("°/R:");
        Serial.print(R_aileronAngle);
        Serial.print("° Flaps:");
        
        // Определяем статус закрылков
        const char* flapsStatus = "MID";
        if (processedData.yAxis2 < -300) {
            flapsStatus = "UP";
        } else if (processedData.yAxis2 > 300) {
            flapsStatus = "DOWN";
        }
        Serial.print(flapsStatus);
        
        Serial.print(" Motor:");
        Serial.print(motorSpeed);
        Serial.print(" B1:");
        Serial.print(processedData.button1 ? "ON" : "OFF");
        Serial.print(" B2:");
        Serial.print(processedData.button2 ? "ON" : "OFF");
        Serial.println();
        
        lastPrint = millis();
    }
}