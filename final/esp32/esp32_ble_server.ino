#include <Wire.h>
#include <Adafruit_MLX90614.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SDA_PIN 47
#define SCL_PIN 48

#define LED_PIN 45

Adafruit_MLX90614 mlx;

BLECharacteristic *tempCharacteristic;
BLECharacteristic *controlCharacteristic;

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define TEMP_UUID           "12345678-1234-5678-1234-56789abcdef1"
#define CONTROL_UUID        "12345678-1234-5678-1234-56789abcdef2"

class ControlCallbacks : public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {

        String value = pCharacteristic->getValue().c_str();

        Serial.print("Comando recibido: ");
        Serial.println(value);

        if(value == "LED_ON"){
            digitalWrite(LED_PIN,HIGH);
        }

        if(value == "LED_OFF"){
            digitalWrite(LED_PIN,LOW);
        }

    }

};

void setup() {

    Serial.begin(115200);

    pinMode(LED_PIN,OUTPUT);

    Wire.begin(SDA_PIN,SCL_PIN);

    if(!mlx.begin()){
        Serial.println("MLX90614 no encontrado");
        while(1);
    }

    BLEDevice::init("ESP32_SENSOR");

    BLEServer *server = BLEDevice::createServer();

    BLEService *service =
        server->createService(SERVICE_UUID);

    tempCharacteristic =
        service->createCharacteristic(
            TEMP_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY
        );

    controlCharacteristic =
        service->createCharacteristic(
            CONTROL_UUID,
            BLECharacteristic::PROPERTY_WRITE
        );

    controlCharacteristic->setCallbacks(new ControlCallbacks());

    service->start();

    BLEAdvertising *advertising =
        BLEDevice::getAdvertising();

    advertising->addServiceUUID(SERVICE_UUID);
    advertising->start();

    Serial.println("Servidor BLE iniciado");
}

void loop() {

    float obj = mlx.readObjectTempC();
    float amb = mlx.readAmbientTempC();

    String dato =
        String(obj,2) + "," + String(amb,2);

    Serial.println(dato);

    tempCharacteristic->setValue(dato.c_str());
    tempCharacteristic->notify();

    delay(1000);
}
