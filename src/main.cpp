// САМОЛЕТ (приемник) - ПРЯМОЕ СООТВЕТСТВИЕ
#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

ServoManager servoManager;
ESPNowManager& espNowManager = ESPNowManager::getInstance();

const uint8_t transmitterMac[] = {0xEC, 0xE3, 0x34, 0x19, 0x23, 0xD4};

static ControlData currentData;
static unsigned long lastDataTime = 0;

void onDataReceived(const ControlData& data) {
    currentData = data;
    lastDataTime = millis();
    
    // НЕМЕДЛЕННОЕ обновление сервопривода
    servoManager.update(currentData);
    
    digitalWrite(2, HIGH);
    
    // Диагностика (ограниченная частота)
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 100) {
        Serial.printf("🎯 Y1:%-4d → PWM:%-3d\n", data.yAxis1, data.xAxis1);
        lastDebug = millis();
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("✈️ САМОЛЕТ ЗАПУЩЕН (использует ESPNowManager)");
    Serial.println("========================");
    
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    // Инициализация сервопривода
    Serial.println("🎯 Инициализация сервопривода...");
    servoManager.begin();
    
    // Инициализация ESP-NOW через менеджер
    Serial.println("📡 Инициализация ESP-NOW через менеджер...");
    espNowManager.begin();
    espNowManager.registerCallback(onDataReceived);
    
    if (espNowManager.addPeer(transmitterMac)) {
        Serial.println("✅ Пульт добавлен через менеджер");
    } else {
        Serial.println("❌ Ошибка добавления пульта через менеджер");
    }
    
    Serial.println("🚀 Система готова к работе");
    Serial.println("   Y1: -512=0°, 0=90°, 512=180°");
    Serial.println("   X1: двигатель");
    Serial.println("========================");
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Выключаем LED через 50мс после получения данных
    if (currentMillis - lastDataTime > 50) {
        digitalWrite(2, LOW);
    }
    
    // Мигание при отсутствии данных
    if (currentMillis - lastDataTime > 2000) {
        static unsigned long lastBlink = 0;
        if (currentMillis - lastBlink > 500) {
            digitalWrite(2, !digitalRead(2));
            lastBlink = currentMillis;
        }
    }
    
    // Аварийная остановка при потере связи
    if (currentMillis - lastDataTime > 3000) {
        servoManager.emergencyStop();
    }
    
    // МИНИМАЛЬНАЯ задержка для максимальной отзывчивости
    delay(1);
}