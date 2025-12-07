#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

ServoManager servoManager;
ESPNowManager& espNowManager = ESPNowManager::getInstance();

void onDataReceived(const ControlData& data) {
    servoManager.update(data);
}

// НОВАЯ ФУНКЦИЯ для обработки Serial команд
void checkSerialCommands() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch(cmd) {
            case 't': // Запуск тестов
                servoManager.runManualTests();
                break;
                
            case 'm': // Быстрая проверка мотора
                Serial.println("🔧 Quick motor check");
                // Можно добавить безопасную проверку
                break;
                
            case 's': // Статус системы
                Serial.println("📊 System status:");
                Serial.print("  ESP-NOW connected: ");
                Serial.println(espNowManager.isConnected() ? "YES" : "NO");
                break;
                
            case 'h': // Помощь
                Serial.println("📝 Available commands:");
                Serial.println("  t - Run full servo tests");
                Serial.println("  m - Quick motor check");
                Serial.println("  s - System status");
                Serial.println("  h - This help message");
                break;
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Уменьшили с 2000 мс
    
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
    checkSerialCommands();         // Проверяем команды
    delay(50);                     // Уменьшили с 100 мс
}