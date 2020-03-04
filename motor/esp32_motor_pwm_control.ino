int led_gpio = 4;
int brightness = 0;
int speed123 = 0;

void setup() {
  pinMode(2, OUTPUT);
  //pinMode(4, OUTPUT);
  //pinMode(0, OUTPUT);
  Serial.begin(9600);
  ledcAttachPin(led_gpio, 0);
  ledcSetup(0, 4000, 8);
  Serial.setTimeout(5000);
}

void loop() {
  if (Serial.available()) {
    String order = Serial.readStringUntil('\n');
    switch(order[0]) {
      case 's':
        Serial.println("STOP!");
        ledcWrite(0, 0);
        digitalWrite(2, LOW);
        break;
      case 'r':
        Serial.println("Reverse!");
        ledcWrite(0, 255);
        digitalWrite(2, LOW);
        break;
      case 'b':
        Serial.println("Begin!");
        ledcWrite(0, 0);
        digitalWrite(2, HIGH);
        break;
      case 'm':
          speed123 = Serial.parseInt();
          ledcWrite(0, speed123);
          digitalWrite(2, LOW);
      default:
        ledcWrite(0, speed123);
        digitalWrite(2, LOW);
        break;
    }
  }
}
