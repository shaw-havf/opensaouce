/*
  This is a simple code to test BH1750FVI Light senosr
  communicate using I2C Protocol
  this library enable 2 slave device address
  Main address  0x23
  secondary address 0x5C
  connect this sensor as following :
  VCC >>> 3.3V
  SDA >>> A4
  SCL >>> A5
  addr >> A3
  Gnd >>>Gnd

  Written By : Mohannad Rawashdeh

*/

// First define the library :
#include "dht.h"
#include <Wire.h>
#include <BH1750FVI.h>
#define dht_apin A0 // Analog Pin sensor is connected to
dht DHT;
unsigned long time;
BH1750FVI LightSensor;

#include <DFRobot_sim808.h>
#include <SoftwareSerial.h>

#define MESSAGE_LENGTH 160
char message[MESSAGE_LENGTH];
int messageIndex = 0;
char MESSAGE[300];
char lat[12];
char lon[12];
char wspeed[12];

char phone[16];
char datetime[24];

#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX,PIN_RX);
DFRobot_SIM808 sim808(&mySerial);//Connect RX,TX,PWR,


void setup() {   // put your setup code here, to run once:
  Serial.begin(9600);
  LightSensor.begin();

  /*
    Set the address for this sensor
    you can use 2 different address
    Device_Address_H "0x5C"
    Device_Address_L "0x23"
    you must connect Addr pin to A3 .
  */
  LightSensor.SetAddress(Device_Address_H);//Address 0x5C
  // To adjust the slave on other address , uncomment this line
  // lightMeter.SetAddress(Device_Address_L); //Address 0x5C
  //-----------------------------------------------
  /*
    set the Working Mode for this sensor
    Select the following Mode:
    Continuous_H_resolution_Mode
    Continuous_H_resolution_Mode2
    Continuous_L_resolution_Mode
    OneTime_H_resolution_Mode
    OneTime_H_resolution_Mode2
    OneTime_L_resolution_Mode

    The data sheet recommanded To use Continuous_H_resolution_Mode
  */

  LightSensor.SetMode(Continuous_H_resolution_Mode);

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
}


void loop() {
  // put your main code here, to run repeatedly:
  time = millis();
  
  DHT.read11(dht_apin);
  uint16_t lux = LightSensor.GetLightIntensity();// Get Lux value

  delay(2000);//Wait 5 seconds before accessing sensor again.
   //*********** Detecting unread SMS ************************
   messageIndex = sim808.isSMSunread();

   //*********** At least, there is one UNREAD SMS ***********
   if (messageIndex > 0)
   { 
      Serial.print("messageIndex: ");
      Serial.println(messageIndex);
      
      sim808.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);
                 
      //***********In order not to full SIM Memory, is better to delete it**********
      sim808.deleteSMS(messageIndex);
      Serial.print("From number: ");
      Serial.println(phone);  
      Serial.print("Datetime: ");
      Serial.println(datetime);        
      Serial.print("Recieved Message: ");
      Serial.println(message); 

    while(!sim808.getGPS())
    {
      
    }

   
      Serial.print(sim808.GPSdata.year);
      Serial.print("/");
      Serial.print(sim808.GPSdata.month);
      Serial.print("/");
      Serial.print(sim808.GPSdata.day);
      Serial.print(" ");
      Serial.print(sim808.GPSdata.hour);
      Serial.print(":");
      Serial.print(sim808.GPSdata.minute);
      Serial.print(":");
      Serial.print(sim808.GPSdata.second);
      Serial.print(":");
      Serial.println(sim808.GPSdata.centisecond);
      Serial.print("latitude :");
      Serial.println(sim808.GPSdata.lat);
      Serial.print("longitude :");
      Serial.println(sim808.GPSdata.lon);
      Serial.print("speed_kph :");
      Serial.println(sim808.GPSdata.speed_kph);
      Serial.print("heading :");
      Serial.println(sim808.GPSdata.heading);
      Serial.println();
  
      float la = sim808.GPSdata.lat;
      float lo = sim808.GPSdata.lon;
      float ws = sim808.GPSdata.speed_kph;
  
      dtostrf(la, 6, 2, lat); //put float value of la into char array of lat. 6 = number of digits before decimal sign. 2 = number of digits after the decimal sign.
      dtostrf(lo, 6, 2, lon); //put float value of lo into char array of lon
      dtostrf(ws, 6, 2, wspeed);  //put float value of ws into char array of wspeed
    
      sprintf(MESSAGE, "Latitude : %s\nLongitude : %s\nWind Speed : %s kph\nMy Module Is Working%s\n", lat, lon, wspeed, lat, lon,DHT.humidity,DHT.temperature,lux  );
      
  
       Serial.println("Sim808 init success");
       Serial.println("Start to send message ...");
    
       Serial.println(MESSAGE);
       Serial.println(phone);
  
       sim808.sendSMS(phone,MESSAGE);
  
      //************* Turn off the GPS power ************
      sim808.detachGPS();
  
      
   }
}
