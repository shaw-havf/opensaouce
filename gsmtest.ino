//7 to TXD and 8 to RXD
#include <SoftwareSerial.h>
SoftwareSerial GPRS(7,8);
unsigned char buffer[64];
int count=0;

void setup() {
GPRS.begin(9600);
Serial.begin(9600);
}

void loop() {
 while(GPRS.available())
 {
  buffer[count++]=GPRS.read();
  if(count==64) break;
  
 }

 Serial.write(buffer,count);
 clearBufferArray();
 count=0;

 if (Serial.available()) {
  byte b=Serial.read();
  if (b=='^')
  GPRS.write( 0x1a );
  else
  GPRS.write(b);
 }
}

void clearBufferArray()
{
  for (int i=0; i<count;i++)
  {
    buffer[i]=NULL;
  }
}
