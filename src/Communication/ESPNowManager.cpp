#include "ESPNowManager.h"
#include <Arduino.h>

// Статическая переменная для доступа к экземпляру из статической функции
static ESPNowManager* espNowInstance = nullptr;

void ESPNowManager::begin() {
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ Ошибка инициализации ESP-NOW");
        return;
    }
    
    // Сохраняем указатель на экземпляр ДО регистрации callback
    espNowInstance = this;
    
    esp_now_register_recv_cb(onDataReceived);
    
    // Вывод MAC адреса для спаривания
    Serial.print("📡 MAC приемника: ");
    Serial.println(WiFi.macAddress());
    
    Serial.println("✅ ESP-NOW приемник готов");
}

void ESPNowManager::registerCallback(DataReceivedCallback callback) {
    dataCallback = callback;
}

bool ESPNowManager::addPeer(const uint8_t* macAddress) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        Serial.println("✅ Peer добавлен через ESPNowManager");
        return true;
    } else {
        Serial.println("❌ Ошибка добавления peer через ESPNowManager");
        return false;
    }
}

void ESPNowManager::onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
    if (len != sizeof(ControlData)) {
        Serial.printf("❌ Неверный размер пакета: %d (ожидалось %d)\n", len, sizeof(ControlData));
        return;
    }
    
    ControlData receivedData;
    memcpy(&receivedData, data, sizeof(receivedData));
    
    // Валидация CRC
    uint16_t calculatedCRC = 0;
    const uint8_t* bytes = (const uint8_t*)&receivedData;
    for(size_t i = 0; i < sizeof(ControlData) - sizeof(uint16_t); i++) {
        calculatedCRC += bytes[i];
    }
    
    if (calculatedCRC != receivedData.crc) {
        Serial.println("❌ Ошибка CRC");
        return;
    }
    
    // Вызов callback функции через экземпляр
    if (espNowInstance != nullptr && espNowInstance->dataCallback != nullptr) {
        espNowInstance->dataCallback(receivedData);
    }
    
    // Редкая диагностика для отладки связи
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 5000) {
        Serial.printf("📥 ESP-NOW: связь стабильна (последний CRC: %04X)\n", receivedData.crc);
        lastPrint = millis();
    }
}