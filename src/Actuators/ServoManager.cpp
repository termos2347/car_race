#include "ServoManager.h"
#include <Arduino.h>

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
    Serial.println("📌 Power Safety Configuration:");
    Serial.print("   - Smooth Movement: ");
    Serial.println(SMOOTH_SERVO_MOVEMENT ? "ENABLED" : "DISABLED");
    Serial.print("   - Safe Test Mode: ");
    Serial.println(SAFE_TEST_MODE ? "ENABLED" : "DISABLED");
    Serial.println("   - Test Type: SIMULTANEOUS (All servos together)");
    
    delay(3000);

    // Инициализация сервоприводов
    Serial.println("🎯 Initializing servos...");
    elevatorServo.begin();
    rudderServo.begin();
    leftAileronServo.begin();
    rightAileronServo.begin();
    flapsServo.begin();
    motorServo.begin();
    
    #if TEST_AUX1
        aux1Servo.begin();
    #endif
    #if TEST_AUX2
        aux2Servo.begin();
    #endif  
    #if TEST_AUX3
        aux3Servo.begin();
    #endif
    
    // Запуск тестов в зависимости от настроек
    #if SAFE_TEST_MODE
        safeTestSequence();
    #else
        simultaneousTestSequence();
    #endif
    
    Serial.println("✅ ALL Servos INIT OK");
}

void ServoManager::moveAllServos(int elevator, int rudder, int leftAileron, int rightAileron, 
                                int flaps, int motor, int aux1, int aux2, int aux3) {
    // ВСЕ сервоприводы двигаются ОДНОВРЕМЕННО
    elevatorServo.write(elevator);
    rudderServo.write(rudder);
    leftAileronServo.write(leftAileron);
    rightAileronServo.write(rightAileron);
    flapsServo.write(flaps);
    motorServo.write(motor);
    
    #if TEST_AUX1
        aux1Servo.write(aux1);
    #endif
    #if TEST_AUX2
        aux2Servo.write(aux2);
    #endif
    #if TEST_AUX3
        aux3Servo.write(aux3);
    #endif
}

void ServoManager::simultaneousTestSequence() {
    Serial.println("🧪 SIMULTANEOUS Servo Test Sequence");
    Serial.println("🎯 ALL servos moving TOGETHER at the same time!");
    isTesting = true;
    
    // ТЕСТ 1: Все в нейтральное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 1: ALL SERVOS → NEUTRAL");
    moveAllServos(ELEVATOR_NEUTRAL, RUDDER_NEUTRAL, AILERON_NEUTRAL, AILERON_NEUTRAL,
                  FLAPS_NEUTRAL, MOTOR_NEUTRAL, AUX1_NEUTRAL, AUX2_NEUTRAL, AUX3_NEUTRAL);
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 2: Все в минимальное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 2: ALL SERVOS → MINIMUM");
    moveAllServos(ELEVATOR_MIN, RUDDER_MIN, AILERON_MIN, AILERON_MIN,
                  FLAPS_MIN, MOTOR_MIN, AUX1_MIN, AUX2_MIN, AUX3_MIN);
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 3: Все в максимальное положение ОДНОВРЕМЕННО
    Serial.println("🎯 TEST 3: ALL SERVOS → MAXIMUM");
    moveAllServos(ELEVATOR_MAX, RUDDER_MAX, AILERON_MAX, AILERON_MAX,
                  FLAPS_MAX, MOTOR_MAX, AUX1_MAX, AUX2_MAX, AUX3_MAX);
    delay(TEST_DELAY_LONG);
    
    // ТЕСТ 4: Элероны в противофазе + другие нейтральные
    Serial.println("🎯 TEST 4: AILERONS ANTI-PHASE + OTHERS NEUTRAL");
    moveAllServos(ELEVATOR_NEUTRAL, RUDDER_NEUTRAL, AILERON_MAX, AILERON_MIN,
                  FLAPS_NEUTRAL, MOTOR_NEUTRAL, AUX1_NEUTRAL, AUX2_NEUTRAL, AUX3_NEUTRAL);
    delay(TEST_DELAY_SHORT);
    
    // ТЕСТ 5: Противоположная фаза элеронов
    Serial.println("🎯 TEST 5: AILERONS REVERSE ANTI-PHASE");
    moveAllServos(ELEVATOR_NEUTRAL, RUDDER_NEUTRAL, AILERON_MIN, AILERON_MAX,
                  FLAPS_NEUTRAL, MOTOR_NEUTRAL, AUX1_NEUTRAL, AUX2_NEUTRAL, AUX3_NEUTRAL);
    delay(TEST_DELAY_SHORT);
    
    // ТЕСТ 6: Закрылки выпущены + мотор на полную
    Serial.println("🎯 TEST 6: FLAPS DEPLOYED + MOTOR FULL");
    moveAllServos(ELEVATOR_NEUTRAL, RUDDER_NEUTRAL, AILERON_NEUTRAL, AILERON_NEUTRAL,
                  FLAPS_MAX, MOTOR_MAX, AUX1_NEUTRAL, AUX2_NEUTRAL, AUX3_NEUTRAL);
    delay(TEST_DELAY_SHORT);
    
    // ФИНАЛ: Все обратно в нейтральное
    Serial.println("🎯 FINAL: ALL SERVOS → NEUTRAL");
    moveAllServos(ELEVATOR_NEUTRAL, RUDDER_NEUTRAL, AILERON_NEUTRAL, AILERON_NEUTRAL,
                  FLAPS_NEUTRAL, MOTOR_NEUTRAL, AUX1_NEUTRAL, AUX2_NEUTRAL, AUX3_NEUTRAL);
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
        elevatorServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_RUDDER
        Serial.println("🎯 Testing RUDDER");
        rudderServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_AILERONS
        Serial.println("🎯 Testing AILERONS");
        leftAileronServo.testSequence();
        delay(TEST_DELAY_SHORT);
        rightAileronServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_FLAPS
        Serial.println("🎯 Testing FLAPS");
        flapsServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_MOTOR
        Serial.println("🎯 Testing MOTOR");
        motorServo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_AUX1
        Serial.println("🎯 Testing AUX1");
        aux1Servo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_AUX2
        Serial.println("🎯 Testing AUX2");
        aux2Servo.testSequence();
        delay(TEST_DELAY_LONG);
    #endif
    
    #if TEST_AUX3
        Serial.println("🎯 Testing AUX3");
        aux3Servo.testSequence();
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
    int leftAileronAngle = map(rollValue, -512, 512, AILERON_MAX, AILERON_MIN);
    int rightAileronAngle = map(rollValue, -512, 512, AILERON_MIN, AILERON_MAX);
    
    leftAileronServo.write(leftAileronAngle);
    rightAileronServo.write(rightAileronAngle);
}

void ServoManager::updateAileronsSmooth(int rollValue) {
    int leftAileronAngle = map(rollValue, -512, 512, AILERON_MAX, AILERON_MIN);
    int rightAileronAngle = map(rollValue, -512, 512, AILERON_MIN, AILERON_MAX);
    
    leftAileronServo.writeSmooth(leftAileronAngle, SERVO_SPEED_FAST);
    rightAileronServo.writeSmooth(rightAileronAngle, SERVO_SPEED_FAST);
}

void ServoManager::updateFlaps(int flapsValue) {
    int flapsAngle;
    if (flapsValue < -300) {
        flapsAngle = FLAPS_MIN;
    } else if (flapsValue > 300) {
        flapsAngle = FLAPS_MAX;
    } else {
        flapsAngle = FLAPS_NEUTRAL;
    }
    
    flapsServo.write(flapsAngle);
}

void ServoManager::updateFlapsSmooth(int flapsValue) {
    int flapsAngle;
    if (flapsValue < -300) {
        flapsAngle = FLAPS_MIN;
    } else if (flapsValue > 300) {
        flapsAngle = FLAPS_MAX;
    } else {
        flapsAngle = FLAPS_NEUTRAL;
    }
    
    flapsServo.writeSmooth(flapsAngle, SERVO_SPEED_SLOW);
}

void ServoManager::updateAuxServos(const ControlData& data) {
    #if TEST_AUX1
        if (data.button1) {
            aux1Servo.write(AUX1_MAX);
        } else {
            aux1Servo.write(AUX1_MIN);
        }
    #endif
    
    #if TEST_AUX2
        if (data.button2) {
            aux2Servo.write(AUX2_MAX);
        } else {
            aux2Servo.write(AUX2_MIN);
        }
    #endif
    
    #if TEST_AUX3
        int aux3Angle = map(data.xAxis2, -512, 512, AUX3_MIN, AUX3_MAX);
        aux3Servo.write(aux3Angle);
    #endif
}

void ServoManager::updateAuxServosSmooth(const ControlData& data) {
    #if TEST_AUX1
        if (data.button1) {
            aux1Servo.writeSmooth(AUX1_MAX, SERVO_SPEED_SLOW);
        } else {
            aux1Servo.writeSmooth(AUX1_MIN, SERVO_SPEED_SLOW);
        }
    #endif
    
    #if TEST_AUX2
        if (data.button2) {
            aux2Servo.writeSmooth(AUX2_MAX, SERVO_SPEED_SLOW);
        } else {
            aux2Servo.writeSmooth(AUX2_MIN, SERVO_SPEED_SLOW);
        }
    #endif
    
    #if TEST_AUX3
        int aux3Angle = map(data.xAxis2, -512, 512, AUX3_MIN, AUX3_MAX);
        aux3Servo.writeSmooth(aux3Angle, SERVO_SPEED_MEDIUM);
    #endif
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
    int elevatorAngle = map(processedData.yAxis1, -512, 512, ELEVATOR_MIN, ELEVATOR_MAX);
    int rudderAngle = map(processedData.xAxis1, -512, 512, RUDDER_MIN, RUDDER_MAX);
    int motorSpeed = map(processedData.yAxis2, -512, 512, MOTOR_MIN, MOTOR_MAX);
    
    #if SMOOTH_SERVO_MOVEMENT
        elevatorServo.writeSmooth(elevatorAngle, SERVO_SPEED_MEDIUM);
        rudderServo.writeSmooth(rudderAngle, SERVO_SPEED_MEDIUM);
        motorServo.write(motorSpeed); // Мотору не нужна плавность
        updateAileronsSmooth(processedData.xAxis2);
        updateFlapsSmooth(processedData.yAxis2);
        updateAuxServosSmooth(processedData);
    #else
        elevatorServo.write(elevatorAngle);
        rudderServo.write(rudderAngle);
        motorServo.write(motorSpeed);
        updateAilerons(processedData.xAxis2);
        updateFlaps(processedData.yAxis2);
        updateAuxServos(processedData);
    #endif
    
    // Вывод отладочной информации
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        int leftAileronAngle = map(processedData.xAxis2, -512, 512, AILERON_MAX, AILERON_MIN);
        int rightAileronAngle = map(processedData.xAxis2, -512, 512, AILERON_MIN, AILERON_MAX);
        
        Serial.print("🎮 Elev:");
        Serial.print(elevatorAngle);
        Serial.print("° Rud:");
        Serial.print(rudderAngle);
        Serial.print("° Ail L:");
        Serial.print(leftAileronAngle);
        Serial.print("°/R:");
        Serial.print(rightAileronAngle);
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