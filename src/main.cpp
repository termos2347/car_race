// САМОЛЕТ (приемник) - С DEBUG РЕЖИМОМ
#include <esp_now.h>
#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

// === НАСТРОЙКИ DEBUG ===
#define DEBUG_MODE true  // true - отладка, false - полет (минимальный вывод)
// =======================

ServoManager servoManager;

// MAC адрес пульта (передатчика)
const uint8_t transmitterMac[] = {0xEC, 0xE3, 0x34, 0x19, 0x23, 0xD4};

// Глобальные переменные
static ControlData currentData;
static bool newDataReceived = false;
static unsigned long lastDataTime = 0;

// Тайминги (в мс)
enum Timing {
  DATA_PROCESS_INTERVAL = 30,     // Обработка данных
  STATUS_PRINT_INTERVAL = 1000,   // Вывод статуса
  LED_BLINK_INTERVAL = 2000,      // Мигание LED в режиме ожидания
  LED_INDICATION_TIME = 25        // Индикация LED
};

// Переменные для неблокирующих таймеров
static unsigned long lastDataProcess = 0;
static unsigned long lastStatusPrint = 0;
static unsigned long lastBlinkTime = 0;
static unsigned long ledOffTime = 0;
static bool ledState = false;
static bool connectionStatus = false;

// Прототипы функций
bool addPeer(const uint8_t* macAddress);
void printDeviceInfo();
void setupIndication();
void handleDataProcessing(unsigned long currentMillis);
void handleLEDIndication(unsigned long currentMillis);
void handleStatusOutput(unsigned long currentMillis);
void handleConnectionCheck(unsigned long currentMillis);

// Функция для добавления пира в ESP-NOW
bool addPeer(const uint8_t* macAddress) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    return esp_now_add_peer(&peerInfo) == ESP_OK;
}

// Вывод информации об устройстве
void printDeviceInfo() {
  #if DEBUG_MODE
    Serial.println("✈️ Система управления самолетом запущена");
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Free RAM: ");
    Serial.println(ESP.getFreeHeap());
  #endif
}

// Индикация готовности
void setupIndication() {
  pinMode(2, OUTPUT);
  for(int i = 0; i < 3; i++) {
    digitalWrite(2, HIGH);
    delay(80);
    digitalWrite(2, LOW);
    delay(80);
  }
}

// Обработка полученных данных
void onDataReceived(const ControlData& data) {
    currentData = data;
    newDataReceived = true;
    lastDataTime = millis();
    
    // Индикация получения данных
    digitalWrite(2, HIGH);
    ledState = true;
    ledOffTime = millis() + LED_INDICATION_TIME;
    
    // Вывод данных в одинаковом формате (только в debug режиме)
    #if DEBUG_MODE
      static unsigned long lastDataPrint = 0;
      if (millis() - lastDataPrint > 100) {
          Serial.printf("J1:%4d,%4d J2:%4d,%4d B1:%d B2:%d CRC:%4d\n", 
                       data.xAxis1, data.yAxis1, 
                       data.xAxis2, data.yAxis2,
                       data.button1, data.button2, data.crc);
          lastDataPrint = millis();
      }
    #endif
}

void setup() {
  #if DEBUG_MODE
    Serial.begin(115200);
    delay(800);
    Serial.println("✈️ Запуск системы управления самолетом...");
  #endif
  
  printDeviceInfo();
  
  // Настройка LED для индикации
  setupIndication();
  
  // Инициализация компонентов
  servoManager.begin();
  
  // Настройка WiFi и ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(true);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  
  if (esp_now_init() != ESP_OK) {
    #if DEBUG_MODE
      Serial.println("❌ Ошибка инициализации ESP-NOW");
    #endif
    return;
  }
  
  // Регистрируем callback для приема данных
  esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (len == sizeof(ControlData)) {
      ControlData data;
      memcpy(&data, incomingData, sizeof(ControlData));
      onDataReceived(data);
    }
  });
  
  // Добавляем пульт как пир
  if (addPeer(transmitterMac)) {
    #if DEBUG_MODE
      Serial.println("✅ Пульт добавлен");
    #endif
    connectionStatus = true;
  } else {
    #if DEBUG_MODE
      Serial.println("❌ Ошибка добавления пульта");
    #endif
    connectionStatus = false;
  }
  
  // Калибровка сервоприводов
  #if DEBUG_MODE
    servoManager.calibrate();
  #else
    // В режиме полета - быстрая калибровка без вывода
    for(int i = 0; i < 8; i++) {
      servoManager.servos[i].servo.writeMicroseconds(1500);
    }
    delay(100);
  #endif
  
  #if DEBUG_MODE
    Serial.println("🚀 Самолет готов к работе");
  #endif
}

void loop() {
  const unsigned long currentMillis = millis();
  
  // Разделение задач по времени для равномерной нагрузки
  handleDataProcessing(currentMillis);
  handleLEDIndication(currentMillis);
  handleStatusOutput(currentMillis);
  handleConnectionCheck(currentMillis);
}

// Обработка полученных данных
void handleDataProcessing(unsigned long currentMillis) {
  if (currentMillis - lastDataProcess >= DATA_PROCESS_INTERVAL) {
    if (newDataReceived) {
      servoManager.update(currentData);
      newDataReceived = false;
    }
    lastDataProcess = currentMillis;
  }
}

// Управление LED индикацией
void handleLEDIndication(unsigned long currentMillis) {
  // Управление LED для индикации получения данных
  if (ledState && currentMillis > ledOffTime) {
    digitalWrite(2, LOW);
    ledState = false;
  }
  
  // Медленное мигание в режиме ожидания
  if (!ledState && currentMillis - lastDataTime > 1000) {
    if (currentMillis - lastBlinkTime > LED_BLINK_INTERVAL) {
      digitalWrite(2, !digitalRead(2));
      lastBlinkTime = currentMillis;
    }
  }
}

// Вывод статуса
void handleStatusOutput(unsigned long currentMillis) {
  #if DEBUG_MODE
    if (currentMillis - lastStatusPrint >= STATUS_PRINT_INTERVAL) {
      // Выводим статус только если не было данных в течение интервала
      if (currentMillis - lastDataTime > STATUS_PRINT_INTERVAL - 500) {
        if (connectionStatus) {
          if (currentMillis - lastDataTime < 2000) {
            Serial.println("✅ Связь стабильная, данные поступают");
          } else {
            Serial.println("⚠️  Пульт подключен, но данные не поступают");
          }
        } else {
          Serial.println("❌ Пульт не зарегистрирован");
        }
      }
      lastStatusPrint = currentMillis;
    }
  #endif
}

// Проверка соединения
void handleConnectionCheck(unsigned long currentMillis) {
  #if DEBUG_MODE
    static unsigned long lastConnectionCheck = 0;
    
    if (currentMillis - lastConnectionCheck > 10000) {
      bool currentStatus = esp_now_is_peer_exist(transmitterMac);
      
      if (currentStatus != connectionStatus) {
        connectionStatus = currentStatus;
        if (connectionStatus) {
          Serial.println("🔗 Пульт подключен");
        } else {
          Serial.println("🔌 Пульт отключен");
        }
      }
      lastConnectionCheck = currentMillis;
    }
  #endif
}