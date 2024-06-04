#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>

const char* ssid = "Arctic Monkeys"; //Enter SSID
const char* password = "ityttmom0209"; //Enter Password
const char* websockets_server = "wss://gdddyr9xoe.execute-api.us-east-2.amazonaws.com/test/?type=board&ID=esp8266&name=IOTTeste"; //server adress and port

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

WebsocketsClient client;
void setup() {
    Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

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

    /*
    // Send a message
    client.send("Hi Server!");
    // Send a ping
    client.ping();
    */
}

void loop() {
    client.poll();
}
