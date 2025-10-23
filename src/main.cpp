// САМОЛЕТ (приемник) - ПРЯМОЕ СООТВЕТСТВИЕ
#include <esp_now.h>
#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"

ServoManager servoManager;
const uint8_t transmitterMac[] = {0xEC, 0xE3, 0x34, 0x19, 0x23, 0xD4};

static ControlData currentData;
static unsigned long lastDataTime = 0;

// Прямая функция преобразования джойстик → сервопривод
int joystickToServo(int joystickValue) {
    // Y1: -512 → 0°, 0 → 90°, 512 → 180°
    return map(joystickValue, -512, 512, 0, 180);
}

void onDataReceived(const ControlData& data) {
    currentData = data;
    lastDataTime = millis();
    
    // НЕМЕДЛЕННОЕ обновление сервопривода
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
    delay(500);
    Serial.println("✈️ САМОЛЕТ ЗАПУЩЕН (прямое соответствие джойстик→серво)");
    Serial.println("========================");
    
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    // Инициализация сервопривода
    Serial.println("🎯 Инициализация сервопривода...");
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