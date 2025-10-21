#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "../Core/Types.h"

class ESPNowManager {
public:
    typedef void (*DataReceivedCallback)(const ControlData& data);
    
    void begin();
    void registerCallback(DataReceivedCallback callback);
    void sendDiagnostic();
    
    // Singleton instance
    static ESPNowManager& getInstance() {
        static ESPNowManager instance;
        return instance;
    }
    
private:
    DataReceivedCallback dataCallback = nullptr;
    
    static void onDataReceived(const uint8_t* mac, const uint8_t* data, int len);
    bool validateCRC(const ControlData& data);
    
    // Приватный конструктор для singleton
    ESPNowManager() = default;
};