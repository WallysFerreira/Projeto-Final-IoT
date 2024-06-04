#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define LDR_PIN A0
#define LED_PIN D6
#define LEDS_QNT 8

const char* ssid = "Arctic Monkeys"; //Enter SSID
const char* password = "ityttmom0209"; //Enter Password
const char* websockets_server = "wss://gdddyr9xoe.execute-api.us-east-2.amazonaws.com/test/?type=board&ID=esp8266&name=IOTTeste"; //server adress and port

using namespace websockets;

CRGB leds[LEDS_QNT];
WebsocketsClient client;
int red_val;
int green_val;
int blue_val;
int luminosity = 0;

void onMessageCallback(WebsocketsMessage message) {
    StaticJsonDocument<384> doc;

    Serial.println(message.data());

    DeserializationError err = deserializeJson(doc, message.data());

    if (err) {
      Serial.print("Deserialize error ");
      Serial.println(err.f_str());
    }

    String requested_by = doc["requestedBy"];
    String name = doc["name"];

    if (!requested_by.equals("null")) {
      String attribute = doc["attribute"];

      if (attribute.equals("rgb")) {
        JsonArray value = doc["value"];

        red_val = value[0];
        green_val = value[1];
        blue_val = value[2];
      }
    } else {
        JsonArray rgb = doc["rgb"];

        red_val = rgb[0];
        green_val = rgb[1];
        blue_val = rgb[2];
    }

    Serial.print("Red: ");
    Serial.println(red_val);
    Serial.print("Green: ");
    Serial.println(green_val);
    Serial.print("Blue: ");
    Serial.println(blue_val);

    changeLeds();
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
        client.send("{\"action\":\"boardstatus\",\"data\":{\"boardID\":\"esp8266\"}}");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    }
}

void changeLeds() {
  Serial.println("Changing LEDs");

  for (int i = 0; i < LEDS_QNT; i++) {
    leds[i] = CRGB(red_val, green_val, blue_val);
  }

  FastLED.show();
}

void setup() {
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LEDS_QNT);
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    pinMode(LDR_PIN, INPUT);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Setup Callbacks
    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);
    
    // Connect to server
    client.connect(websockets_server);
}

void loop() {
    client.poll();

    room_luminosity = analogRead(LDR_PIN);

    Serial.print("Room luminosity: ");
    Serial.println(room_luminosity);

    delay(750);
}
