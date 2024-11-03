#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6BmmWiV52"
#define BLYNK_TEMPLATE_NAME "Voice Control"
#define BLYNK_AUTH_TOKEN "eYT_u2rEpL9fWYCDu-CLNT_9f_Up4XK-"
#define DHTTYPE DHT22
#define DHTPIN D5 
#define LINE_TOKEN "XgusLFhzr14nHeZuanKpTBJrnI6AdoHNtQvCgvoFiBi"

#include "DHT.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <BH1750FVI.h>
#include <ESP8266HTTPClient.h>
// Your WiFi credentials.
// Set password to "" for open networks.
BH1750FVI myBH1750(BH1750_DEFAULT_I2CADDR, BH1750_ONE_TIME_HIGH_RES_MODE_2, BH1750_SENSITIVITY_DEFAULT, BH1750_ACCURACY_DEFAULT);
WiFiClientSecure client;
HTTPClient http;
String GAS_ID = "AKfycbzEgLvw-4jBOfo8I827HnsCMLRle31Ef3lGtTdOXNIRSOI5V2kuCvo6DyPHv9usKCWc";
const char* host = "script.google.com";
const int httpsPort = 443;
char ssid[] = "myguyisguy";
char pass[] = "godguy2004";
int led = D3;
int fan = D4;
int motionSensorPin = D6;
int motionVal = 0;
int led_status = 0;
int fan_status = 0;
const int update = 1;
const int wait = 2;
const int sensor = 3;
int state = 0;
uint32_t sleepTimer;
DHT dht(DHTPIN, DHTTYPE);
float t,h;
int mode = 1;  // 0 = Sensor Mode, 1 = Voice Control Mode
int pirState = LOW;  
unsigned long lastMotionCheck = 0;  // เวลาในการเช็คครั้งล่าสุด
unsigned long motionCheckInterval = 5000;  // เช็คการเคลื่อนไหวทุก 5 วินาที
void sensorControl();
void sendLineNotification(String message);
void sendLineNotificationValue(String varName, float value);
void sendData(float value,float value2,String value3);
void setup()
{
  // Debug console
  pinMode(motionSensorPin, INPUT);
  pinMode(led,OUTPUT);
  pinMode(fan,OUTPUT);
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(V4,mode);
  state = wait;
  dht.begin();
  while (myBH1750.begin(D1,D2) != true)
  {
    Serial.println(F("ROHM BH1750FVI is not present"));    //(F()) saves string to flash & keeps dynamic memory free
    delay(5000);
  }
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(F("ROHM BH1750FVI is present"));
  myBH1750.setCalibration(1.15);                           //call before "readLightLevel()", 1.06=white LED & artifical sun
  myBH1750.setSensitivity(2);                           //call before "readLightLevel()", 1.00=no optical filter in front of the sensor
  myBH1750.setResolution(BH1750_CONTINUOUS_HIGH_RES_MODE_2); //call before "readLightLevel()", continuous measurement with 1.00 lux resolution
  sleepTimer = millis();
  sendLineNotificationValue("System On",1);
  Blynk.virtualWrite(V0, 0);  // สั่งให้ LED ปิด
  digitalWrite(led, 0);
  led_status = 0;
  fan_status = 0;
  digitalWrite(fan, 1);
  Blynk.virtualWrite(V1, 0);
  client.setInsecure();                     
}

void loop()
{
  Blynk.run();

   if (mode == 0) {  // Sensor Mode
    unsigned long currentMillis = millis();
    
    // ตรวจสอบ Motion Sensor ทุกๆ motionCheckInterval (5 วินาทีในตัวอย่างนี้)
    if (currentMillis - lastMotionCheck >= motionCheckInterval) {
      lastMotionCheck = currentMillis;  // อัพเดทเวลาที่เช็คครั้งล่าสุด
      
      int motionDetected = digitalRead(motionSensorPin);  // อ่านค่าจาก Motion Sensor
      
      if (motionDetected == HIGH && pirState == LOW) {
        Serial.println("Motion detected! Starting sensor control.");
        pirState = HIGH;
        // เริ่มต้นการทำงานของเซ็นเซอร์ (LED, fan) เมื่อมีการเคลื่อนไหว
        sensorControl();  // เรียกฟังก์ชันสำหรับการควบคุมด้วยเซ็นเซอร์
        sendData(t,motionDetected,"SensorMode");
      } 
      else if (motionDetected == LOW && pirState == HIGH) {
        Serial.println("No motion detected.");
        Blynk.virtualWrite(V0, 0);  // สั่งให้ LED ปิด
        digitalWrite(led, 0);
        digitalWrite(fan, 1);
        Blynk.virtualWrite(V1, 0);
        led_status = 0;
        fan_status = 0;
        sendLineNotificationValue("LED OFF", led_status);
        sendLineNotificationValue("Fan OFF", fan_status);
        delay(3000);
        pirState = LOW;
      }
    }
  }else if (mode == 1) {  // Voice Control Mode
    if(state == wait){
      h = dht.readHumidity();
      t = dht.readTemperature();
      delay(3000);
      state = update;
    }
    else if(state == update) {
      Blynk.virtualWrite(V2, t);
      Blynk.virtualWrite(V3, h);
      sendData(t,0,"VoiceMode");
      delay(2000);
      state = wait;
    }
  }
}
void sensorControl() {
  h = dht.readHumidity();
  t = dht.readTemperature();
  float lux = myBH1750.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lux");
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  sendLineNotificationValue("Lux",lux);
  sendLineNotificationValue("Temp",t);
  Blynk.virtualWrite(V2, t);  // ส่งค่าอุณหภูมิไปยัง Blynk
  Blynk.virtualWrite(V3, h);  // ส่งค่าความชื้นไปยัง Blynk
  if (lux < 200) {  // ควบคุม LED ด้วยเซ็นเซอร์แสง
    Blynk.virtualWrite(V0, 1);  // สั่งให้ LED เปิด
    digitalWrite(led, 1);
    led_status = 1;
    Serial.println("LED ON");
    sendLineNotificationValue("LED ON", led_status);
  }
  else if (lux > 200) {  // ปิด LED ถ้าความสว่างเพียงพอ
    Blynk.virtualWrite(V0, 0);  // สั่งให้ LED ปิด
    digitalWrite(led, 0);
    led_status = 0;
    Serial.println("LED OFF");
    sendLineNotificationValue("LED OFF", led_status);
  }if (t <= 31) {  // ควบคุม fan ด้วยเซ็นเซอร์แสง
    Blynk.virtualWrite(V1, 0);  // สั่งให้ LED เปิด
    digitalWrite(fan, 1);
    fan_status = 0;
    Serial.println("Fan OFF");
    sendLineNotificationValue("Fan OFF", fan_status);
  }
  else if (t > 31) {  // ปิด fan ถ้าความสว่างเพียงพอ
    Blynk.virtualWrite(V1, 1);  // สั่งให้ LED ปิด
    digitalWrite(fan, 0);
    fan_status = 1;
    Serial.println("Fan ON");
    sendLineNotificationValue("Fan ON", fan_status);
  }
  state = update;
}
BLYNK_WRITE(V0)  // ปุ่มเปิด/ปิด LED ผ่านเสียง
{
  if (mode == 1) {  // เช็คว่าอยู่ในโหมด Voice Control
    int value = param.asInt();
    if (value == 1) {
      digitalWrite(led, 1);
      led_status = 1;
      Serial.println("Voice: LED ON");
      sendLineNotificationValue("LED ON", led_status);
    } else {
      digitalWrite(led, 0);
      led_status = 0;
      Serial.println("Voice: LED OFF");
      sendLineNotificationValue("LED OFF", led_status);
    }
  }
}
BLYNK_WRITE(V1)  // ปุ่มเปิด/ปิดพัดลมผ่านเสียง
{
  if (mode == 1) {
    int value = param.asInt();
    if (value == 1) {
      digitalWrite(fan, 0);
      fan_status = 1;
      Serial.println("Voice: Fan ON");
      sendLineNotificationValue("Fan ON", fan_status);
    } else {
      digitalWrite(fan, 1);
      fan_status = 0;
      Serial.println("Voice: Fan OFF");
      sendLineNotificationValue("Fan OFF", fan_status);
    }
  }
}
BLYNK_WRITE(V4)  // ปุ่มสลับโหมดใน Blynk
{
  mode = param.asInt(); // รับค่าจากปุ่ม (0 = Sensor Mode, 1 = Voice Control Mode)
  if (mode == 0) {
    Serial.println("Sensor Mode");
    sendLineNotificationValue("Sensor Mode", mode);
  } else if (mode == 1) {
    Serial.println("Voice Control Mode");
    sendLineNotificationValue("Voice Control Mode", mode);
  }
}
void sendData(float value, float value2, String value3) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection Failed");
    return;
  }

  // Prepare and send data  
  float string_temp = value; 
  float string_motion = value2;
  String mode = value3;
  String url = "/macros/s/" + GAS_ID + "/exec?temp=" + string_temp + "&motion_detected=" + string_motion + "&mode=" + mode; // Send 3 variables 
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");

  // Wait for response
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successful!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  
  Serial.print("reply was: ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
}

void sendLineNotification(String message)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    client.setInsecure(); // สำหรับการเชื่อมต่อ HTTPS แบบไม่ตรวจสอบใบรับรอง
    http.begin(client, "https://notify-api.line.me/api/notify");
 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Bearer " + String(LINE_TOKEN));
 
    String payload = "message=" + message;
    int httpResponseCode = http.POST(payload);
 
    if (httpResponseCode > 0)
    {
      Serial.print("LINE Notify Response Code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error sending LINE Notify: ");
      Serial.println(httpResponseCode);
    }
 
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}

void sendLineNotificationValue(String varName, float value)
{
  String strValue = String(value); // แปลงจาก int เป็น String
  {
    sendLineNotification(varName + " = " + strValue);
  }
}