

// First define the library :
#include "dht.h"
#include <Wire.h>
#include <BH1750FVI.h>
#define dht_apin A0 // Analog Pin sensor is connected to to measure temp and humidity
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
static const uint32_t GPSBaud = 4800;
SoftwareSerial GPRS(7,8);
TinyGPSPlus GPS;
int count=0;
int contentLength = 0;
char receivedStackIndex = 0;
char receivedStack[130];
const char *des = "$GPRMC";
enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT
};
//Write here you server and account data
const char smtp_server[ ] = "in-v3.mailjet.com";      // SMTP server
const char smtp_user_name[ ] = "1e636428a2d9e405a7fe5f2999a27f9f";   // SMTP user name e636428a2d9e
const char smtp_password[ ] = "f959bf1dc702585d1b9fe76d2565d7cb";    // SMTP password
const char smtp_port[ ] = "25";              // SMTP server port

//Write here you SIM card data 
const char apn[] = "internet.m2mportal.de";
const char user_name[] = "m2m";
const char password[] = "sim";

//Write here your information about sender, direcctions and names
const char sender_address[ ] = "siemensdatalogger@gmail.com";    // Sender address
const char sender_name[ ] = "Prototype_1";       // Sender name
const char to_address[ ] = "aghashahab24@gmail.com";        // Recipient address
const char to_name[ ] = "shahowb";           // Recipient name
const char subject[ ] = "Statues Report";
char body;
dht DHT;          //the variable of temprature and humidity 
BH1750FVI LightSensor; //the variable of illuminance
int8_t answer;
int onModulePin= 2;
char aux_str[129];
char cutemp[6];
char cuhum[6];
char cuilum[7];
char buffer[80];
byte pos = 0;
byte parseState = PS_DETECT_MSG_TYPE;


 void sendGSM(const char* msg, int waitMs = 500) {
  GPRS.println(msg);
  delay(waitMs);
  while(GPRS.available()) {
    parseATText(GPRS.read());
  }
}

void setup() {   // put your setup code here, to run once:
  Serial.begin(9600);
    GPRS.begin(GPSBaud);
  LightSensor.begin();
LightSensor.SetAddress(Device_Address_H);
LightSensor.SetMode(Continuous_H_resolution_Mode);

  Serial.println("Running...");
  delay(500);//Delay to let system boot
}


void loop() {

sendemail();
  delay(20000000000000);//Wait 5 seconds before accessing sensor again.
  
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){

  uint8_t x=0,  answer=0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialice the string

  delay(100);

  while( Serial.available() > 0) Serial.read();    // Clean the input buffer

  Serial.println(ATcommand);    // Send the AT command 


    x = 0;
  previous = millis();

  // this loop waits for the answer
  do{
    // if there are data in the UART input buffer, reads it and checks for the answer
    if(Serial.available() != 0){    
      response[x] = Serial.read();
      x++;
      // check if the desired answer is in the response of the module
      if (strstr(response, expected_answer) != NULL)    
      {
        answer = 1;
      }
    }
    // Waits for the answer with time out
  }
  while((answer == 0) && ((millis() - previous) < timeout));    

  return answer;
}


 
 void sendemail(){
  
     Serial.println("Starting Shield to send an email...");

//configure bearer profile

sprintf(aux_str,"AT+SAPBR=3,1,\"Contype\",\"GPRS\"");

Serial.println(aux_str);
 buffer[count++]=GPRS.read();
 Serial.write(buffer,count);

sprintf(aux_str,"AT+SAPBR=3,1,\"APN\",\"%s\"",apn);
        
sendGSM(aux_str);

sprintf(aux_str,"AT+SAPBR=3,1,\"USER\",\"%s\"",user_name);
sendGSM(aux_str);


sprintf(aux_str,"AT+SAPBR=3,1,\"PWD\",\"%s\"",password);
sendGSM(aux_str);


//open a GPRS context
sprintf(aux_str,"AT+SAPBR=1,1");
sendGSM(aux_str, 7000);


  // sets the SMTP server and port
  sprintf(aux_str, "AT+SMTPSRV=\"%s\",\"%s\"", smtp_server, smtp_port);
sendGSM(aux_str);

  // sets user name and password
  sprintf(aux_str, "AT+SMTPAUTH=1,\"%s\",\"%s\"", smtp_user_name, smtp_password);
sendGSM(aux_str);

  // sets sender adress and name
  sprintf(aux_str, "AT+SMTPFROM=\"%s\",\"%s\"", sender_address, sender_name);
sendGSM(aux_str);

  // sets sender adress and name
  sprintf(aux_str, "AT+SMTPRCPT=1,0,\"%s\",\"%s\"", to_address, to_name);
sendGSM(aux_str);

  // subjet of the email
  sprintf(aux_str, "AT+SMTPSUB=\"%s\"", subject);
sendGSM(aux_str);

  // body of the email

  
//sprintf(body,"\"%s\",\"%s\",\"%s\"",cutemp,cuhum,cuilum);


sprintf(aux_str, "AT+SMTPBODY=65"); //no of charactaers +1 for dots in parameters
sendGSM(aux_str);
DHT.read11(dht_apin);
LightSensor.GetLightIntensity();

dtostrf(DHT.temperature,4,2,cutemp); //5 char
sprintf(aux_str, "Temperature =%s C",cutemp);//21 char
sendGSM(aux_str,3000);

dtostrf(DHT.humidity,4,2,cuhum);
sprintf(aux_str, "Humidity =%s pr",cuhum);//18 char
sendGSM(aux_str,3000);

dtostrf(LightSensor.GetLightIntensity(),5,1,cuilum);
sprintf(aux_str, "Illuminance =%s LUX",cuilum);
sendGSM(aux_str,3000);


//sendGSM("Humidity :");//10 char
//dtostrf(DHT.humidity,2,0,aux_str);// 3 char
//sendGSM(aux_str,3000);
//sendGSM("%'\n'");//2 char







sprintf(aux_str,"AT+SMTPSEND");
sendGSM(aux_str, 8000);
sprintf(aux_str,"AT+SAPBR=0,1");
sendGSM(aux_str);

sprintf(aux_str,"AT+CFUN");
sendGSM(aux_str);
 }

 
 //void meshum(){
  //DHT.read11(dht_apin);
  //Serial.print("Current humidity = ");
  //Serial.print("\t");  
  //Serial.print(DHT.humidity);
  //Serial.print("\t");  
  //Serial.println("%  ");
 //sprintf(cuhum,"\"Current humidity =\",\"%s\",\"Percent\"",DHT.humidity);
 //}

 //char mestemp(){
  //DHT.read11(dht_apin);
  //Serial.print("Current temprature = ");
  //Serial.print("\t");  
  //Serial.print(DHT.temperature);
  //Serial.print("\t");  
  //Serial.println("°C  ");
  //return DHT.temperature;
 //}

 //char meslux(){
  //Serial.print("Current light illuminance = ");
  //Serial.print("\t");  
  //Serial.print(LightSensor.GetLightIntensity());
  //Serial.print("\t");  
  //Serial.println("LUX  ");
  //return LightSensor.GetLightIntensity();// Get Lux value
 
// }

void parseATText(byte b) {

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) )
    resetBuffer(); // just to be safe

  /*
   // Detailed debugging
   Serial.println();
   Serial.print("state = ");
   Serial.println(state);
   Serial.print("b = ");
   Serial.println(b);
   Serial.print("pos = ");
   Serial.println(pos);
   Serial.print("buffer = ");
   Serial.println(buffer);*/

  switch (parseState) {
  case PS_DETECT_MSG_TYPE: 
    {
      if ( b == '\n' )
        resetBuffer();
      else {        
        if ( pos == 3 && strcmp(buffer, "AT+") == 0 ) {
          parseState = PS_IGNORING_COMMAND_ECHO;
        }
        else if ( b == ':' ) {
          //Serial.print("Checking message type: ");
          //Serial.println(buffer);

          if ( strcmp(buffer, "+HTTPACTION:") == 0 ) {
            Serial.println("Received HTTPACTION");
            parseState = PS_HTTPACTION_TYPE;
          }
          else if ( strcmp(buffer, "+HTTPREAD:") == 0 ) {
            Serial.println("Received HTTPREAD");            
            parseState = PS_HTTPREAD_LENGTH;
          }
          resetBuffer();
        }
      }
    }
    break;

  case PS_IGNORING_COMMAND_ECHO:
    {
      if ( b == '\n' ) {
        Serial.print("Ignoring echo: ");
        Serial.println(buffer);
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_TYPE:
    {
      if ( b == ',' ) {
        Serial.print("HTTPACTION type is ");
        Serial.println(buffer);
        parseState = PS_HTTPACTION_RESULT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_RESULT:
    {
      if ( b == ',' ) {
        Serial.print("HTTPACTION result is ");
        Serial.println(buffer);
        parseState = PS_HTTPACTION_LENGTH;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_LENGTH:
    {
      if ( b == '\n' ) {
        Serial.print("HTTPACTION length is ");
        Serial.println(buffer);
        
        // now request content
        GPRS.print("AT+HTTPREAD=0,");
        GPRS.println(buffer);
        
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_LENGTH:
    {
      if ( b == '\n' ) {
        contentLength = atoi(buffer);
        Serial.print("HTTPREAD length is ");
        Serial.println(contentLength);
        
        Serial.print("HTTPREAD content: ");
        
        parseState = PS_HTTPREAD_CONTENT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_CONTENT:
    {
      // for this demo I'm just showing the content bytes in the serial monitor
      Serial.write(b);
      
      contentLength--;
      
      if ( contentLength <= 0 ) {

        // all content bytes have now been read

        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;
  }
}
void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}
bool getGPS() 
{
   if(!getGPRMC())    //
     return false;
  // Serial.println(receivedStack);
   if(!parseGPRMC(receivedStack))  //不是$GPRMC字符串开头的GPS信息
     return false;  
    
  // skip mode
    char *tok = strtok(receivedStack, ",");     //起始引导符
    if (! tok) return false;

   // grab time                                  //<1> UTC时间，格式为hhmmss.sss；
   // tok = strtok(NULL, ",");
  char *time = strtok(NULL, ",");
    if (! time) return false;
  uint32_t newTime = (uint32_t)parseDecimal(time);
  getTime(newTime);

    // skip fix
    tok = strtok(NULL, ",");              //<2> 定位状态，A=有效定位，V=无效定位
    if (! tok) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");       //<3> 纬度ddmm.mmmm(度分)格式(前面的0也将被传输)
    if (! latp) return false;

    // grab latitude direction              // <4> 纬度半球N(北半球)或S(南半球)
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude                       //<5> 经度dddmm.mmmm(度分)格式(前面的0也将被传输)
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction            //<6> 经度半球E(东经)或W(西经)
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    float latitude = atof(latp);
    float longitude = atof(longp);

  GPSdata.lat = latitude/100;

    // convert longitude from minutes to decimal  
  GPSdata.lon= longitude/100;

    // only grab speed if needed                  //<7> 地面速率(000.0~999.9节，前面的0也将被传输)
   // if (speed_kph != NULL) {

      // grab the speed in knots
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      // convert to kph
      //*speed_kph = atof(speedp) * 1.852;
    GPSdata.speed_kph= atof(speedp) * 1.852;

   // }

    // only grab heading if needed             
   // if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      //*heading = atof(coursep);
    GPSdata.heading = atof(coursep);
   // }
  
  // grab date
  char *date = strtok(NULL, ",");   
    if (! date) return false;
  uint32_t newDate = atol(date);
  getDate(newDate);

    // no need to continue
   // if (altitude == NULL){
   //   return true;
  //}
  return true;
}
bool getGPRMC()
{
  char c;
  static bool endflag  = false;
  static char count;
    
  while(GPRS->available())   
  { c = GPRS->read();
    if(endflag)
    {
      if(count--)
      {
        receivedStack[receivedStackIndex++] = c;
      }
      else{
        endflag = false;
        receivedStack[receivedStackIndex] = '\0';
        return true;
      }     
    }
    else
    { 
      switch(c)
      {
          
        case '$':                 
          receivedStackIndex = 0;
          receivedStack[receivedStackIndex++] = c;                
          break;
        case '*':
          endflag = true;
          count = 2;
          receivedStack[receivedStackIndex++] = c;                  
          break;
        default:
          if(receivedStackIndex < 120)
            receivedStack[receivedStackIndex++] = c;
          break;
      
      }
      return false;
    }
    return false;
  }
  return false;
}
bool parseGPRMC(char *gpsbuffer)
{
  if(strstr(gpsbuffer,des) == NULL)  //閿熸枻鎷烽敓鏂ゆ嫹$GPRMC閿熻鍑ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹澶撮敓鏂ゆ嫹GPS閿熸枻鎷锋伅
  {
    receivedStackIndex = 0;
    return false;
  }
  else
  {   
    if(gpsbuffer[18] == 'A')        //閿熷彨璁规嫹閿熸枻鎷烽敓鏂ゆ嫹閿熻鍑ゆ嫹閿熸枻鎷锋晥
      return true;
    else
    {
      //Serial.print("NO :");
      //Serial.println(gpsbuffer[18]);
      return false;
    }
      
  }
    
}
int32_t parseDecimal(const char *term)
{
  bool negative = *term == '-';
  if (negative) ++term;
  int32_t ret = 100 * (int32_t)atol(term);
  while (isdigit(*term)) ++term;
  if (*term == '.' && isdigit(term[1]))
  {
    ret += 10 * (term[1] - '0');
    if (isdigit(term[2]))
      ret += term[2] - '0';
  }
  return negative ? -ret : ret;
}
 void getTime(uint32_t time){
   GPSdata.hour     =  time / 1000000;
   GPSdata.minute  = (time / 10000) % 100;
   GPSdata.second = (time / 100) % 100;
   GPSdata.centisecond =  time % 100;
 }
