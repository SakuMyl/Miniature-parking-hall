int speed123 = 254;
String chars;
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

void setup() {

  Serial.begin(9600);
  ledcAttachPin(4, 0);
  ledcSetup(0, 4000, 8);
  ledcAttachPin(2, 1);
  ledcSetup(1, 4000, 8);
  Serial.setTimeout(5000);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void stop() {
  SerialBT.println("STOP!");
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  return;
}

void loop() {
  if (SerialBT.available()) {
    String order = SerialBT.readStringUntil('\n');
    SerialBT.println(speed123);
    switch(order[0]) {
      case 'r':
        SerialBT.println("Reverse!");
        ledcWrite(0, speed123);
        ledcWrite(1, 0);
        delay(1000);
        stop();
        break;
      case 'b':
        SerialBT.println("Begin!");
        ledcWrite(0, 0);
        ledcWrite(1, speed123);
        delay(1000);
        stop();
        break;
      case '1':
          speed123 = 254;
          break;
      case '2':
          speed123 = 200;
          break;
      case '3':
          speed123 = 150;
          break;
      default:
        break;
    }
  }

}
