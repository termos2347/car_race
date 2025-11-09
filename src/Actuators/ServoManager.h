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

// Режим тестирования (true - безопасное последовательное, false - синхронное)
#define SAFE_TEST_MODE false

// Задержки между тестами (миллисекунды)
#define TEST_DELAY_SHORT  1000
#define TEST_DELAY_LONG   3000

// Включить/выключить тестирование отдельных сервоприводов
#define TEST_ELEVATOR     true
#define TEST_RUDDER       true  
#define TEST_AILERONS     true
#define TEST_FLAPS        true
#define TEST_MOTOR        true


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
    void simultaneousTestSequence();  // ИЗМЕНИЛ НАЗВАНИЕ
    
private:
    // Основные сервоприводы управления полетом
    ServoGroup L_elevatorServo;       // Левый руль высоты
    ServoGroup R_elevatorServo;       // Правый руль высоты
    ServoGroup L_rudderServo;         // Левый руль направления
    ServoGroup R_rudderServo;         // Правый руль направления

    ServoGroup L_aileronServo;        // Левый элерon
    ServoGroup R_aileronServo;        // Правый элерон
    ServoGroup L_flapServo;           // Левый закрылок
    ServoGroup R_flapServo;           // Правый закрылок
    ServoGroup motorServo;            // Двигатель
    
    bool isTesting = false;
    
    // Настройки углов сервоприводов
    // ELEVATOR
    static const int L_ELEVATOR_MIN = 0;
    static const int L_ELEVATOR_MAX = 180;
    static const int L_ELEVATOR_NEUTRAL = 90;

    static const int R_ELEVATOR_MIN = 0;
    static const int R_ELEVATOR_MAX = 180;
    static const int R_ELEVATOR_NEUTRAL = 90;
    
    //RUDDER
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
    
    // Вспомогательные методы
    void updateAilerons(int rollValue);
    void updateAileronsSmooth(int rollValue);
    void updateFlaps(int flapsValue);
    void updateFlapsSmooth(int flapsValue);
    void applyDeadZone(int16_t& axisValue, int deadZone);
    
    // Новый метод для одновременного движения
    void moveAllServos(int L_elevator, int R_elevator, int L_rudder, int R_rudder, 
                  int L_aileron, int R_aileron, int L_flaps, int R_flaps, int motor);
};