#pragma once
#include <ESP32Servo.h>
#include "Core/Types.h"
#include "ServoGroup.h"

// ============================================================================
// НАСТРОЙКИ БЕЗОПАСНОСТИ
// ============================================================================

// Режим работы сервоприводов (true - плавное движение, false - прямое)
#define SMOOTH_SERVO_MOVEMENT false

// Скорости движения сервоприводов (время полного хода 0-180° в миллисекундах)
#define SERVO_SPEED_FAST     100    // Элероны, рули - быстро
#define SERVO_SPEED_MEDIUM   200    // Основные управления  
#define SERVO_SPEED_SLOW     500    // Закрылки - медленно
#define SERVO_SPEED_TEST    1000    // Тестирование - очень медленно

// ============================================================================
// НАСТРОЙКИ ТЕСТИРОВАНИЯ  
// ============================================================================

// Задержки между тестами (миллисекунды)
#define TEST_DELAY_SHORT  1000
#define TEST_DELAY_LONG   3000

// ============================================================================
// НАСТРОЙКИ УПРАВЛЕНИЯ
// ============================================================================

// Мертвые зоны для осей джойстика (избежание дрожания)
#define DEADZONE_XAXIS1 20
#define DEADZONE_YAXIS1 20  
#define DEADZONE_XAXIS2 20
#define DEADZONE_YAXIS2 20

class ServoManager {
public:
    ServoManager();
    void begin();
    void update(const ControlData& data);

    void testSequence();
    void safeTestSequence();
    void simultaneousTestSequence();

    void runManualTests(); // Новый метод для ручного запуска
    void safeStartSequence();

    // Методы для управления ESC
    void calibrateESC();
    void escTestSimple();
    void writeMicroseconds(int us);  // ← ДОБАВЬТЕ ЭТУ СТРОЧКУ
    void blheliArmingSequence();
    
    // Геттеры
    bool isMotorArmed() const { return motorArmed; }
    bool getIsTesting() const { return isTesting; }
    
    // Управление тестами
    void enableTests() { testsEnabled = true; }
    void disableTests() { testsEnabled = false; }
    
    // Экстренная остановка двигателя
    void emergencyStop() { 
    motorServo.writeMicroseconds(1000);  // ← Используем микросекунды
    motorArmed = false; 
    }
    
    // НОВЫЕ ПУБЛИЧНЫЕ МЕТОДЫ ДЛЯ ТЕСТИРОВАНИЯ
    void testMotorDirect(); // Принудительный тест двигателя
    void directMotorTest(int powerPercent);
    
private:
    // Основные сервоприводы управления полетом
    ServoGroup L_elevatorServo;
    ServoGroup R_elevatorServo;
    ServoGroup L_rudderServo;
    ServoGroup R_rudderServo;
    ServoGroup L_aileronServo;
    ServoGroup R_aileronServo;
    ServoGroup L_flapServo;
    ServoGroup R_flapServo;
    ServoGroup motorServo;
    
    bool isTesting = false;
    bool motorArmed = false;
    bool firstMotorUpdate = true;
    bool testsEnabled = false;  // Флаг для включения тестов

    bool blheliFirstRun = true;
    unsigned long blheliActivationStart = 0;
    int blheliActivationStep = 0;
    
    // Настройки углов сервоприводов
    // ELEVATOR
    static const int L_ELEVATOR_MIN = 0;
    static const int L_ELEVATOR_MAX = 180;
    static const int L_ELEVATOR_NEUTRAL = 90;

    static const int R_ELEVATOR_MIN = 0;
    static const int R_ELEVATOR_MAX = 180;
    static const int R_ELEVATOR_NEUTRAL = 90;
    
    // RUDDER
    static const int L_RUDDER_MIN = 0;
    static const int L_RUDDER_MAX = 180;
    static const int L_RUDDER_NEUTRAL = 90;
    
    static const int R_RUDDER_MIN = 0;
    static const int R_RUDDER_MAX = 180;
    static const int R_RUDDER_NEUTRAL = 90;
    
    // AILERON
    static const int L_AILERON_MIN = 0;
    static const int L_AILERON_MAX = 180;
    static const int L_AILERON_NEUTRAL = 90;

    static const int R_AILERON_MIN = 0;
    static const int R_AILERON_MAX = 180;
    static const int R_AILERON_NEUTRAL = 90;
    
    // FLAPS
    static const int L_FLAPS_MIN = 0;
    static const int L_FLAPS_MAX = 180;
    static const int L_FLAPS_NEUTRAL = 90;
    
    static const int R_FLAPS_MIN = 0;
    static const int R_FLAPS_MAX = 180;
    static const int R_FLAPS_NEUTRAL = 90;

    // MOTOR
    static const int MOTOR_MIN = 0;
    static const int MOTOR_MAX = 180;
    static const int MOTOR_NEUTRAL = 0;
    static const int MOTOR_TEST_MAX = 60;  // Максимальное значение для тестов (безопасно)

    // Настройки импульсов
    static const int SERVO_MIN_PULSE = 500;
    static const int SERVO_MAX_PULSE = 2400;
    static const int MOTOR_MIN_PULSE = 1000;
    static const int MOTOR_MAX_PULSE = 2000;
    
    // Вспомогательные методы
    void updateAilerons(int rollValue);
    void updateAileronsSmooth(int rollValue);
    void updateFlaps(int flapsValue);
    void updateFlapsSmooth(int flapsValue);
    void applyDeadZone(int16_t& axisValue, int deadZone);
    void safeMotorStart();
    void testMotorSequence();
    void moveAllServos(int L_elevator, int R_elevator, int L_rudder, int R_rudder,
                       int L_aileron, int R_aileron, int L_flaps, int R_flaps, int motor);
};