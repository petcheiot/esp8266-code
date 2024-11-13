#include "SensorConnection.h"
#include "ComponentConnection.h"

const char *ssid = "INFINITUM9FAE";
const char *password = "XkTntfe4Cz";
// const char *ssid = "AZAEL";
// const char *password = "pinchemuertodehambre";

// URLs to the AWS EC2 instance
const char *lectureURL = "https://api.petche.shop/arduino/lecture";
const char *waterPumpURL = "https://api.petche.shop/arduino/state/45";

// DHT11 configuration
#include <DHT.h>
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Temperature sensor connection that will send data every second and will send them in batches of three
SensorConnection<float> temperatureSensor(lectureURL, SensorConnection<float>::SensorType::Temperature, 1 * 1000, 1);
float LastReadTemperature = 0.0f;

// Humidity sensor configuration
constexpr int HUMIDITY_PIN = A0;
constexpr float MAX_HUMIDITY = 1024;
constexpr float HUMIDITY_PERCENT_REQUIRED = 0.5f;

// Temperature sensor connection that will send data every second and will send them in batches of three
SensorConnection<float> humiditySensor(lectureURL, SensorConnection<float>::SensorType::Humidity, 1 * 1000, 1);
float LastReadHumidity = 0.0f;

// LCD I2C configuration
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Waterpump connection that will get data from the database every second
ComponentConnection waterPump(waterPumpURL, ComponentConnection::ComponentType::WaterPump, 1 * 1000);
constexpr int WATERPUMP_PIN = D4;
constexpr int ACTIVATION_VALUE = 2;

void PrintDataToLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(String(LastReadTemperature) + " Celcius");

  lcd.setCursor(0, 1);
  lcd.print("Humedad: " + String(LastReadHumidity) + "%");
}

void setup() {
  // Initialize the serial begin
  Serial.begin(9600);

  // Start a WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Conectado con éxito, mi IP es: ");
  Serial.println(WiFi.localIP());

  // Pin layout
  pinMode(WATERPUMP_PIN, OUTPUT);
  pinMode(HUMIDITY_PIN, INPUT);

  // Initialize DHT11
  dht.begin();

  // LCD I2C initialization
  lcd.init();
  lcd.backlight();

  // Extra initialization time
  delay(1000);
}

void loop() {
  // Only continue if there is a WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error en la conexión WIFI");
    return;
  }

  // Update temperature
  temperatureSensor.Update([]() {
    LastReadTemperature = dht.readTemperature();
    Serial.print("Temperatura leída: ");
    Serial.println(LastReadTemperature);

    PrintDataToLCD();

    return LastReadTemperature;
  });

  // Update humidity
  humiditySensor.Update([]() {
    LastReadHumidity = min(2.0f * (1.0f - static_cast<float>(analogRead(HUMIDITY_PIN)) / MAX_HUMIDITY), 1.0f) * 100.0f;
    Serial.print("Humedad leída: ");
    Serial.println(LastReadHumidity);

    PrintDataToLCD();

    return LastReadHumidity;  
  });

  // Update waterpump
  int value;
  bool read = waterPump.Update(value);
  if (read) {
    Serial.print("Valor recibido de la base de datos: ");
    Serial.println(value);

    // The relay works with an inverted value, LOW is on and HIGH is OFF
    digitalWrite(WATERPUMP_PIN, value == ACTIVATION_VALUE ? LOW : HIGH);
  }

  delay(10);
}
