#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>

#define NETWORK_SSID_1      "..."
#define NETWORK_PASSWORD_1  "..."
#define NETWORK_SSID_2      "..."
#define NETWORK_PASSWORD_2  "..."
#define NETWORK_SSID_3      "..."
#define NETWORK_PASSWORD_3  "..."
#define SERVER_HOST         "..."
#define SERVER_PORT         ...
#define STATUS_TOPIC        "home/garage/door/status"
#define SWITCH_TOPIC        "home/garage/door/switch"
#define STATUS_PIN           D4
#define FORCE_PIN            D3

int state = 0;
bool triggered = false;
bool forceTrigger = false;

WiFiClient WiFiClient;
ESP8266WiFiMulti wifiMulti;
PubSubClient client(WiFiClient);

void setup() {
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(FORCE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FORCE_PIN), forceInterrupt, FALLING);
  
  Serial.begin(115200);
  delay(100);
  Serial.println("HELLO");
  Serial.println("Setting up wifi list");
  wifiMulti.addAP(NETWORK_SSID_1, NETWORK_PASSWORD_1);
  wifiMulti.addAP(NETWORK_SSID_2, NETWORK_PASSWORD_2);
  wifiMulti.addAP(NETWORK_SSID_3, NETWORK_PASSWORD_3);

  client.setServer(SERVER_HOST, SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  delay(1000);

  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.print("Connected to: ");
    Serial.println(WiFi.SSID());
    connectMQTT();
    client.loop();
    
    if (forceTrigger) {
      client.publish(SWITCH_TOPIC, "1", true);
      forceTrigger = false;
    }
  } else {
    triggered = false;
    Serial.println("Trying to connect...");
  }

  if (state) {
    digitalWrite(STATUS_PIN, LOW);
  } else {
    digitalWrite(STATUS_PIN, HIGH);
  }
  Serial.println(".");
}

void callback(char* topic, byte* data, unsigned int length) {
  char charData[length+1];
  
  for (int i=0;i<length;i++) {
    charData[i] = (char)data[i];
  }
  
  state = atoi(charData);

  Serial.println(state);

  if (!state && !triggered) {
    client.publish(SWITCH_TOPIC, "1", true);
  }
  
  triggered = true;
}

void connectMQTT() {
  if (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("")) {
      Serial.println("connected");
      client.subscribe(STATUS_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

ICACHE_RAM_ATTR void forceInterrupt() {
  forceTrigger = true;
}
  
