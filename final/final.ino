#include "secrets.h"
#include "ThingSpeak.h"
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <SFE_BMP180.h>
#include <Wire.h>

#define BUILTIN_LED1 D4 //GPIO2
#define BUILTIN_LED2 D0 //GPIO16
#define DHTpin D5       //GPIO14
#define LDR_PIN A0      //

#define ALTITUDE 1655.0 // 1655m (Ciudad de Guatemala)
#define MIN_ADC_PARAMETER 309

const char *SENSOR_LABEL = "Sensor 2";

char ssid[] = SECRET_SSID;   // network SSID (name)
char pass[] = SECRET_PASS;   // network password

long sensorStationChannelNumber = SECRET_CH_ID;
char thingSpeakApiKey[] = SECRET_WRITE_APIKEY;

unsigned int temperatureFieldNumber = 5;      // Field 1
unsigned int absolutePressureFieldNumber = 6; // Field 2
unsigned int humidityFieldNumber = 7;         // Field 4
unsigned int luminosityAdcFieldNumber = 8;    // Field 5

DHTesp dht;
SFE_BMP180 bmp180;
WiFiClient client;

float humidity;

void setup() {
  Serial.begin(115200);

  if (bmp180.begin()) {
    Serial.println("bmp180 iniciado exitosamente");
  } else {
    Serial.println("bmp180 fallo al iniciar");
  }

  dht.setup(DHTpin, DHTesp::AM2302);
  pinMode(BUILTIN_LED2, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);  // Inicializacion ThingSpeak

}

void loop() {
  char status;
  double temperature, absolutePressure, p0, altitudeByPressureReading;
  float ldrSensorValue,luminosity;

  connectToWiFi();

  ldrSensorValue = analogRead(LDR_PIN);
  Serial.print("Valor LDR:"); Serial.println(ldrSensorValue);
  luminosity = measureBrightness(ldrSensorValue);
  ThingSpeak.setField(luminosityAdcFieldNumber, luminosity);
  
  
  status = bmp180.startTemperature();
  if (status != 0)
  {
    delay(status);

    status = bmp180.getTemperature(temperature);
    if (status != 0)
    {
      Serial.print("temperatura: ");
      Serial.print(temperature,2);
      Serial.print(" C, ");
      
      ThingSpeak.setField(temperatureFieldNumber, (float)temperature);

      status = bmp180.startPressure(3);
      if (status != 0)
      {
        delay(status);

        status = bmp180.getPressure(absolutePressure,temperature);
        if (status != 0)
        {
          ThingSpeak.setField(absolutePressureFieldNumber, (float)absolutePressure);
          
          Serial.print("presion absoluta: ");
          Serial.print(absolutePressure,2);
          Serial.print(" mb, ");
          Serial.print(absolutePressure*0.0295333727,2);
          Serial.println(" inHg");

          p0 = bmp180.sealevel(absolutePressure,ALTITUDE);
          Serial.print("presion relativa (nivel del mar): ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");

          altitudeByPressureReading = bmp180.altitude(absolutePressure,p0);
          ThingSpeak.setElevation((int)altitudeByPressureReading);
          Serial.print("altitud calculada: ");
          Serial.print(altitudeByPressureReading,0);
          Serial.print(" metros, ");

        } // pressure.getPressure(P, T)
      } // pressure.startPressure(3)
    } // pressure.getTemperature(T)
  } // pressure.startTemperature

  delay(dht.getMinimumSamplingPeriod());
 
  humidity = dht.getHumidity();

  ThingSpeak.setField(humidityFieldNumber, (float)humidity);

  Serial.print("humedad:");
  Serial.println(humidity);

  ThingSpeak.setStatus(SENSOR_LABEL);

  int httpCode = ThingSpeak.writeFields(sensorStationChannelNumber, thingSpeakApiKey);

  if (httpCode == 200) {
    Serial.println("Escritura al canal ThingSpeak exitosa.");
  }
  else {
    Serial.println("Problema escribiendo al canal. HTTP error code " + String(httpCode));
  }
  
  delay(60000);
}

void connectToWiFi() {
  // Conectar o reconectar a WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Intentando conectar a SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\Conectado.");
    digitalWrite(BUILTIN_LED2, HIGH);
  }
}

// medir flujo de luminosidad por unidad de area
// 1 lx = 1 lumen por metro cuadrado
float measureBrightness(int photocellReading) {
  
  if (photocellReading < 10) {
    Serial.println("Ambiente Oscuro");
  } else if (photocellReading < 200) {
    Serial.println("Ambiente Tenue");
  } else if (photocellReading < 500) {
    Serial.println("Ambiente Claro");
  } else if (photocellReading < 800) {
    Serial.println("Ambiente Luminoso");
  } else {
    Serial.println("Ambiente Muy Luminoso");
  }

  float lux=0.00;
  if (photocellReading > MIN_ADC_PARAMETER) {
    lux = 10.8 * photocellReading - 3052.000000;
  }
  Serial.print("Lux:");Serial.println(lux);

  return lux;
}
