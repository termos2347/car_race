#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

ServoManager servoManager;
ESPNowManager& espNowManager = ESPNowManager::getInstance();

void onDataReceived(const ControlData& data) {
    // ПРОСТО ВЫЗЫВАЕМ update С ДАННЫМИ
    servoManager.update(data);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("🎯 START PROGRAM - CLEAN ARCHITECTURE");
    
    // Светодиод выключен во время тестирования
    Serial.println("💡 LED OFF during testing");
    
    // Инициализация ServoManager (включает тест сервопривода)
    servoManager.begin();

    // Инициализация ESP-NOW
    espNowManager.begin();
    espNowManager.registerCallback(onDataReceived);
    espNowManager.addPeer(); // Теперь без параметров
    
    Serial.println("✅ READY - Waiting for transmitter connection...");
    Serial.println("💡 LED will turn ON when connection established");
}

void loop() {
    // Обновляем состояние связи и индикацию
    espNowManager.updateConnection();
    
    delay(100); // Небольшая задержка для стабильности
}