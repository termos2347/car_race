#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

ServoManager servoManager;
ESPNowManager& espNowManager = ESPNowManager::getInstance();

const uint8_t transmitterMac[] = {0x14, 0x33, 0x5C, 0x37, 0x82, 0x58};

void onDataReceived(const ControlData& data) {
    // ПРОСТО ВЫЗЫВАЕМ update С ДАННЫМИ
    servoManager.update(data);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("🎯 START PROGRAM");
    
    // Инициализация ServoManager
    servoManager.begin();

    // Инициализация ESP-NOW
    espNowManager.begin();
    espNowManager.registerCallback(onDataReceived);
    espNowManager.addPeer(transmitterMac);
    
    Serial.println("✅ READY - ULTRA SIMPLE VERSION");
}

void loop() {
    // АБСОЛЮТНО НИЧЕГО НЕ ДЕЛАЕМ В LOOP
    // Всё обрабатывается в callback
    delay(1000); // Просто чтобы не грузил процессор
}