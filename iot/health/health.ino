#define BLYNK_TEMPLATE_ID "TMPL3zeT4l8QO"
#define BLYNK_TEMPLATE_NAME "Heart Health Monitor"
#define BLYNK_AUTH_TOKEN "gjkFRTR5T-rmcj-RgFEbXyNALcRAK4Za"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/************ WIFI ************/
char ssid[] = "Altrin()";
char pass[] = "123456788";

/************ LCD ************/
LiquidCrystal_I2C lcd(0x27, 16, 2);

/************ TEMP ************/
#define ONE_WIRE_BUS D5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/************ ECG ************/
#define ECG_PIN A0
unsigned long lastBeat = 0;
int bpm = 0;

BlynkTimer timer;

/************ SENSOR READ ************/
void sendSensorData() {
  int ecgValue = analogRead(ECG_PIN);
  float ecgMetric = (ecgValue * 3.3) / 1023.0;

  if (ecgValue > 600) {
    unsigned long now = millis();
    if (now - lastBeat > 300) {
      bpm = 60000 / (now - lastBeat);
      lastBeat = now;
    }
  }

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  float tempF = (tempC * 9.0 / 5.0) + 32.0;

  // LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ECG:");
  lcd.print(ecgMetric, 2);
  lcd.print(" HR:");
  lcd.print(bpm);

  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(tempF, 1);
  lcd.print((char)223);
  lcd.print("F");

  // Serial
  Serial.print("Temp: ");
  Serial.print(tempF);
  Serial.print(" F | HR: ");
  Serial.print(bpm);
  Serial.print(" | ECG: ");
  Serial.println(ecgMetric);

  // Blynk virtual pins
  Blynk.virtualWrite(V1, tempF);      // Temperature
  Blynk.virtualWrite(V2, bpm);        // Heart Rate
  Blynk.virtualWrite(V3, ecgMetric);  // ECG
}

/************ SETUP ************/
void setup() {
  Serial.begin(9600);

  Wire.begin(D2, D1);
  lcd.init();
  lcd.backlight();

  sensors.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, sendSensorData);
}

/************ LOOP ************/
void loop() {
  Blynk.run();
  timer.run();
}
