#include <WiFi.h>
#include <ESP32Servo.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Initialize Wifi connection to the router
char ssid[] = "aalto open";     // your network SSID (name)
char password[] = ""; // your network key

// Initialize Telegram BOT
#define BOTtoken "991870882:AAHpEmLLp6IxCeFNGvLeExnWgh_dtDAnX8E"  // your Bot Token (Get from Botfather)

// STEPPER pins
#define STEP 27
#define DIR 14
#define ENABLE 12

// STEPPER properties
#define FULLTURN 200 // steps to make a full turn
#define DELAY 25 // delay inbetween STEP-pulses in ms

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime = 0;   //last time messages' scan has been done
bool Start = false;
bool parkOrLeaveInProcess = false;

Servo gateServo;
const int gatePin = 25;
const int irPin = 26;
// Mocked for now
const int switchPin = 13;

int parked = 0;
const int spots = 3;
String cars[spots];
String chatIds[spots];
int current_position = 0;
const int spot_angle = FULLTURN / spots - 1; // for zeroing-switch

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chatId = String(bot.messages[i].chat_id);
    if (parkOrLeaveInProcess) {
      bot.sendMessage("Someone is already entering or leaving the parking hall. Please wait.", "");
      continue;
    }
    String text = bot.messages[i].text;

    String fromName = bot.messages[i].from_name;
    if (fromName == "") fromName = "Guest";

    if (text == "/open") {
      openGate();
      bot.sendMessage(chatId, "Gate opened, you may pass", "");
    }

    if (text == "/close") {
      closeGate();
      bot.sendMessage(chatId, "Gate closed, YOU SHALL NOT PASS!", "");
    }

    if (text == "/spots") {
      bot.sendMessage(chatId, String(spots - parked) + " spot(s) left.");
    }

    if (text.startsWith("/park")) {
      String plateNumber = text.substring(6);
      if (validatePlateNumber(plateNumber, chatId)) {
        park(plateNumber, chatId);
      }
    }

    if (text.startsWith("/leave")) {
      String plateNumber = text.substring(7);
      if (validatePlateNumber(plateNumber, chatId)) {
        leave(plateNumber, chatId);
      }
    }

    if (text == "/start") {
      start(chatId);
    }
  }
}

bool checkPassage() {
  Serial.println("check passage");
  int passing = 0;
  int start_time = millis();
  int timeout = 20000;
  while (!passing) {
    // Check the state twice to account for random incorrect bits
    int state = digitalRead(irPin);
    delay(10);
    passing = state ? digitalRead(irPin) : 0;
    delay(100);
    if (millis() > start_time+timeout) return false;
  }
  while (passing) {
    passing = digitalRead(irPin);
    delay(100);
  }
  delay(2000);
  return true;
}

bool validatePlateNumber(String plateNumber, String chatId) {
  if (plateNumber.length() != 3) {
    bot.sendMessage(chatId, "Please give a three character plate number after the command separated by a space.", "");
    return false;
  }
  return true;
}

int getSpotFor(String plateNumber, String chatId) {
  for (int i = 0; i < spots; i++) {
    if (cars[i] == plateNumber && chatIds[i]== chatId) return i;
  }
  return -1;
}

int findNewSpotFor(String plateNumber, String chatId) {
  for (int i = 0; i < spots; i++) {
    if (cars[i] == "") {
      cars[i] = plateNumber;
      chatIds[i] = chatId;
      return i;
    }
  }
  return -1;
}

void leave(String plateNumber, String chatId) {
  Serial.println("leave");
  int spot = getSpotFor(plateNumber, chatId);
  if (spot < 0) {
    bot.sendMessage(chatId, "Either car with plate number " + plateNumber + " is not parked or the car isn't yours.", "");
  } else {
    parkOrLeaveInProcess = true;
    int steps = abs((current_position - spot)*spot_angle);
    int direc = current_position-spot;
    rotateDisk(steps, direc);
    current_position = spot;
    openGate();
    bot.sendMessage(chatId, "You may leave.", "");
    if (checkPassage()) {
      cars[spot] = "";
      chatIds[spot] = "";
      parked--;
    } else {
      bot.sendMessage(chatId, "Are you having trouble with leaving?", "");
    }
    closeGate();
    parkOrLeaveInProcess = false;
  }
}

void start(String chatId) {
  bot.sendMessage(chatId, "Commands:\n"
                          "/start: display a list of commands\n"
                          "/open: open the gate\n"
                          "/close: close the gate\n"
                          "/park ASD: park a car with plate number ASD (ASD can be replaced with any plate number)\n"
                          "/leave ASD: get a car with plate number ASD out (similar to /park)\n"
                          "/spots: get the number of spots left in the parking hall\n"
                          ,""
                 );
}

void park(String plateNumber, String chatId) {
  Serial.println("park");
  if (parked == spots) {
    bot.sendMessage(chatId, "No spots left, sorry.", "");
    return;
  }
  for (int i = 0; i < spots; i++) {
    if (cars[i] == plateNumber) {
      bot.sendMessage(chatId, plateNumber + " already inside.", "");
      return;
    }
  }
  parkOrLeaveInProcess = true;
  // Should never return zero at this point
  int i;
  if ((i = findNewSpotFor(plateNumber, chatId)) < 0) return;
  int steps = abs((current_position - i)*spot_angle);
  int direc = current_position-i;
  rotateDisk(steps, direc);
  current_position = i;
  openGate();
  bot.sendMessage(chatId, plateNumber + ", you may go in.", "");
  if (!(checkPassage())) {
    bot.sendMessage(chatId, "The sensor didn't see you go in. Contact the infodesk.", "");
  }
  closeGate();
  parked++;
  parkOrLeaveInProcess = false;
}

void rotateDisk(int steps, int direc) {
  Serial.println("rotate disk");
  if (direc >= 0) {
    digitalWrite(DIR, LOW);
  } else {
    digitalWrite(DIR, HIGH);
  }
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP, HIGH);
    delay(DELAY);
    digitalWrite(STEP, LOW);
    delay(DELAY);
  }
}

// Calibrate disk rotation
void diskSetup() {
  digitalWrite(DIR, LOW);
  Serial.println("Start diskSetup");
  int value = 0;
  
  while (!digitalRead(switchPin)) {
    Serial.println(value); // DEBUG
    digitalWrite(STEP, HIGH);
    delay(40);
    digitalWrite(STEP, LOW);
    delay(40);
    int value = digitalRead(switchPin);
  }
  Serial.println("123");
}

void openGate() {
  Serial.println("Gate open");
  gateServo.write(85);
}

void closeGate() {
  Serial.println("Gate closed");
  //Anything below 4 have caused a servo to behave erroneously
  gateServo.write(175);
}

void setup() {
  gateServo.attach(gatePin);
  closeGate();
  Serial.begin(115200);
  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(ENABLE,OUTPUT);
  pinMode(STEP,OUTPUT);
  pinMode(DIR,OUTPUT);
  digitalWrite(ENABLE,LOW);
  pinMode(irPin, INPUT);
  pinMode(switchPin, INPUT);
  diskSetup();  
  
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
}
