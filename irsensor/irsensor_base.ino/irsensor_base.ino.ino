int irpin = 14;
void setup() {
  pinMode(irpin, INPUT);
  Serial.begin(9600);
  // put your setup code here, to run once:

}

void loop() {
  int value = digitalRead(irpin);
  Serial.println(value);
  // put your main code here, to run repeatedly:
}
