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

// Define OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define sensor pins
#define DHT_PIN 12
#define DHT_TYPE DHT11
#define MQ135_PIN 36
#define MQ2_PIN 39

// Initialize sensors
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP085_Unified bmp;

// Sensor thresholds for air quality evaluation
const int CO2_GOOD = 550;
const int CO2_MODERATE = 850;
const int GAS_LIGHT = 1500;
const int GAS_HEAVY = 2000;

float temperature, humidity, pressure;
int mq135_value, mq2_value;

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Connecting to WiFi...");
  display.display();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.clearDisplay();
  display.println("WiFi connected!");
  display.display();

  // Initialize sensors
  dht.begin();
  if (!bmp.begin()) {
    Serial.println("BMP180 sensor not found!");
    while (true);
  }
}

void loop() {
  // Read sensors
  readDHT();
  readBMP();
  readGasSensors();

  // Display sensor readings
  displayReadings();

  delay(2000);  // Delay before next loop iteration
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
  // Clear OLED display
  display.clearDisplay();

  // Display temperature, humidity, and pressure
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.print(temperature);
  display.println(" C");

  display.setCursor(0, 10);
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");

  display.setCursor(0, 20);
  display.print("Pressure: ");
  display.print(pressure);
  display.println(" hPa");

  // Display gas sensor readings
  display.setCursor(0, 30);
  display.print("CO2: ");
  display.print(mq135_value);

  display.print("  Gas: ");
  display.print(mq2_value);

  // Evaluate air quality
  String airQuality = evaluateAirQuality();
  String gasStatus = evaluateGasStatus();

  // Display air quality and gas status
  display.setCursor(0, 40);
  display.print("Air Quality: ");
  display.println(airQuality);

  display.setCursor(0, 50);
  display.print("Gas Status: ");
  display.println(gasStatus);

  // Send buffer to the OLED
  display.display();
  // Upload data to ThingSpeak
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

  delay(2000);   // Wait 2 seconds before sending next data
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