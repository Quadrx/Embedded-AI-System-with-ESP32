#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

static BLEUUID serviceUUID("12345678-1234-5678-1234-56789abcdef0");
static BLEUUID charUUID("12345678-1234-5678-1234-56789abcdef1");

BLEAdvertisedDevice* myDevice = nullptr;
bool doConnect = false;
bool connected = false;

BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

static void notifyCallback(
    BLERemoteCharacteristic* pBLERemoteCharacteristic,
    uint8_t* pData,
    size_t length,
    bool isNotify)
{
    Serial.print("Temperatura recibida: ");

    for (size_t i = 0; i < length; i++) {
        Serial.print((char)pData[i]);
    }

    Serial.println();
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {

    void onResult(BLEAdvertisedDevice advertisedDevice) {

        Serial.print("Encontrado: ");
        Serial.println(advertisedDevice.getName().c_str());

        if (advertisedDevice.getName() == "raspberry_kevin") {

            Serial.println("Raspberry encontrada");

            myDevice = new BLEAdvertisedDevice(advertisedDevice);

            BLEDevice::getScan()->stop();

            doConnect = true;
        }
    }
};

bool connectToServer() {

    Serial.println("Conectando...");

    pClient = BLEDevice::createClient();

    if (!pClient->connect(myDevice)) {

        Serial.println("Error de conexion");
        return false;
    }

    Serial.println("Conectado");

    BLERemoteService* pRemoteService =
        pClient->getService(serviceUUID);

    if (pRemoteService == nullptr) {

        Serial.println("Servicio no encontrado");
        pClient->disconnect();
        return false;
    }

    Serial.println("Servicio encontrado");

    pRemoteCharacteristic =
        pRemoteService->getCharacteristic(charUUID);

    if (pRemoteCharacteristic == nullptr) {

        Serial.println("Caracteristica no encontrada");
        pClient->disconnect();
        return false;
    }

    Serial.println("Caracteristica encontrada");

    Serial.print("Can Notify: ");
    Serial.println(pRemoteCharacteristic->canNotify());

    if (pRemoteCharacteristic->canNotify()) {

        pRemoteCharacteristic->registerForNotify(
            notifyCallback
        );

        Serial.println("Suscrito a notificaciones");
    }
    else {

        Serial.println("La caracteristica no soporta notify");
    }

    return true;
}

void setup() {

    Serial.begin(115200);

    Serial.println("Escaneando...");

    BLEDevice::init("");

    BLEScan* pScan = BLEDevice::getScan();

    pScan->setAdvertisedDeviceCallbacks(
        new MyAdvertisedDeviceCallbacks()
    );

    pScan->setActiveScan(true);

    pScan->start(30, false);
}

void loop() {

    if (doConnect && !connected) {

        connected = connectToServer();

        doConnect = false;
    }

    if (connected && pClient != nullptr) {

        if (!pClient->isConnected()) {

            Serial.println("Conexion perdida");

            connected = false;

            BLEDevice::getScan()->start(30, false);
        }
    }

    delay(100);
}
