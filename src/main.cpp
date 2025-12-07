#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

ServoManager servoManager;
ESPNowManager& espNowManager = ESPNowManager::getInstance();

void onDataReceived(const ControlData& data) {
    servoManager.update(data);
}

void checkSerialCommands() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch(cmd) {
            case 't': // Полный тест
                servoManager.runManualTests();
                break;
                
            case 'c': // Калибровка ESC
                servoManager.calibrateESC();
                break;
                
            case 'm': // Простой тест мотора
                servoManager.escTestSimple();
                break;
                
            case 'd': // Direct motor test - 50% power
                Serial.println("🔧 DIRECT MOTOR TEST - 50% POWER FOR 3 SECONDS");
                servoManager.testMotorDirect();
                break;
                
            case '1': // Тест 10% мощности
                Serial.println("🔧 Setting motor to 10% (1100μs)");
                servoManager.directMotorTest(10);
                break;
                
            case '2': // Тест 25% мощности
                Serial.println("🔧 Setting motor to 25% (1250μs)");
                servoManager.directMotorTest(25);
                break;
                
            case '3': // Тест 50% мощности
                Serial.println("🔧 Setting motor to 50% (1500μs)");
                servoManager.directMotorTest(50);
                break;
                
            case '0': // Стоп
                Serial.println("🔧 STOPPING motor (1000μs)");
                servoManager.directMotorTest(0);
                break;
                
            case 'b': // BLHeli arming sequence
                servoManager.blheliArmingSequence();
                break;
                
            case 's': // Статус
                Serial.println("📊 System status:");
                Serial.print("  ESC armed: ");
                Serial.println(servoManager.isMotorArmed() ? "YES" : "NO");
                Serial.print("  ESP-NOW: ");
                Serial.println(espNowManager.isConnected() ? "CONNECTED" : "DISCONNECTED");
                break;
                
            case 'x': // Экстренная остановка мотора
                servoManager.emergencyStop();
                Serial.println("🛑 EMERGENCY MOTOR STOP");
                break;
                
            case 'h': // Помощь
                Serial.println("📝 Available commands:");
                Serial.println("  t - Full servo tests (with motor)");
                Serial.println("  c - Calibrate ESC");
                Serial.println("  m - Simple motor test");
                Serial.println("  d - Direct motor test (50%, 3s)");
                Serial.println("  0 - Stop motor (0%)");
                Serial.println("  1 - Motor 10%");
                Serial.println("  2 - Motor 25%");
                Serial.println("  3 - Motor 50%");
                Serial.println("  s - System status");
                Serial.println("  x - Emergency motor stop");
                Serial.println("  h - This help");
                break;
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("🎯 FLIGHT CONTROL SYSTEM");
    Serial.println("📡 ESP-NOW RC Controller");
    Serial.println("📝 Send 'h' for available commands");
    
    servoManager.begin();
    espNowManager.begin();
    espNowManager.registerCallback(onDataReceived);
    espNowManager.addPeer();
    
    Serial.println("✅ READY - Waiting for transmitter...");
}

void loop() {
    espNowManager.updateConnection();
    checkSerialCommands();
    delay(50);
}