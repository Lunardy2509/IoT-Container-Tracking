#define SIM800L_IP5306_VERSION_20200811
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 1024

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

// === Sensor & GPS Config ===
#define DHTPIN 18
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define RX 32
#define TX 33
#define GPS_BAUD 9600
HardwareSerial neogps(2);
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;

// === Modem & MQTT Config ===
#define SerialMon Serial
#define SerialAT Serial1
#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26

TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);
PubSubClient mqttClient(gsmClient);

const char apn[] = "telkomsel";
const char gprsUser[] = "wap";
const char gprsPass[] = "wap123";
const char* mqttServer = "mqtt.eclipseprojects.io";
const int mqttPort = 1883;

const char* mqttTopicMPU = "esp32/mpu_data";              // MPU data (every 1 second)
const char* mqttTopicContainer = "esp32/container_data";  // Container data (every 15 mins)
const char* container_number = "ESP32S";
const char* container_type = "REEFER";

// === Timing & Buffer Config ===
unsigned long lastMPUSendTime = 0;
unsigned long lastContainerSendTime = 0;
const unsigned long MPU_SEND_INTERVAL = 1000;         // MPU data (every 1 second)
const unsigned long CONTAINER_SEND_INTERVAL = 900000; // Container data (every 15 mins) 900000

#define MAX_READINGS 60
float netAccelReadings[MAX_READINGS];
float accelXBuffer[MAX_READINGS];
float accelYBuffer[MAX_READINGS];
float accelZBuffer[MAX_READINGS];
int currentIndex = 0;
bool bufferFull = false;

// === Helper Functions ===
void powerOnModem() {
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_POWER_ON, HIGH);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(100);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, HIGH);
}

void setupModem() {
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  modem.restart();
  modem.gprsConnect(apn, gprsUser, gprsPass);
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) return;
    delay(5000);
  }
}

float computeNetAccel(float x, float y, float z) {
  float magnitude = sqrt(x * x + y * y + z * z);
  return abs(magnitude - 9.81);
}

float getMaxNetAccel() {
  float maxVal = netAccelReadings[0];
  int limit = bufferFull ? MAX_READINGS : currentIndex;
  for (int i = 1; i < limit; i++) {
    if (netAccelReadings[i] > maxVal) maxVal = netAccelReadings[i];
  }
  return maxVal;
}

float computeAverage(float* dataArray) {
  int limit = bufferFull ? MAX_READINGS : currentIndex;
  float sum = 0;
  for (int i = 0; i < limit; i++) sum += dataArray[i];
  return sum / limit;
}

String determineShockStatus(float maxNetAccel) {
  if (maxNetAccel <= 10.31279056) return "NORMAL";
  else if (maxNetAccel <= 13.33154273) return "MODERATE";
  else if (maxNetAccel <= 18.30426328) return "SEVERE";
}

// === Setup ===
void setup() {
  SerialMon.begin(115200);
  delay(10);
  powerOnModem();
  setupModem();
  setupMQTT();

  neogps.begin(GPS_BAUD, SERIAL_8N1, RX, TX);
  dht.begin();

  if (!mpu.begin()) {
    SerialMon.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  SerialMon.println("MPU6050 sensor initialized");
}

// === Loop ===
void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  unsigned long currentMillis = millis();

  // === MPU Reading ===
  if (currentMillis - lastMPUSendTime >= MPU_SEND_INTERVAL) {
    lastMPUSendTime = currentMillis;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float accelX = a.acceleration.x;
    float accelY = a.acceleration.y;
    float accelZ = a.acceleration.z;
    float netAccel = computeNetAccel(accelX, accelY, accelZ);

    netAccelReadings[currentIndex] = netAccel;
    accelXBuffer[currentIndex] = accelX;
    accelYBuffer[currentIndex] = accelY;
    accelZBuffer[currentIndex] = accelZ;
    currentIndex++;

    if (currentIndex >= MAX_READINGS) {
      currentIndex = 0;
      bufferFull = true;

      float maxNet = getMaxNetAccel();
      float avgX = computeAverage(accelXBuffer);
      float avgY = computeAverage(accelYBuffer);
      float avgZ = computeAverage(accelZBuffer);
      String shockStatus = determineShockStatus(maxNet);

      String payloadMPU = "{";
      payloadMPU += "\"container_number\": \"" + String(container_number) + "\", ";
      payloadMPU += "\"shock_status\": \"" + shockStatus + "\", ";
      payloadMPU += "\"avg_accel_x\": " + String(avgX) + ", ";
      payloadMPU += "\"avg_accel_y\": " + String(avgY) + ", ";
      payloadMPU += "\"avg_accel_z\": " + String(avgZ);
      payloadMPU += "}";

      SerialMon.println("Publishing MPU Data:");
      SerialMon.println(payloadMPU);
      mqttClient.publish(mqttTopicMPU, payloadMPU.c_str());
    }
  }

  // === Container Data ===
  if (currentMillis - lastContainerSendTime >= CONTAINER_SEND_INTERVAL) {
    lastContainerSendTime = currentMillis;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    while (neogps.available() > 0) gps.encode(neogps.read());

    String payloadContainer = "{";
    payloadContainer += "\"container_number\": \"" + String(container_number) + "\", ";
    payloadContainer += "\"container_type\": \"" + String(container_type) + "\", ";
    payloadContainer += "\"temperature\": " + String(temperature) + "°C, ";
    payloadContainer += "\"humidity\": " + String(humidity) + "%";

    if (gps.location.isValid()) {
      payloadContainer += ", \"latitude\": " + String(gps.location.lat(), 6);
      payloadContainer += ", \"longitude\": " + String(gps.location.lng(), 6);
    }
    payloadContainer += "}";

    SerialMon.println("Publishing Container Data:");
    SerialMon.println(payloadContainer);
    mqttClient.publish(mqttTopicContainer, payloadContainer.c_str());
  }
}
