#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <Adafruit_APDS9960.h>

#define LDR_PIN A0
#define LED_PIN D6
#define LEDS_QNT 8

const char* ssid = "Arctic Monkeys"; //Enter SSID
const char* password = "ityttmom0209"; //Enter Password
String device_name = "IOTTeste";

using namespace websockets;

Adafruit_APDS9960 apds;
CRGB leds[LEDS_QNT];
WebsocketsClient client;
int red_val;
int green_val;
int blue_val;
float brightness_pct = 1.0;
int room_luminosity = 0;

void onMessageCallback(WebsocketsMessage message) {
    StaticJsonDocument<384> doc;

    Serial.println(message.data());

    DeserializationError err = deserializeJson(doc, message.data());

    if (err) {
      Serial.print("Deserialize error ");
      Serial.println(err.f_str());
      return;
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

        client.send(String("{\"action\":\"answerchangerequest\",\"data\":{\"controllerID\":\"" + requested_by + "\",\"confirmed\":true,\"attribute\":\"" + attribute + "\",\"value\":[" + red_val + "," + green_val + "," + blue_val + "]}}"));
      } else if (attribute.equals("power")) {
        int brightness = doc["value"];

        brightness_pct = (float) brightness / 100;

        client.send(String("{\"action\":\"answerchangerequest\",\"data\":{\"controllerID\":\"" + requested_by + "\",\"confirmed\":true,\"attribute\":\"" + attribute + "\",\"value\":" + brightness + "}}"));
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
    Serial.print("brightness: ");
    Serial.print(brightness_pct * 100);
    Serial.println("%");

    changeLeds();
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
        
        client.send(String("{\"action\":\"boardstatus\",\"data\":{\"boardID\":\"" + WiFi.macAddress() + "\"}}"));
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    }
}

void changeLeds() {
  Serial.println("Changing LEDs");

  for (int i = 0; i < LEDS_QNT; i++) {
    leds[i] = CRGB((int) (red_val * brightness_pct), (int) (green_val * brightness_pct), (int) (blue_val * brightness_pct));
  }

  FastLED.show();
}

void handleGesture() {
    switch ( apds.readGesture() ) {
      case APDS9960_UP:
        if (brightness_pct < 100) brightness_pct += 1.0;
        Serial.println("UP");
        break;
      case APDS9960_DOWN:
        if (brightness_pct > 0) brightness_pct -= 1.0;
        Serial.println("DOWN");
        break;
      case APDS9960_LEFT:
        Serial.println("LEFT");
        break;
      case APDS9960_RIGHT:
        Serial.println("RIGHT");
        break;
      /*
      case DIR_NEAR:
        Serial.println("NEAR");
        break;
      case DIR_FAR:
        Serial.println("FAR");
        break;
      */
      default:
        Serial.println("NONE");
    }
}

void setup() {
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LEDS_QNT);
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    pinMode(LDR_PIN, INPUT);

    String websockets_server = String("wss://gdddyr9xoe.execute-api.us-east-2.amazonaws.com/test/?type=board&ID=" + WiFi.macAddress() + "&name=" + device_name);

    for (int i = 0; i < 3; i++) {
      Serial.print("Meu identificador é: ");
      Serial.println(WiFi.macAddress());
      delay(600);
    }

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    if (apds.begin()) {
      Serial.println("Sensor de gestos inicializado.");
    } else {
      Serial.println("Erro ao iniciar sensor de gestos");
    }

    // Start running the APDS-9960 gesture sensor engine
    apds.enableGesture(true);

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

    // Apparently this is blocking
    //handleGesture();

    delay(900);
}
