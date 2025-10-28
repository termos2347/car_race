// САМОЛЕТ (приемник) - УЛУЧШЕННАЯ ВЕРСИЯ
#include <WiFi.h>
#include "Core/Types.h"
#include "Actuators/ServoManager.h"
#include "Communication/ESPNowManager.h"

ServoManager servoManager;
ESPNowManager& espNowManager = ESPNowManager::getInstance();

const uint8_t transmitterMac[] = {0x14, 0x33, 0x5C, 0x37, 0x82, 0x58};

static ControlData currentData;
static unsigned long lastDataTime = 0;
static bool dataReceived = false;

void onDataReceived(const ControlData& data) {
    currentData = data;
    lastDataTime = millis();
    dataReceived = true;
    
    // НЕМЕДЛЕННОЕ обновление сервопривода
    servoManager.update(currentData);
    
    digitalWrite(2, HIGH);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n✈️ САМОЛЕТ ЗАПУЩЕН - ДИАГНОСТИЧЕСКАЯ ВЕРСИЯ");
    Serial.println("========================================");
    
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    // Инициализация ServoManager (тест ВНУТРИ begin())
    Serial.println("🔧 Инициализация ServoManager...");
    servoManager.begin();  // Тест сервопривода УЖЕ внутри этой функции

    // Инициализация ESP-NOW
    Serial.println("📡 Инициализация ESP-NOW...");
    espNowManager.begin();
    espNowManager.registerCallback(onDataReceived);
    
    if (espNowManager.addPeer(transmitterMac)) {
        Serial.println("✅ Пульт добавлен");
    } else {
        Serial.println("❌ Ошибка добавления пульта");
    }
    
    Serial.println("🚀 Система готова к работе");
    Serial.println("   Y1: -512=0°, 0=90°, 512=180°");
    Serial.println("   X1: 0=stop, 512=max speed");
    Serial.println("========================================");
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Выключаем LED через 50мс после получения данных
    if (currentMillis - lastDataTime > 50) {
        digitalWrite(2, LOW);
    }
    
    // Авто-калибровка при первом получении данных
    if (dataReceived && currentMillis - lastDataTime < 100) {
        static bool firstData = true;
        if (firstData) {
            firstData = false;
            Serial.println("✅ Первые данные получены - система активна");
        }
    }
    
    // Мигание при отсутствии данных > 2 сек
    if (currentMillis - lastDataTime > 2000) {
        static unsigned long lastBlink = 0;
        if (currentMillis - lastBlink > 500) {
            digitalWrite(2, !digitalRead(2));
            lastBlink = currentMillis;
            
            static bool connectionWarning = false;
            if (!connectionWarning) {
                Serial.println("⚠️  Ожидание данных от пульта...");
                connectionWarning = true;
            }
        }
    }
    
    // Аварийная остановка при потере связи > 3 сек
    if (currentMillis - lastDataTime > 3000) {
        static unsigned long lastEmergencyPrint = 0;
        if (currentMillis - lastEmergencyPrint > 2000) {
            Serial.println("🛑 Потеря связи с пультом - аварийный режим");
            lastEmergencyPrint = currentMillis;
        }
        servoManager.emergencyStop();
    }
    
    // Минимальная задержка для стабильности
    delay(2);
}