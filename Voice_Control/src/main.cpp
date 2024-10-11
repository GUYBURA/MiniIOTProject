#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6BmmWiV52"
#define BLYNK_TEMPLATE_NAME "Voice Control"
#define BLYNK_AUTH_TOKEN "eYT_u2rEpL9fWYCDu-CLNT_9f_Up4XK-"
#define DHTTYPE DHT22
#define DHTPIN D5 

#include "DHT.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <BH1750FVI.h>
// Your WiFi credentials.
// Set password to "" for open networks.
BH1750FVI myBH1750(BH1750_DEFAULT_I2CADDR, BH1750_ONE_TIME_HIGH_RES_MODE_2, BH1750_SENSITIVITY_DEFAULT, BH1750_ACCURACY_DEFAULT);
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
int mode = 0;  // 0 = Sensor Mode, 1 = Voice Control Mode
int pirState = LOW;  
unsigned long lastMotionCheck = 0;  // เวลาในการเช็คครั้งล่าสุด
unsigned long motionCheckInterval = 5000;  // เช็คการเคลื่อนไหวทุก 5 วินาที
void sensorControl();
void setup()
{
  // Debug console
  pinMode(motionSensorPin, INPUT);
  pinMode(led,OUTPUT);
  pinMode(fan,OUTPUT);
  digitalWrite(LED_BUILTIN,1);
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
  
  Serial.println(F("ROHM BH1750FVI is present"));
  myBH1750.setCalibration(1.15);                           //call before "readLightLevel()", 1.06=white LED & artifical sun
  myBH1750.setSensitivity(2);                           //call before "readLightLevel()", 1.00=no optical filter in front of the sensor
  myBH1750.setResolution(BH1750_CONTINUOUS_HIGH_RES_MODE_2); //call before "readLightLevel()", continuous measurement with 1.00 lux resolution
  sleepTimer = millis();                                   
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
      } 
      else if (motionDetected == LOW && pirState == HIGH) {
        Serial.println("No motion detected.");
        pirState = LOW;
        Blynk.virtualWrite(V0, 0);  // สั่งให้ LED ปิด
        digitalWrite(led, 0);
        led_status = 0;
        digitalWrite(fan, 0);
        Blynk.virtualWrite(V1, 0);
      }
    }
  }else if (mode == 1) {  // Voice Control Mode
    if(state == wait){
      h = dht.readHumidity();
      t = dht.readTemperature();
      delay(5000);
      state = update;
    }
    else if(state == update) {
      Blynk.virtualWrite(V2, t);
      Blynk.virtualWrite(V3, h);
      delay(5000);
      state = wait;
    }
  }
}
void sensorControl() {
  if(state == wait) {
    state = sensor;
  } else if (state == sensor) {
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

    if (lux < 50) {  // ควบคุม LED ด้วยเซ็นเซอร์แสง
      Blynk.virtualWrite(V0, 1);  // สั่งให้ LED เปิด
      digitalWrite(led, 1);
      led_status = 1;
      Serial.println("LED ON");
      state = update;
    }
    if (lux > 50) {  // ปิด LED ถ้าความสว่างเพียงพอ
      Blynk.virtualWrite(V0, 0);  // สั่งให้ LED ปิด
      digitalWrite(led, 0);
      led_status = 0;
      Serial.println("LED OFF");
      state = update;
    }
  } else if (state == update) {
    Blynk.virtualWrite(V2, t);  // ส่งค่าอุณหภูมิไปยัง Blynk
    Blynk.virtualWrite(V3, h);  // ส่งค่าความชื้นไปยัง Blynk
    delay(5000);
    state = wait;
  }
}
BLYNK_WRITE(V0)  // ปุ่มเปิด/ปิด LED ผ่านเสียง
{
  if (mode == 1) {  // เช็คว่าอยู่ในโหมด Voice Control
    int value = param.asInt();
    if (value == 1) {
      digitalWrite(led, 1);
      led_status = 1;
      Serial.println("Voice: LED ON");
    } else {
      digitalWrite(led, 0);
      led_status = 0;
      Serial.println("Voice: LED OFF");
    }
  }
}
BLYNK_WRITE(V1)  // ปุ่มเปิด/ปิดพัดลมผ่านเสียง
{
  if (mode == 1) {
    int value = param.asInt();
    if (value == 1) {
      digitalWrite(fan, 1);
      fan_status = 1;
      Serial.println("Voice: Fan ON");
    } else {
      digitalWrite(fan, 0);
      fan_status = 0;
      Serial.println("Voice: Fan OFF");
    }
  }
}
BLYNK_WRITE(V4)  // ปุ่มสลับโหมดใน Blynk
{
  mode = param.asInt(); // รับค่าจากปุ่ม (0 = Sensor Mode, 1 = Voice Control Mode)
  if (mode == 0) {
    Serial.println("Sensor Mode");
  } else if (mode == 1) {
    Serial.println("Voice Control Mode");
  }
}