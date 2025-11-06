#pragma once
#include <ESP32Servo.h>
#include "Core/Types.h"
#include "ServoGroup.h"

// ============================================================================
// НАСТРОЙКИ БЕЗОПАСНОСТИ
// ============================================================================

// Режим работы сервоприводов (true - плавное движение, false - прямое)
#define SMOOTH_SERVO_MOVEMENT true

// Скорости движения сервоприводов (время полного хода 0-180° в миллисекундах)
#define SERVO_SPEED_FAST     100    // Элероны, рули - быстро
#define SERVO_SPEED_MEDIUM   200    // Основные управления  
#define SERVO_SPEED_SLOW     500    // Закрылки, AUX - медленно
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
#define TEST_AUX1         true  // Временно отключено для экономии питания
#define TEST_AUX2         true
#define TEST_AUX3         true

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
    ServoGroup elevatorServo;       // Руль высоты
    ServoGroup rudderServo;         // Руль направления
    ServoGroup leftAileronServo;    // Левый элерон
    ServoGroup rightAileronServo;   // Правый элерон
    
    // Дополнительные сервоприводы
    ServoGroup motorServo;          // Двигатель
    ServoGroup flapsServo;          // Закрылки
    ServoGroup aux1Servo;           // Дополнительный сервопривод 1
    ServoGroup aux2Servo;           // Дополнительный сервопривод 2
    ServoGroup aux3Servo;           // НОВЫЙ сервопривод
    
    bool isTesting = false;
    
    // Настройки углов сервоприводов
    static const int ELEVATOR_MIN = 0;
    static const int ELEVATOR_MAX = 180;
    static const int ELEVATOR_NEUTRAL = 90;
    
    static const int RUDDER_MIN = 0;
    static const int RUDDER_MAX = 180;
    static const int RUDDER_NEUTRAL = 90;
    
    static const int AILERON_MIN = 0;
    static const int AILERON_MAX = 180;
    static const int AILERON_NEUTRAL = 90;
    
    static const int FLAPS_MIN = 0;
    static const int FLAPS_MAX = 180;
    static const int FLAPS_NEUTRAL = 90;
    
    static const int AUX1_MIN = 0;
    static const int AUX1_MAX = 180;
    static const int AUX1_NEUTRAL = 90;
    
    static const int AUX2_MIN = 0;
    static const int AUX2_MAX = 180;
    static const int AUX2_NEUTRAL = 90;

    static const int AUX3_MIN = 0;
    static const int AUX3_MAX = 180;
    static const int AUX3_NEUTRAL = 90;

    static const int MOTOR_MIN = 0;
    static const int MOTOR_MAX = 180;
    static const int MOTOR_NEUTRAL = 0;
    
    // Вспомогательные методы
    void updateAilerons(int rollValue);
    void updateAileronsSmooth(int rollValue);
    void updateFlaps(int flapsValue);
    void updateFlapsSmooth(int flapsValue);
    void updateAuxServos(const ControlData& data);
    void updateAuxServosSmooth(const ControlData& data);
    void applyDeadZone(int16_t& axisValue, int deadZone);
    
    // Новый метод для одновременного движения
    void moveAllServos(int elevator, int rudder, int leftAileron, int rightAileron, 
                      int flaps, int motor, int aux1, int aux2, int aux3);
};