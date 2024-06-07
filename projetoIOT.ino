#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_APDS9960.h>

#define LDR_PIN A0
#define LED_PIN D6
#define LED_QNT 8
#define WIFI_LENGTH 3

const char* ssid[] = {"SENAC-Mesh", "Senac-Mesh", "Arctic Monkeys"}; //Enter SSID
const char* password[] = {"09080706", "09080706", "ityttmom0209"}; //Enter Password
String device_name = "IOTTeste";

using namespace websockets;

struct Color {
  int red;
  int green;
  int blue;
};

Adafruit_APDS9960 apds;
Adafruit_NeoPixel pixels(LED_QNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WebsocketsClient client;
/*
int red_val;
int green_val;
int blue_val;
*/
float brightness_pct = 1.0;
int room_luminosity = 0;
int selected_color = 0;
Color colors[4];

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

        colors[0].red = value[0];
        colors[0].green = value[1];
        colors[0].blue = value[2];
        selected_color = 0;

        /*
        red_val = value[0];
        green_val = value[1];
        blue_val = value[2];
        */

        changeLeds(requested_by, 1);
        //client.send(String("{\"action\":\"answerchangerequest\",\"data\":{\"controllerID\":\"" + requested_by + "\",\"confirmed\":true,\"attribute\":\"" + attribute + "\",\"value\":[" + red_val + "," + green_val + "," + blue_val + "]}}"));
      } else if (attribute.equals("power")) {
        int brightness = doc["value"];
        brightness_pct = brightness / 100.0;

        client.send(String("{\"action\":\"answerchangerequest\",\"data\":{\"controllerID\":\"" + requested_by + "\",\"confirmed\":true,\"attribute\":\"" + attribute + "\",\"value\":" + brightness + "}}"));
        changeLeds(requested_by, 0);
      }
    } else {
        JsonArray rgb = doc["rgb"];
        JsonArray rgb_history = doc["rgb_history"];

        colors[0].red = rgb[0];
        colors[0].green = rgb[1];
        colors[0].blue = rgb[2];
        
        colors[1].red = rgb_history[0][0];
        colors[1].green = rgb_history[0][1];
        colors[1].blue = rgb_history[0][2];

        colors[2].red = rgb_history[1][0];
        colors[2].green = rgb_history[1][1];
        colors[2].blue = rgb_history[1][2];

        colors[3].red = rgb_history[2][0];
        colors[3].green = rgb_history[2][1];
        colors[3].blue = rgb_history[2][2];

        /*
        red_val = rgb[0];
        green_val = rgb[1];
        blue_val = rgb[2];
        */
    }

    Serial.print("Red: ");
    //Serial.println(red_val);
    Serial.println(colors[0].red);
    Serial.print("Green: ");
    //Serial.println(green_val);
    Serial.println(colors[0].green);
    Serial.print("Blue: ");
    //Serial.println(blue_val);
    Serial.println(colors[0].blue);
    Serial.print("brightness: ");
    Serial.print(brightness_pct);
    Serial.println("%");

}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
        
        client.send(String("{\"action\":\"boardstatus\",\"data\":{\"boardID\":\"" + WiFi.macAddress() + "\"}}"));
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    }
}

void changeLeds(String requested_by, int send_update) {
  Serial.println("Changing LEDs");
  Serial.println(requested_by);

  for (int i = 0; i < LED_QNT; i++) {
    pixels.setPixelColor(i, pixels.Color((int) (brightness_pct * colors[selected_color].red), (int) (brightness_pct * colors[selected_color].green), (int) (brightness_pct * colors[selected_color].blue)));

    pixels.show();
  }

  if (send_update) client.send(String("{\"action\":\"answerchangerequest\",\"data\":{\"controllerID\":\"" + requested_by + "\",\"confirmed\":true,\"attribute\":\"rgb\",\"value\":[" + colors[selected_color].red + "," + colors[selected_color].green + "," + colors[selected_color].blue + "]}}"));
}

void handleGesture() {
    switch ( apds.readGesture() ) {
      case APDS9960_UP:
        if ((brightness_pct + 5.0) < 100.0) brightness_pct += 5.0;
        else brightness_pct = 100.0;

        changeLeds(String("gesture"), 1);

        Serial.println("UP");
        break;
      case APDS9960_DOWN:
        if ((brightness_pct - 5.0) > 0.0) brightness_pct -= 5.0;
        else brightness_pct = 0.0;

        changeLeds(String("gesture"), 1);

        Serial.println("DOWN");
        break;
      case APDS9960_LEFT:
        if (selected_color == 0) {
          selected_color = 3;
        } else {
          selected_color -= 1;
        }

        changeLeds(String("gesture"), 1);

        Serial.println("LEFT");
        break;
      case APDS9960_RIGHT:
        if (selected_color == 3) {
          selected_color = 0;
        } else {
          selected_color += 1;
        }

        changeLeds(String("gesture"), 1);

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
    Serial.begin(115200);
    
    pinMode(LDR_PIN, INPUT);
    pixels.begin();

    String websockets_server = String("wss://gdddyr9xoe.execute-api.us-east-2.amazonaws.com/test/?type=board&ID=" + WiFi.macAddress() + "&name=" + device_name);

    for (int i = 0; i < 3; i++) {
      Serial.print("Meu identificador é: ");
      Serial.println(WiFi.macAddress());
      delay(600);
    }

    for (int j = 0; j < WIFI_LENGTH; j++) {
      WiFi.begin(ssid[j], password[j]);
    // Wait some time to connect to wifi
      for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
      }

      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
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
    pixels.clear();

    room_luminosity = analogRead(LDR_PIN);

    Serial.print("Room luminosity: ");
    Serial.println(room_luminosity);

    // Apparently this is blocking
    //handleGesture();

    delay(700);
}
