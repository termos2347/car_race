// САМОЛЕТ (приемник) - ПРОСТАЯ ВЕРСИЯ ДЛЯ SG90
#include <esp_now.h>
#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"

ServoManager servoManager;
const uint8_t transmitterMac[] = {0xEC, 0xE3, 0x34, 0x19, 0x23, 0xD4};

static ControlData currentData;
static unsigned long lastDataTime = 0;

void onDataReceived(const ControlData& data) {
    currentData = data;
    lastDataTime = millis();
    
    // СРАЗУ обновляем сервопривод
    servoManager.update(currentData);
    
    digitalWrite(2, HIGH);
}

bool addPeer(const uint8_t* macAddress) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    return esp_now_add_peer(&peerInfo) == ESP_OK;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("✈️ САМОЛЕТ ЗАПУЩЕН (SG90 с прямым управлением)");
    Serial.println("========================");
    
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    // Инициализация сервопривода SG90
    Serial.println("🎯 Инициализация сервопривода SG90...");
    servoManager.begin();
    
    // Инициализация ESP-NOW
    Serial.println("📡 Инициализация ESP-NOW...");
    
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ Ошибка инициализации ESP-NOW");
        while(1) delay(1000);
    }
    
    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *incomingData, int len) {
        if (len == sizeof(ControlData)) {
            ControlData data;
            memcpy(&data, incomingData, sizeof(ControlData));
            
            // Простая проверка CRC
            uint16_t calculatedCRC = 0;
            const uint8_t* bytes = (const uint8_t*)&data;
            for(size_t i = 0; i < sizeof(ControlData) - sizeof(uint16_t); i++) {
                calculatedCRC += bytes[i];
            }
            
            if (calculatedCRC == data.crc) {
                onDataReceived(data);
            }
        }
    });
    
    if (addPeer(transmitterMac)) {
        Serial.println("✅ Пульт добавлен");
    } else {
        Serial.println("❌ Ошибка добавления пульта");
    }
    
    Serial.println("🚀 Система готова к работе");
    Serial.println("   Y1 - сервопривод (-512=0°, 512=180°)");
    Serial.println("   X1 - двигатель");
    Serial.println("========================");
    
    // ЗАПУСК ДИАГНОСТИЧЕСКОГО ТЕСТА ДЖОЙСТИКА
    Serial.println("🎮 ДИАГНОСТИКА: Подвигайте джойстик по оси Y...");
}

// Временная функция для диагностики джойстика
void debugJoystickValues(const ControlData& data) {
    static unsigned long lastDebug = 0;
    static int lastY1 = 0;
    
    if (millis() - lastDebug > 200 && abs(data.yAxis1 - lastY1) > 10) {
        int angle = map(data.yAxis1, -512, 512, 0, 180);
        Serial.printf("🔍 ДЖОЙСТИК: Y1=%-4d → угол=%-3d°\n", data.yAxis1, angle);
        lastY1 = data.yAxis1;
        lastDebug = millis();
    }
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Выключаем LED через 100мс после получения данных
    if (currentMillis - lastDataTime > 100) {
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
    
    // Аварийная остановка
    if (currentMillis - lastDataTime > 3000) {
        servoManager.emergencyStop();
    }
    
    delay(20);
}