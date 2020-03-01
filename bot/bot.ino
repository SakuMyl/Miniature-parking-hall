#include <WiFi.h>
#include <ESP32Servo.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Initialize Wifi connection to the router
char ssid[] = "aalto open";     // your network SSID (name)
char password[] = ""; // your network key

// Initialize Telegram BOT
#define BOTtoken "991870882:AAHpEmLLp6IxCeFNGvLeExnWgh_dtDAnX8E"  // your Bot Token (Get from Botfather)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;
bool parkOrLeaveInProcess = false;

Servo gateServo;
const int gatePin = 14;

int parked = 0;
const int spots = 3;
String cars[spots];
String chatIds[spots];

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

void leave(String plateNumber, String chatId) {
  int spot = getSpotFor(plateNumber, chatId);
  if (spot < 0) {
    bot.sendMessage(chatId, "Either car with plate number " + plateNumber + " is not parked or the car isn't yours.", "");
  } else {
    parkOrLeaveInProcess = true;
    openGate();
    cars[spot] = "";
    chatIds[spot] = "";
    bot.sendMessage(chatId, "You may leave.", "");
    //Mock the delay of the user leaving
    delay(2000);
    parked--;
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
  for (int i = 0; i < spots; i++) {
    if (cars[i] == "") {
      cars[i] = plateNumber;
      chatIds[i] = chatId;
      break;
    }
  }
  //Temporary mock implementation
  rotateDisk();
  openGate();
  bot.sendMessage(chatId, plateNumber + ", you may go in.", "");
  //Mock user going in
  delay(2000);
  closeGate();
  parked++;
  parkOrLeaveInProcess = false;
}

void rotateDisk() {
  return;
}

void openGate() {
  gateServo.write(90);
}

void closeGate() {
  //Anything below 4 have caused a servo to behave erroneously
  gateServo.write(4);
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
