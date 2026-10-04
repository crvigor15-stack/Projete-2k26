#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define PINO_HALL 34
#define PINO_EMG  35
#define PINO_LED  2

// Configurações do Filtro de Média Móvel para o EMG
const int WINDOW_SIZE = 10;
int emgBuffer[WINDOW_SIZE];
int indexBuffer = 0;
long somaEMG = 0;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Callbacks para monitorar conexões BLE
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      BLEDevice::startAdvertising(); // Reinicia o Advertising ao desconectar
    }
};

void setup() {
  Serial.begin(115200);
  
  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_LED, LOW);
  
  pinMode(PINO_HALL, INPUT);
  pinMode(PINO_EMG, INPUT);

  // Configura atenuação do ADC para leitura correta do EMG de 0 a 3.3V
  analogSetAttenuation(ADC_11db);

  // Zera o buffer da média móvel
  for (int i = 0; i < WINDOW_SIZE; i++) {
    emgBuffer[i] = 0;
  }

  // Inicialização BLE
  BLEDevice::init("Luva_NeuroMotion");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("Luva NeuroMotion (Hall + EMG) pronta para conectar via Web Bluetooth!");
}

void loop() {
  // Amostragem contínua do EMG para a Média Móvel
  int emgBruto = analogRead(PINO_EMG);
  somaEMG -= emgBuffer[indexBuffer];
  emgBuffer[indexBuffer] = emgBruto;
  somaEMG += emgBuffer[indexBuffer];
  indexBuffer = (indexBuffer + 1) % WINDOW_SIZE;
  int emgFiltrado = somaEMG / WINDOW_SIZE;

  // Transmissão de dados via BLE e Monitor Serial
  if (deviceConnected) {
    digitalWrite(PINO_LED, HIGH);

    int sensorHall = analogRead(PINO_HALL);

    // Formato do pacote transmitido via BLE: "HALL_VAL,EMG_VAL"
    char bufferPayload[32];
    snprintf(bufferPayload, sizeof(bufferPayload), "%d,%d", sensorHall, emgFiltrado);

    pCharacteristic->setValue(bufferPayload);
    pCharacteristic->notify();

    Serial.print("Hall: ");
    Serial.print(sensorHall);
    Serial.print(" | EMG Bruto: ");
    Serial.print(emgBruto);
    Serial.print(" | EMG Filtrado: ");
    Serial.println(emgFiltrado);
  } else {
    digitalWrite(PINO_LED, LOW);
  }

  delay(20); // Taxa de amostragem/transmissão de aproximadamente 50Hz
}
