#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085_U.h>
#include "DHT.h"

// WiFi credentials
const char* ssid = "WiFi Name";
const char* password = "WiFi Password";

// ThingSpeak details
const char* server = "http://api.thingspeak.com";
String apiKey = "Paste your API Key here";

// OLED Display Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C  // I2C address from scanner
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sensor Pins and Initialization
#define DHT_PIN 12
#define DHT_TYPE DHT11
#define MQ135_PIN 36
#define MQ2_PIN 39

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP085_Unified bmp;

// Sensor Thresholds for Air Quality Evaluation
const int CO2_GOOD = 550;
const int CO2_MODERATE = 850;
const int GAS_LIGHT = 1500;
const int GAS_HEAVY = 2000;

float temperature, humidity, pressure;
int mq135_value, mq2_value;

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Initialize I2C Communication
  Wire.begin(21, 22);  // Make sure your PCB uses these pins

  // Initialize OLED Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 allocation failed!");
    while (true);
  }

  Serial.println("OLED initialized successfully!");

  // Show Startup Message
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Hello!");
  display.display();
  delay(2000);

  // WiFi Connection
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected!");
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("WiFi Connected!");
  display.display();
  delay(2000);

  // Initialize Sensors
  dht.begin();
  if (!bmp.begin()) {
    Serial.println("BMP180 sensor not found!");
    while (true);
  }
}

void loop() {
  // Read Sensors
  readDHT();
  readBMP();
  readGasSensors();

  // Display Sensor Readings
  displayReadings();

  // Upload Data to ThingSpeak
  uploadData();

  delay(2000);  // Wait before next update
}

void readDHT() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void readBMP() {
  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure) {
    pressure = event.pressure / 100.0F;  // Convert from Pa to hPa
  } else {
    Serial.println("Error reading pressure from BMP180 sensor");
  }
}

void readGasSensors() {
  mq135_value = analogRead(MQ135_PIN);
  mq2_value = analogRead(MQ2_PIN);
}

void displayReadings() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(temperature); display.println(" C");

  display.setCursor(0, 10);
  display.print("Humidity: "); display.print(humidity); display.println(" %");

  display.setCursor(0, 20);
  display.print("Pressure: "); display.print(pressure); display.println(" hPa");

  display.setCursor(0, 30);
  display.print("CO2: "); display.print(mq135_value);
  display.print("  Gas: "); display.print(mq2_value);

  // Air Quality Status
  display.setCursor(0, 40);
  display.print("Air Quality: "); display.println(evaluateAirQuality());

  // Gas Status
  display.setCursor(0, 50);
  display.print("Gas Status: "); display.println(evaluateGasStatus());

  display.display();
}

void uploadData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "/update?api_key=" + apiKey +
                 "&field1=" + String(temperature) +
                 "&field2=" + String(humidity) +
                 "&field3=" + String(pressure) +
                 "&field4=" + String(mq135_value) +
                 "&field5=" + String(mq2_value);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Data sent successfully");
    } else {
      Serial.println("Error sending data");
    }
    http.end();
  }
}

String evaluateAirQuality() {
  if (mq135_value < CO2_GOOD) {
    return "Good";
  } else if (mq135_value < CO2_MODERATE) {
    return "Moderate";
  } else {
    return "Poor";
  }
}

String evaluateGasStatus() {
  if (mq2_value > GAS_HEAVY) {
    return "Heavy Smoke";
  } else if (mq2_value > GAS_LIGHT) {
    return "Light Smoke";
  } else {
    return "Normal";
  }
}
