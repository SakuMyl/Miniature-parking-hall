#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  if (SerialBT.available()) {
    char command = SerialBT.read();
    switch(command) {
      case 'e':
        aja();
        break;
      case 't':
        peruuta();
        break;
      case 'p':
        parkeeraa();
        break;
      default:
        break;
    }
    
  }
  delay(20);
}
