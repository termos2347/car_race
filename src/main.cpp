// САМОЛЕТ (приемник) - С ВЫВОДОМ MAC-АДРЕСОВ
#include <esp_now.h>
#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"

#define DEBUG_MODE true

ServoManager servoManager;
const uint8_t transmitterMac[] = {0xEC, 0xE3, 0x34, 0x19, 0x23, 0xD4};

static ControlData currentData;
static unsigned long lastDataTime = 0;
static unsigned long lastBlinkTime = 0;
static unsigned long ledOffTime = 0;
static bool ledState = false;

enum Timing {
  LED_BLINK_INTERVAL = 2000,
  LED_INDICATION_TIME = 25
};

// Функция для форматированного вывода MAC-адреса
void printMacAddress(const uint8_t* mac, const char* label) {
  #if DEBUG_MODE
    Serial.printf("%s: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                 label, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  #endif
}

bool addPeer(const uint8_t* macAddress) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    return esp_now_add_peer(&peerInfo) == ESP_OK;
}

void onDataReceived(const ControlData& data) {
    currentData = data;
    lastDataTime = millis();
    
    digitalWrite(2, HIGH);
    ledState = true;
    ledOffTime = millis() + LED_INDICATION_TIME;
    
    // Проверка CRC
    uint16_t calculatedCRC = 0;
    const uint8_t* bytes = (const uint8_t*)&data;
    for(size_t i = 0; i < sizeof(ControlData) - sizeof(uint16_t); i++) {
        calculatedCRC += bytes[i];
    }
    
    if (calculatedCRC == data.crc) {
      servoManager.update(currentData);
      
      #if DEBUG_MODE
        static unsigned long lastDataPrint = 0;
        if (millis() - lastDataPrint > 100) {
          Serial.printf("J1:%4d,%4d J2:%4d,%4d\n", 
                       data.xAxis1, data.yAxis1, 
                       data.xAxis2, data.yAxis2);
          lastDataPrint = millis();
        }
      #endif
    } else {
      #if DEBUG_MODE
        Serial.println("❌ Ошибка CRC");
      #endif
    }
}

void setup() {
  #if DEBUG_MODE
    Serial.begin(115200);
    delay(500);
    Serial.println("✈️ САМОЛЕТ ЗАПУЩЕН");
    Serial.println("========================");
  #endif
  
  // Вывод MAC-адресов ДО инициализации компонентов
  #if DEBUG_MODE
    Serial.print("MAC самолета:  ");
    Serial.println(WiFi.macAddress());
    printMacAddress(transmitterMac, "MAC пульта  ");
    Serial.println("------------------------");
  #endif
  
  pinMode(2, OUTPUT);
  servoManager.begin();
  
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    #if DEBUG_MODE
      Serial.println("❌ Ошибка инициализации ESP-NOW");
    #endif
    return;
  }
  
  esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (len == sizeof(ControlData)) {
      ControlData data;
      memcpy(&data, incomingData, sizeof(ControlData));
      onDataReceived(data);
    }
  });
  
  if (addPeer(transmitterMac)) {
    #if DEBUG_MODE
      Serial.println("✅ Пульт добавлен в пиры");
    #endif
  } else {
    #if DEBUG_MODE
      Serial.println("❌ Ошибка добавления пульта");
    #endif
  }
  
  // Калибровка сервоприводов
  #if DEBUG_MODE
    servoManager.calibrate();
  #else
    servoManager.quickCalibrate();
  #endif
  
  #if DEBUG_MODE
    Serial.println("🚀 Самолет готов к работе");
    Serial.println("========================");
  #endif
}

void loop() {
  unsigned long currentMillis = millis();
  
  // LED индикация получения данных
  if (ledState && currentMillis > ledOffTime) {
    digitalWrite(2, LOW);
    ledState = false;
  }
  
  // Мигание при отсутствии данных
  if (!ledState && currentMillis - lastDataTime > 1000) {
    if (currentMillis - lastBlinkTime > LED_BLINK_INTERVAL) {
      digitalWrite(2, !digitalRead(2));
      lastBlinkTime = currentMillis;
    }
  }
}