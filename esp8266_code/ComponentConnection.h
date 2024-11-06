#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#include <vector>

class ComponentConnection {
public:
  enum class ComponentType {
    WaterPump = 1
  };
public:
  ComponentConnection(const char *url, ComponentType componentType, unsigned long millisecondsBetweenReadings) noexcept 
    : m_Url(url), m_ComponentType(componentType), m_MillisecondsBetweenReadings(millisecondsBetweenReadings)
  {}

  bool Update(int &value) {
    if (millis() - m_LastTime < m_MillisecondsBetweenReadings){
      return false;
    }
    m_LastTime = millis();

    return GetValue(value);
  }

private:
  bool GetValue(int &value) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient httpConnection;
    bool connectedSuccessfuly = httpConnection.begin(client, m_Url);

    if (!connectedSuccessfuly) {
      Serial.println("Hubo un error en la conexión HTTP");
      httpConnection.end();
      return false;
    }

    int codigoDeRespuesta = httpConnection.GET();
    if (codigoDeRespuesta < 0) {
      Serial.println("ERROR - No se pudo realizar la petición GET");
      return false;
    } else if (codigoDeRespuesta < 200 || codigoDeRespuesta > 299) {
      Serial.println("ERROR - La petición GET obtuvo un código de respuesta negativo");
      return false;
    }
    Serial.println("Lectura éxitosa");

    String res = httpConnection.getString();
    int startIndex = res.indexOf(KEY_TO_FIND) + KEY_TO_FIND.length() + 1;
    int endIndex = res.indexOf("\n", startIndex);
    value = res.substring(startIndex, endIndex).toInt();
  
    httpConnection.end();
    return true;
  }

private:
  inline static const String KEY_TO_FIND = String("\"value\": ");

  const char *m_Url;
  ComponentType m_ComponentType;
  unsigned long m_MillisecondsBetweenReadings;

  unsigned long m_LastTime = 0;
};
