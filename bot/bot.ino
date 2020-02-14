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

Servo gateServo;
const int gatePin = 14;

int parkissa = 0;
int paikkoja = 3;
String autot[paikkoja];

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/open") {
      openGate();
      bot.sendMessage(chat_id, "Gate opened, you may pass", "");
    }

    if (text == "/close") {
      closeGate();
      bot.sendMessage(chat_id, "Gate closed, YOU SHALL NOT PASS!", "");
    }

    if (text.startsWith("/park") {
      if (text.length() >= 10) {
        String rekisteri = text[7]+text[8]+text[9];
        if (parkissa < paikkoja) {
          
        }
      }
    }

  }
}

void openGate() {
  gateServo.write(90);
}

void closeGate() {
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
