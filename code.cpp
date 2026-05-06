#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// --- Arduino AP credentials ---
const char* ap_ssid     = "IoT-Sensor";
const char* ap_password = "123456789";

// --- Laptop IP on the Arduino hotspot ---
const char* serverURL = "http://192.168.4.2/dasbo/receive.php";
// --- LCD setup ---
LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- DHT setup ---
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Sensors ---
#define WATERPIN 34
#define MQPIN    35
#define LDRPIN   39

// --- Buzzer ---
#define BUZZERPIN 26

void setup() {
  Serial.begin(115200);
  Wire.begin(18, 19);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting AP...");

  // Start Arduino as Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);

  IPAddress apIP = WiFi.softAPIP();
  Serial.println("\nAP Started!");
  Serial.print("Arduino AP IP: ");
  Serial.println(apIP);  // Will print 192.168.4.1

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AP: IoT-Sensor");
  lcd.setCursor(0, 1);
  lcd.print("Pass:123456789");
  lcd.setCursor(0, 2);
  lcd.print("Arduino IP:");
  lcd.setCursor(0, 3);
  lcd.print(apIP);

  delay(3000);
  lcd.clear();

  dht.begin();
  pinMode(BUZZERPIN, OUTPUT);
  digitalWrite(BUZZERPIN, LOW);
}

void sendData(float temp, float hum, int gas, int waterPercent, int lightPercent) {
  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String payload = "temperature=" + String(temp, 1)
                 + "&humidity="    + String(hum, 1)
                 + "&gas="         + String(gas)
                 + "&water="       + String(waterPercent)
                 + "&light="       + String(lightPercent);

  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    Serial.print("HTTP Response: ");
    Serial.println(httpCode);
    Serial.println(http.getString());
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}

void loop() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  int waterRaw = analogRead(WATERPIN);
  int gas      = analogRead(MQPIN);

  int lightRaw = 0;
  for (int i = 0; i < 5; i++) {
    lightRaw += analogRead(LDRPIN);
    delay(5);
  }
  lightRaw /= 5;

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT Read Error!");
    temp = 0;
    hum  = 0;
  }

  int lightPercent = map(lightRaw, 2500, 0, 0, 100);
  lightPercent = constrain(lightPercent, 0, 100);

  int waterPercent = map(waterRaw, 0, 4095, 0, 100);
  waterPercent = constrain(waterPercent, 0, 100);

  String gasStatus;
  if (gas > 2500)      gasStatus = "HIGH!";
  else if (gas > 1500) gasStatus = "MED";
  else                 gasStatus = "LOW";

  if (temp >= 30 || gas > 2500) {
    digitalWrite(BUZZERPIN, HIGH);
  } else {
    digitalWrite(BUZZERPIN, LOW);
  }

  // --- LCD ---
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temp); lcd.print("C H:"); lcd.print(hum); lcd.print("%   ");
  lcd.setCursor(0, 1);
  lcd.print("Gas:"); lcd.print(gas); lcd.print(" "); lcd.print(gasStatus); lcd.print("   ");
  lcd.setCursor(0, 2);
  lcd.print("Water:"); lcd.print(waterPercent); lcd.print("%   ");
  lcd.setCursor(0, 3);
  lcd.print("Light:"); lcd.print(lightPercent); lcd.print("%   ");

  // --- Serial ---
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" | Hum: "); Serial.print(hum);
  Serial.print(" | Gas: "); Serial.print(gas);
  Serial.print(" | Water: "); Serial.print(waterPercent);
  Serial.print("% | Light: "); Serial.print(lightPercent); Serial.println("%");

  // --- Send to server ---
  sendData(temp, hum, gas, waterPercent, lightPercent);

  delay(2000);
}