// САМОЛЕТ (приемник) - С АВТОТЕСТОМ СЕРВОПРИВОДА
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
      // НЕМЕДЛЕННО обновляем сервопривод и двигатель
      servoManager.update(currentData);
      
      #if DEBUG_MODE
        static unsigned long lastDataPrint = 0;
        if (millis() - lastDataPrint > 200) {
          Serial.printf("📡 X1:%4d (серво) | Y1:%4d (двигатель)\n", data.xAxis1, data.yAxis1);
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
    Serial.println("✈️ САМОЛЕТ ЗАПУЩЕН (1 серв + 1 двигатель)");
    Serial.println("========================");
  #endif
  
  // Вывод MAC-адресов
  #if DEBUG_MODE
    Serial.print("MAC самолета:  ");
    Serial.println(WiFi.macAddress());
    printMacAddress(transmitterMac, "MAC пульта  ");
    Serial.println("------------------------");
  #endif
  
  pinMode(2, OUTPUT);
  
  // ИНИЦИАЛИЗАЦИЯ И ТЕСТ СЕРВОПРИВОДА
  #if DEBUG_MODE
    Serial.println("🎯 ИНИЦИАЛИЗАЦИЯ СЕРВОПРИВОДА...");
  #endif
  servoManager.begin();
  
  // ЗАПУСК ТЕСТОВОЙ ПОСЛЕДОВАТЕЛЬНОСТИ СРАЗУ ПОСЛЕ ИНИЦИАЛИЗАЦИИ
  #if DEBUG_MODE
    Serial.println("🎯 ЗАПУСК АВТОТЕСТА СЕРВОПРИВОДА...");
    delay(100); // Минимальная задержка для стабилизации
  #endif
  servoManager.testSequence();
  
  // Продолжаем обычную инициализацию
  #if DEBUG_MODE
    Serial.println("📡 ИНИЦИАЛИЗАЦИЯ ESP-NOW...");
  #endif
  
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
  
  // Быстрая калибровка
  servoManager.quickCalibrate();
  
  #if DEBUG_MODE
    Serial.println("🚀 Самолет готов к работе");
    Serial.println("   X1 - сервопривод, Y1 - двигатель");
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