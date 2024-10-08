#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6BmmWiV52"
#define BLYNK_TEMPLATE_NAME "Voice Control"
#define BLYNK_AUTH_TOKEN "eYT_u2rEpL9fWYCDu-CLNT_9f_Up4XK-"

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
int led_status = 0;
int fan_status = 0;
const int update = 1;
const int wait = 2;
const int sensor = 3;
int state = 0;
uint32_t sleepTimer;

void setup()
{
  // Debug console
  pinMode(led,OUTPUT);
  pinMode(fan,OUTPUT);
  digitalWrite(LED_BUILTIN,1);
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  state = wait;
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
  if(state == wait){
    state = sensor;
  }else if(state == sensor){
    float  lux = myBH1750.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lux");
    if (lux < 10) { //สามารถกำหนดค่าความสว่างตามต้องการได้
      Blynk.virtualWrite(V0, 1);  // สั่งให้ LED ติดสว่าง
      digitalWrite(led,1);
      Serial.println("LED ON");
      Serial.println();
    }
    if (lux > 10) { //สามารถกำหนดค่าความสว่างตามต้องการได้
      Blynk.virtualWrite(V0, 0); // สั่งให้ LED ดับ
      digitalWrite(led,0);
      Serial.println("LED OFF");
      Serial.println();
    }
    state = wait;
  }
}
BLYNK_WRITE(V0)
{   
  int value = param.asInt(); // Get value as integer
  if (value == 1){
    digitalWrite(led,1);
    led_status = 1;
  }else{
    digitalWrite(led,0);
    led_status = 0;
  }
}
BLYNK_WRITE(V1)
{   
  int value = param.asInt(); // Get value as integer
  if (value == 1){
    digitalWrite(fan,1);
    fan_status = 1;
  }else{
    digitalWrite(fan,0);
    fan_status = 0;
  }
}