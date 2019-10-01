// This sketch catches BLE advertisements and sends them to MQTT
// 1. In the Arduino IDE, choose Tools / Board / ESP32 Dev module
// 2. In the Arduino IDE, choose Tools / Partition Scheme / Minimal SPIFFS
// 3. Flash this sketch to the ESP32 board

#define MDASH_APP_NAME "ble-gateway"
#include <mDash.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <WiFi.h>

#define WIFI_NETWORK "MyWifiNetworkName"
#define WIFI_PASSWORD "MyWifiPassword"
#define DEVICE_PASSWORD "mDashDeviceToken"

int scanTime = 3;  // In seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice d) {
      char addr[20];
      const unsigned char *p = d.getPayload();
      int plen = d.getPayloadLength();

      snprintf(addr, sizeof(addr), "%s", d.getAddress().toString().c_str());
      mDashNotify("DB.Save", "{\"mac\":%Q,\"adv\":%H}", addr, plen, p);
      Serial.printf("Advertised Device: %s\n", addr);
    }
};

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  mDashSetServer("mdash.net", 1883);
  mDashBegin(DEVICE_PASSWORD);

  // Until connected to the cloud, enable provisioning over serial
  while (mDashGetState() != MDASH_CONNECTED)
    if (Serial.available() > 0) mDashCLI(Serial.read());

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  // active scan uses more power
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();
  delay(50);
}
