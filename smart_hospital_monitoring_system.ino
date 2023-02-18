#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <MAX30100_PulseOximeter.h>
#include <HX711.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// ThingSpeak credentials
const char* server = "api.thingspeak.com";
const String writeAPIKey = "your_API_KEY";

// BME680 sensor
Adafruit_BME680 bme;

// MAX30100 sensor
MAX30100_PulseOximeter pox;

// Load cell
HX711 scale;

// LED pins
const int yellowLEDPin = 2;
const int redLEDPin = 3;
const int greenLEDPin = 4;

void setup() {
  Serial.begin(9600);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize ThingSpeak library
  ThingSpeak.begin(client);

  // Initialize BME680 sensor
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Initialize MAX30100 sensor
  if (!pox.begin()) {
    Serial.println("Could not find a valid MAX30100 sensor, check wiring!");
    while (1);
  }

  // Initialize load cell
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Initialize LED pins
  pinMode(yellowLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
}

void loop() {
  // Read sensor data
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0;

  // Print BME680 sensor data to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  // Read MAX30100 sensor data
  pox.update();
  int heartRate = pox.getHeartRate();
  int oxygenSaturation = pox.getSpO2();

  // Print MAX30100 sensor data to serial monitor
  Serial.print("Heart Rate: ");
  Serial.print(heartRate);
  Serial.println(" bpm");
  Serial.print("Oxygen Saturation: ");
  Serial.print(oxygenSaturation);
  Serial.println(" %");

  // Read load cell data
  float salineLevel = scale.get_units();

  // Print load cell data to serial monitor
  Serial.print("Saline Level: ");
  Serial.print(salineLevel);
  Serial.println(" ml");

  // Send sensor data to ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, pressure);
  ThingSpeak.setField(4, heartRate);
  ThingSpeak.setField(5, oxygenSaturation);
  ThingSpeak.setField(6, salineLevel);
  int statusCode = ThingSpeak.writeFields(writeAPIKey);
  Serial.print("ThingSpeak status code: ");
  Serial.println(statusCode);

// Determine saline level status and turn on corresponding LED
if (salineLevel < 200) {
digitalWrite(redLEDPin, HIGH);
digitalWrite(yellowLEDPin, LOW);
digitalWrite(greenLEDPin, LOW);
} else if (salineLevel < 400) {
digitalWrite(yellowLEDPin, HIGH);
digitalWrite(redLEDPin, LOW);
digitalWrite(greenLEDPin, LOW);
} else {
digitalWrite(greenLEDPin, HIGH);
digitalWrite(redLEDPin, LOW);
digitalWrite(yellowLEDPin, LOW);
}

delay(10000);
}
