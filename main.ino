#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";
const char *serverUrl = "example.com"; 
const int serverPort = 80;

void setup() {
  Serial.begin(115200);
  setupDisplay();
  connectToWiFi();
  dht.begin();
}

void loop() {
  delay(2000); 
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isValidRead(humidity, temperature)) {
    Serial.printf("Humidity: %.1f %%, Temperature: %.1f *C\n", humidity, temperature);
    sendDataToServer(temperature, humidity);
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}

bool isValidRead(float humidity, float temperature) {
  return !isnan(humidity) && !isnan(temperature);
}

void sendDataToServer(float temperature, float humidity) {
  WiFiClient client;
  if (client.connect(serverUrl, serverPort)) {
    String jsonData = "{\"temperature\":" + String(temperature, 2) + ", \"humidity\":" + String(humidity, 2) + "}";

    client.println("POST /weather HTTP/1.1");
    client.println("Host: " + String(serverUrl));
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println("Content-Length: " + jsonData.length());
    client.println();
    client.print(jsonData);

    Serial.println("JSON data sent to server");
    readServerResponse(client);
    client.stop();
  } else {
    Serial.println("Connection to server failed");
  }
}

void readServerResponse(WiFiClient &client) {
  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.println(line);
    }
  }
}
