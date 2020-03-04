int x;
#define FULLTURN 400
#define BAUD (9600)
#define STEP 0
#define DIR 2
#define ENABLE 4

String autot[] = {"123","231","321"};
#define spots 3
int paikka = 0;

void rotate(int steps, int direc) {
  if (direc) {
    digitalWrite(DIR, HIGH);
  } else {
    digitalWrite(DIR, LOW);
  }
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP, HIGH);
    delay(10);
    digitalWrite(STEP, LOW);
    delay(10);
  }
}

void setup() 
{
  Serial.begin(BAUD);
  pinMode(ENABLE,OUTPUT); // Enable
  pinMode(STEP,OUTPUT); // Step
  pinMode(DIR,OUTPUT); // Dir
  digitalWrite(ENABLE,LOW); // Set Enable low
  rotate(FULLTURN, 0);
}

int getSpotFor(String rekkari) {
  for (int i = 0; i < spots; i++) {
    if (autot[i] == rekkari) return i;
  }
  return -1;
}

void loop() 
{
  if (Serial.available()) {
    String rek = Serial.readStringUntil('\n');
    int kohta = getSpotFor(rek);
    Serial.println(String(kohta));
    if (kohta >= 0) {
      int kaanto = kohta - paikka;
      Serial.println("Rotating");
      if (kaanto > 0) {
       rotate(abs(kaanto)*133, 1);
      } else {
        rotate(abs(kaanto)*133, 0);
      }
      paikka = kohta;
      Serial.println("pausing for 3s");
      delay(3000);
    }
  }
}
