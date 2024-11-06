#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#include <vector>

template <typename LectureType>
class SensorConnection {
public:
  enum class SensorType : uint8_t {
    Humidity = 1, Temperature = 2
  };

public:
  SensorConnection(const char *url, SensorType sensorType, unsigned long millisecondsBetweenLectures, unsigned long lecturesPerConnection) noexcept 
    : m_Url(url), m_SensorType(sensorType), m_MillisecondsBetweenLectures(millisecondsBetweenLectures), m_LecturesPerConnection(lecturesPerConnection)
  {
    m_Lectures.reserve(lecturesPerConnection);
  }

  template <typename LectureFunction>
  void Update(LectureFunction lectureFunction) {
    if (millis() - m_LastTime < m_MillisecondsBetweenLectures) {
      return;
    }
    m_LastTime = millis();

    LectureType lecture = lectureFunction();
    m_Lectures.push_back(lecture);
    TrySending();
  }

  private:
  void TrySending() {
    if (m_Lectures.size() < m_LecturesPerConnection) {
      return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient httpConnection;
    bool connectedSuccessfuly = httpConnection.begin(client, m_Url);

    if (!connectedSuccessfuly) {
      Serial.println("Hubo un error en la conexión HTTP");
      httpConnection.end();
      return;
    }

    String jsonData = "";
    jsonData += "{\n";
    jsonData += "\t\"microcontrollerId\": " + String("45") + ",\n";
    jsonData += "\t\"sensorType\": " + String(static_cast<uint8_t>(m_SensorType)) + ",\n";
    jsonData += "\t\"values\": [";
    for (size_t i = 0; i < m_Lectures.size(); ++i) {
        jsonData += String(m_Lectures[i]);
        if (i < m_Lectures.size() - 1) {
          jsonData += ", ";
        }
    }
    jsonData += "]\n";
    jsonData += "}";
    m_Lectures.clear();

    httpConnection.addHeader("Content-Type", "application/json");
    int responseCode = httpConnection.POST(jsonData);
    if (responseCode < 0) {
      Serial.println("ERROR - No se pudo realizar la petición POST");
    } else if (responseCode < 200 || responseCode > 299) {
      Serial.println("ERROR - Se regresó un código de error de la petición");
    }
    Serial.println("Envío éxitoso");
    Serial.println(jsonData);

    httpConnection.end();
  }

private:
  const char *m_Url;
  SensorType m_SensorType;
  unsigned long m_MillisecondsBetweenLectures;
  unsigned long m_LecturesPerConnection;

  unsigned long m_LastTime = 0;
  std::vector<LectureType> m_Lectures;
};
