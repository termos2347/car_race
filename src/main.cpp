// САМОЛЕТ (приемник) - ОПТИМИЗИРОВАННАЯ ВЕРСИЯ
#include <esp_now.h>
#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

ServoManager servoManager;

// MAC адрес пульта (передатчика)
const uint8_t transmitterMac[] = {0xEC, 0xE3, 0x34, 0x19, 0x23, 0xD4};

// Глобальные переменные для оптимизации
static ControlData lastReceivedData;
static bool newDataReceived = false;
static unsigned long lastDataTime = 0;

// Тайминги (в мс)
enum Timing {
  DATA_RECEIVE_TIMEOUT = 1000,    // Таймаут потери связи
  SERIAL_PRINT_INTERVAL = 5000,   // Вывод в Serial
  LED_BLINK_INTERVAL = 2000,      // Мигание LED в режиме ожидания
  LED_INDICATION_TIME = 25        // Короткая индикация при получении данных
};

// Переменные для неблокирующих таймеров
static unsigned long lastSerialPrint = 0;
static unsigned long lastBlinkTime = 0;
static unsigned long ledOffTime = 0;
static bool ledState = false;
static bool connectionStatus = false;

// Прототипы функций (ИСПРАВЛЕНО - добавлены параметры)
bool addPeer(const uint8_t* macAddress);
void optimizedDeviceInfo();
void handleDataProcessing();
void handleLEDIndication(unsigned long currentMillis);
void handleSerialOutput(unsigned long currentMillis);
void handleConnectionCheck(unsigned long currentMillis);

// Функция для добавления пира в ESP-NOW
bool addPeer(const uint8_t* macAddress) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    esp_err_t result = esp_now_add_peer(&peerInfo);
    return result == ESP_OK;
}

// Оптимизированный вывод информации об устройстве
void optimizedDeviceInfo() {
  Serial.println("✈️ Система управления самолетом запущена");
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Free RAM: ");
  Serial.println(ESP.getFreeHeap());
}

// Обработка полученных данных (без блокирующих задержек)
void onControlDataReceived(const ControlData& data) {
    lastReceivedData = data;
    newDataReceived = true;
    lastDataTime = millis();
    
    // Быстрая индикация получения данных
    digitalWrite(2, HIGH);
    ledState = true;
    ledOffTime = millis() + LED_INDICATION_TIME;
    
    // Минимальный вывод при получении данных (только при изменении)
    static ControlData lastPrintedData;
    const int PRINT_DEADZONE = 5;
    
    if (abs(data.xAxis1 - lastPrintedData.xAxis1) > PRINT_DEADZONE ||
        abs(data.yAxis1 - lastPrintedData.yAxis1) > PRINT_DEADZONE) {
        Serial.printf("📥 Данные: X=%4d, Y=%4d\n", data.xAxis1, data.yAxis1);
        lastPrintedData = data;
    }
}

void setup() {
  Serial.begin(115200);
  delay(800);  // Уменьшенная задержка
  
  Serial.println("✈️ Запуск системы управления самолетом...");
  
  // Оптимизированная информация об устройстве
  optimizedDeviceInfo();
  
  // Настройка LED для индикации
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  
  // Инициализация компонентов
  servoManager.begin();
  
  // Настройка WiFi и ESP-NOW
  WiFi.mode(WIFI_STA);
  
  // Оптимизация WiFi для снижения энергопотребления
  WiFi.setSleep(true);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Ошибка инициализации ESP-NOW");
    return;
  }
  
  // Регистрируем callback для приема данных
  esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (len == sizeof(ControlData)) {
      ControlData data;
      memcpy(&data, incomingData, sizeof(ControlData));
      onControlDataReceived(data);
    }
  });
  
  // Добавляем пульт как пир
  if (addPeer(transmitterMac)) {
    Serial.println("✅ Пульт добавлен");
    connectionStatus = true;
  } else {
    Serial.println("❌ Ошибка добавления пульта");
    connectionStatus = false;
  }
  
  // Калибровка сервоприводов
  servoManager.calibrate();
  
  // Индикация готовности (укороченная)
  for(int i = 0; i < 3; i++) {
    digitalWrite(2, HIGH);
    delay(80);
    digitalWrite(2, LOW);
    delay(80);
  }
  
  Serial.println("🚀 Самолет готов к работе");
}

void loop() {
  const unsigned long currentMillis = millis();

  // Разделение задач по времени для равномерной нагрузки
  handleDataProcessing();
  handleLEDIndication(currentMillis);
  handleSerialOutput(currentMillis);
  handleConnectionCheck(currentMillis);
}

// Обработка полученных данных
void handleDataProcessing() {
  if (newDataReceived) {
    servoManager.update(lastReceivedData);
    newDataReceived = false;
  }
}

// Управление LED индикацией
void handleLEDIndication(unsigned long currentMillis) {
  // Управление LED для индикации получения данных
  if (ledState && currentMillis > ledOffTime) {
    digitalWrite(2, LOW);
    ledState = false;
  }
  
  // Медленное мигание в режиме ожидания (если нет данных более 1 секунды)
  if (!ledState && currentMillis - lastDataTime > DATA_RECEIVE_TIMEOUT) {
    if (currentMillis - lastBlinkTime > LED_BLINK_INTERVAL) {
      digitalWrite(2, !digitalRead(2));
      lastBlinkTime = currentMillis;
    }
  }
}

// Управление выводом в Serial
void handleSerialOutput(unsigned long currentMillis) {
  if (currentMillis - lastSerialPrint >= SERIAL_PRINT_INTERVAL) {
    // Проверка соединения и вывод статуса
    bool peerExists = esp_now_is_peer_exist(transmitterMac);
    
    if (peerExists) {
      if (currentMillis - lastDataTime < DATA_RECEIVE_TIMEOUT) {
        Serial.println("✅ Связь стабильная, данные поступают");
      } else {
        Serial.println("⚠️  Пульт подключен, но данные не поступают");
      }
    } else {
      Serial.println("❌ Пульт не зарегистрирован");
    }
    
    lastSerialPrint = currentMillis;
  }
}

// Проверка соединения
void handleConnectionCheck(unsigned long currentMillis) {
  static unsigned long lastConnectionCheck = 0;
  
  // Проверяем соединение раз в 10 секунд
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
}