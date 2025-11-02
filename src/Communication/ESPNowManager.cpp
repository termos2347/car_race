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
    
    // Инициализация пина светодиода
    pinMode(HardwareConfig::LED_PIN, OUTPUT);
    digitalWrite(HardwareConfig::LED_PIN, LOW); // Изначально выключен
    
    // Вывод MAC адреса для спаривания
    Serial.print("📡 MAC приемника: ");
    Serial.println(WiFi.macAddress());
    
    Serial.println("✅ ESP-NOW инициализирован");
}

void ESPNowManager::registerCallback(DataReceivedCallback callback) {
    dataCallback = callback;
    Serial.println("✅ Callback зарегистрирован в ESPNowManager");
}

bool ESPNowManager::addPeer(const uint8_t* macAddress) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        Serial.print("✅ Peer добавлен: ");
        for (int i = 0; i < 6; i++) {
            Serial.print(macAddress[i], HEX);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
        return true;
    } else {
        Serial.println("❌ Ошибка добавления peer через ESPNowManager");
        return false;
    }
}

void ESPNowManager::setConnectionStatus(bool connected) {
    if (connectionActive != connected) {
        connectionActive = connected;
        if (connected) {
            Serial.println("📶 Связь с пультом УСТАНОВЛЕНА");
            digitalWrite(HardwareConfig::LED_PIN, HIGH); // Постоянно горит при связи
        } else {
            Serial.println("📶 Связь с пультом ПОТЕРЯНА");
            digitalWrite(HardwareConfig::LED_PIN, LOW); // Выключаем при потере
        }
    }
}

void ESPNowManager::updateConnectionIndicator() {
    // Если связь активна - светодиод постоянно горит, управление не нужно
    if (connectionActive) {
        return;
    }
    
    // Если связи нет - мигаем каждые 500мс
    unsigned long currentTime = millis();
    if (currentTime - lastIndicatorUpdate > 500) {
        indicatorState = !indicatorState;
        digitalWrite(HardwareConfig::LED_PIN, indicatorState);
        lastIndicatorUpdate = currentTime;
    }
}

void ESPNowManager::updateConnection() {
    const unsigned long CONNECTION_TIMEOUT = 2000; // Таймаут связи 2 секунды
    
    // Проверяем потерю связи только если она была активна
    if (connectionActive) {
        if (millis() - lastPacketTime > CONNECTION_TIMEOUT) {
            setConnectionStatus(false);
        }
    }
    
    // Обновляем индикатор (для мигания при потере связи)
    updateConnectionIndicator();
}

void ESPNowManager::onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
    if (len != sizeof(ControlData)) {
        Serial.printf("❌ Неверный пакет: %d байт\n", len);
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
        return; // Тихий сброс пакета с ошибкой CRC
    }
    
    // Обновляем время последнего пакета и статус связи
    if (espNowInstance != nullptr) {
        espNowInstance->lastPacketTime = millis();
        espNowInstance->setConnectionStatus(true);
    }
    
    // Вызов callback функции
    if (espNowInstance != nullptr && espNowInstance->dataCallback != nullptr) {
        espNowInstance->dataCallback(receivedData);
    }
    
    // УПРОЩЕННАЯ диагностика связи
    static unsigned long lastStablePrint = 0;
    static int packetCount = 0;
    packetCount++;
    
    if (millis() - lastStablePrint > 10000) {
        Serial.printf("📡 ESP-NOW: %d пакетов/10сек\n", packetCount);
        lastStablePrint = millis();
        packetCount = 0;
    }
}