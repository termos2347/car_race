#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

ServoManager servoManager;
ESPNowManager& espNowManager = ESPNowManager::getInstance();

void onDataReceived(const ControlData& data) {
    servoManager.update(data);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("🎯 START PROGRAM - ADVANCED SERVO CONTROL");
    Serial.println("📋 Configuration Loaded from ServoManager.h");
    
    servoManager.begin();
    espNowManager.begin();
    espNowManager.registerCallback(onDataReceived);
    espNowManager.addPeer();
    
    Serial.println("✅ READY - Waiting for transmitter connection...");
}

void loop() {
    espNowManager.updateConnection();
    delay(100);
}