#include "Connection.h"

const char *ssid = "INFINITUM9FAE";
const char *password = "XkTntfe4Cz";

// URLs to the AWS EC2 instance
const char *temperatureURL = "https://api.petche.shop/arduino/lecture";
const char *waterPumpURL = "https://api.petche.shop/arduino/state/45";

#include <DHT.h> 
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Temperature sensor connection that will send data every second and will send them in batches of three
SensorConnection<float> temperatureSensor(temperatureURL, SensorConnection<float>::SensorType::Humidity, 1 * 1000, 3);

// Waterpump connection that will get data from the database every second
ComponentConnection waterPump("https://api.petche.shop/arduino/state/45", ComponentConnection::ComponentType::WaterPump, 1 * 1000);

void setup() {
  delay(10);
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

  dht.begin();
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
    float temperature = dht.readTemperature();
    Serial.print("Temperatura leída: ");
    Serial.println(temperature);
    return temperature;
  });

  // Update waterpump
  int value;
  bool read = waterPump.Update(value);
  if (read) {
    Serial.print("Valor recibido de la base de datos: ");
    Serial.println(value);
  }

  delay(10);
}