/*
 Connection:
 BH1750 -   Arduino 
 ------------------
 VCC    -   5v
 GND    -   GND
 SCL    -   SCL(Analog Pin 5)
 SDA    -   SDA(Analog Pin 4)
 ADD    -   NC (or GND)
 */
 
 
#include <Wire.h>

 
int BH1750address = 0x23;
byte buff[2];


void setup()
{
  Wire.begin();
  Serial.begin(9600);
   pinMode(9, OUTPUT);
pinMode(10, OUTPUT);
  Serial.print("  BH1750 Light  ");
  Serial.print("Intensity Sensor");
  delay(2000);
}
 
void loop()
{
    digitalWrite(9, HIGH);
  digitalWrite(10, LOW);
  int i;
  uint16_t value=0;
  BH1750_Init(BH1750address);
  delay(200);
 
  if(2==BH1750_Read(BH1750address))
  {
    value=((buff[0]<<8)|buff[1])/1.2;

    
    Serial.print(value);     
    Serial.println("[lux]"); 
  }
  delay(150);
}
 
int BH1750_Read(int address) 
{
  int i=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()) 
  {
    buff[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();  
  return i;
}
 
void BH1750_Init(int address) 
{
  Wire.beginTransmission(address);
  Wire.write(0x10);
  Wire.endTransmission();
}
