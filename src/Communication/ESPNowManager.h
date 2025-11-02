#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "Core/Types.h"

class ESPNowManager {
public:
    typedef void (*DataReceivedCallback)(const ControlData& data);
    
    void begin();
    void registerCallback(DataReceivedCallback callback);
    bool addPeer(const uint8_t* macAddress);
    void sendDiagnostic();
    
    // Методы для управления индикацией связи
    void setConnectionStatus(bool connected);
    bool isConnected() const { return connectionActive; }
    void updateConnection(); // Обновление состояния связи и индикации
    
    // Singleton instance
    static ESPNowManager& getInstance() {
        static ESPNowManager instance;
        return instance;
    }
    
private:
    DataReceivedCallback dataCallback = nullptr;
    bool connectionActive = false;
    unsigned long lastPacketTime = 0;
    unsigned long lastIndicatorUpdate = 0;
    bool indicatorState = false;
    
    static void onDataReceived(const uint8_t* mac, const uint8_t* data, int len);
    bool validateCRC(const ControlData& data);
    void updateConnectionIndicator();
    
    // Приватный конструктор для singleton
    ESPNowManager() = default;
};