# Руководство по работе с проектом и добавлению новых сервоприводов

## 🏗️ Архитектура проекта

```
src/
├── main.cpp                          # Точка входа
├── Core/
│   └── Types.h                       # Конфигурация пинов и структуры данных
├── Communication/
│   ├── ESPNowManager.h              # Управление беспроводной связью
│   └── ESPNowManager.cpp
└── Actuators/
    ├── ServoManager.h               # Главный менеджер всех сервоприводов
    ├── ServoManager.cpp
    ├── ServoGroup.h                 # Переиспользуемый компонент сервопривода
    └── ServoGroup.cpp
```

## 🎯 Как добавить новый сервопривод

### Шаг 1: Добавить пин в Types.h

```cpp
// В файле Core/Types.h
struct HardwareConfig {
    // Существующие пины...
    static const uint8_t ELEVATOR_PIN = 12;
    static const uint8_t RUDDER_PIN = 14;
    static const uint8_t AILERON_PIN = 15;
    
    // НОВЫЙ ПИН - например, для закрылков (flaps)
    static const uint8_t FLAPS_PIN = 16;
    
    static const uint8_t MOTOR_PIN = 13;
    static const uint8_t LED_PIN = 2;
};
```

### Шаг 2: Добавить сервопривод в ServoManager.h

```cpp
// В файле Actuators/ServoManager.h
class ServoManager {
private:
    // Существующие сервоприводы...
    ServoGroup elevatorServo;
    ServoGroup rudderServo;
    ServoGroup aileronServo;
    
    // НОВЫЙ сервопривод
    ServoGroup flapsServo;
    
    // Настройки для нового сервопривода
    static const int FLAPS_MIN = 0;        // Минимальный угол
    static const int FLAPS_MAX = 90;       // Максимальный угол  
    static const int FLAPS_NEUTRAL = 0;    // Нейтральное положение
};
```

### Шаг 3: Инициализировать в ServoManager.cpp

```cpp
// В конструкторе ServoManager (в ServoManager.cpp)
ServoManager::ServoManager()
    : elevatorServo(HardwareConfig::ELEVATOR_PIN, ELEVATOR_MIN, ELEVATOR_MAX, ELEVATOR_NEUTRAL, "ELEVATOR"),
      rudderServo(HardwareConfig::RUDDER_PIN, RUDDER_MIN, RUDDER_MAX, RUDDER_NEUTRAL, "RUDDER"),
      aileronServo(HardwareConfig::AILERON_PIN, AILERON_MIN, AILERON_MAX, AILERON_NEUTRAL, "AILERON"),
      // НОВЫЙ сервопривод
      flapsServo(HardwareConfig::FLAPS_PIN, FLAPS_MIN, FLAPS_MAX, FLAPS_NEUTRAL, "FLAPS")
{
}

// В методе begin()
void ServoManager::begin() {
    // Существующая инициализация...
    elevatorServo.begin();
    rudderServo.begin(); 
    aileronServo.begin();
    
    // НОВАЯ инициализация
    flapsServo.begin();
}

// В методе testSequence()
void ServoManager::testSequence() {
    // Существующие тесты...
    elevatorServo.testSequence();
    rudderServo.testSequence();
    aileronServo.testSequence();
    
    // НОВЫЙ тест
    flapsServo.testSequence();
}
```

### Шаг 4: Добавить управление в метод update()

```cpp
// В методе update() в ServoManager.cpp
void ServoManager::update(const ControlData& data) {
    // Существующее управление...
    
    // НОВОЕ управление - например, закрылки по кнопке 1
    if (data.button1) {
        flapsServo.write(FLAPS_MAX);  // Полностью выпустить закрылки
    } else {
        flapsServo.write(FLAPS_MIN);  // Убрать закрылки
    }
    
    // Или аналоговое управление (например, по оси Y второго джойстика)
    // int flapsAngle = map(data.yAxis2, -512, 512, FLAPS_MIN, FLAPS_MAX);
    // flapsServo.write(flapsAngle);
    
    // Обновляем вывод в Serial
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        Serial.print("🎮 Elevator:");
        Serial.print(elevatorAngle);
        Serial.print("° Rudder:");
        Serial.print(rudderAngle); 
        Serial.print("° Aileron:");
        Serial.print(aileronAngle);
        Serial.print("° Flaps:");
        Serial.print(data.button1 ? FLAPS_MAX : FLAPS_MIN);  // НОВЫЙ вывод
        Serial.println("°");
        lastPrint = millis();
    }
}
```

## 🎮 Примеры управления разными сервоприводами

### 1. **Закрылки (Flaps)** - по кнопке
```cpp
// Простое управление - кнопка вкл/выкл
if (data.button1) {
    flapsServo.write(FLAPS_MAX);
} else {
    flapsServo.write(FLAPS_MIN);
}
```

### 2. **Шасси (Landing Gear)** - по кнопке с двумя положениями
```cpp
// 0° - убрано, 90° - выпущено
if (data.button2) {
    landingGearServo.write(90);
} else {
    landingGearServo.write(0);
}
```

### 3. **Триммеры (Trimmers)** - по дополнительным осям
```cpp
// Плавное управление триммером руля высоты
int elevatorTrim = map(data.yAxis2, -512, 512, TRIM_MIN, TRIM_MAX);
elevatorTrimServo.write(elevatorTrim);
```

### 4. **Сервопривод с безопасной зоной**
```cpp
// Создание сервопривода с ограниченным диапазоном для безопасности
ServoGroup safeServo(HardwareConfig::SOME_PIN, 30, 150, 90, "SAFE_SERVO");
```

## 🔧 Быстрый шаблон для добавления нового сервопривода

### 1. **В Types.h:**
```cpp
static const uint8_t NEW_SERVO_PIN = 16;
```

### 2. **В ServoManager.h:**
```cpp
ServoGroup newServo;
static const int NEW_SERVO_MIN = 0;
static const int NEW_SERVO_MAX = 180; 
static const int NEW_SERVO_NEUTRAL = 90;
```

### 3. **В ServoManager.cpp:**
```cpp
// В конструкторе:
newServo(HardwareConfig::NEW_SERVO_PIN, NEW_SERVO_MIN, NEW_SERVO_MAX, NEW_SERVO_NEUTRAL, "NEW_SERVO")

// В begin():
newServo.begin();

// В testSequence():
newServo.testSequence();

// В update():
int newServoAngle = map(data.axis, -512, 512, NEW_SERVO_MIN, NEW_SERVO_MAX);
newServo.write(newServoAngle);
```

## 📊 Пример вывода после добавления закрылков

```
🚀 START ServoManager
🚀 INIT ELEVATOR Servo
🚀 INIT RUDDER Servo
🚀 INIT AILERON Servo  
🚀 INIT FLAPS Servo        <-- НОВЫЙ сервопривод
🧪 START Complete Servo Test Sequence
🧪 TEST ELEVATOR
🧪 TEST RUDDER
🧪 TEST AILERON
🧪 TEST FLAPS              <-- НОВЫЙ тест
🎮 Elevator:84° Rudder:90° Aileron:45° Flaps:0°    <-- НОВЫЙ вывод
```
