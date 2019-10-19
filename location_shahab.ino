#include <DFRobot_sim808.h>
#include <SoftwareSerial.h>
#include "dht.h"
#include <Wire.h>
#include <BH1750FVI.h>
#define dht_apin A0 // Analog Pin sensor is connected to
dht DHT;

#define MESSAGE_LENGTH 160
char message[MESSAGE_LENGTH];
int messageIndex = 0;
char MESSAGE[300];
char lat[8];
char lon[8];
char hea[3];
char lux[5];
char hum[3];
char tem[3];
char phone[16];
char datetime[10];

#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX,PIN_RX);
DFRobot_SIM808 sim808(&mySerial);//Connect RX,TX,PWR,
BH1750FVI LightSensor;
void setup()
{
  mySerial.begin(9600);
  Serial.begin(9600);

  //******** Initialize sim808 module *************
  while(!sim808.init())
  {
      Serial.print("Sim808 init error\r\n");
      delay(1000);
  }
  delay(3000);

  if( sim808.attachGPS())
      Serial.println("Open the GPS power success");
  else 
      Serial.println("Open the GPS power failure");
      
  Serial.println("Init Success, please send SMS message to me!");
  LightSensor.SetAddress(Device_Address_H);
  LightSensor.SetMode(Continuous_H_resolution_Mode);
}

void loop()
{
  //*********** Detecting unread SMS ************************
   messageIndex = sim808.isSMSunread();
  DHT.read11(dht_apin);
  float lux = LightSensor.GetLightIntensity();
   //*********** At least, there is one UNREAD SMS ***********
   if (messageIndex > 0)
   { 
      Serial.print("messageIndex: ");
      Serial.println(messageIndex);
      
      sim808.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);
                 
      //***********In order not to full SIM Memory, is better to delete it**********
      sim808.deleteSMS(messageIndex);


    while(!sim808.getGPS())
    {
      
    }

   
      Serial.print(sim808.GPSdata.year);
      Serial.print("/");
      Serial.print(sim808.GPSdata.month);
      Serial.print("/");
      Serial.print(sim808.GPSdata.day);
      Serial.print(" ");


  


      dtostrf(sim808.GPSdata.lat, 6, 2, lat); //put float value of la into char array of lat. 6 = number of digits before decimal sign. 2 = number of digits after the decimal sign.
      dtostrf(sim808.GPSdata.lon, 6, 2, lon); //put float value of lo into char array of lon
      dtostrf(sim808.GPSdata.heading, 3, 1, hea);
      //dtostrf(LightSensor.GetLightIntensity(), 3, 0, lux);
      dtostrf(DHT.humidity, 2, 0, hum);
      dtostrf(DHT.temperature, 3, 1, tem);
    
      sprintf(MESSAGE, lat, lon, hea,hum,tem);
      
  

       
    
  
       sim808.sendSMS(phone,MESSAGE);
  
      //************* Turn off the GPS power ************
      sim808.detachGPS();
  
      
   }
}
