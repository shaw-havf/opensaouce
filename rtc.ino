//MCP7940N  RTC DEMO CODE  SLEEP DEMO
//KEVIN DARRAH 2/21/17
//http://www.kevindarrah.com/wiki/index.php?title=RTC_Board

#include <Wire.h>

//functions
void rtcInit();
void rtcGetTime();
void checkSerial();
boolean checkAlarm();
void goToSleep(byte sleepSeconds);

//defines
#define RTCADDR B1101111//page11 datasheet
#define RTCSEC 0x00
#define RTCMIN 0x01
#define RTCHOUR 0x02
#define RTCWKDAY 0x03
#define RTCDATE 0x04
#define RTCMTH 0x05
#define RTCYEAR 0x06
#define CONTROL 0x07
#define OSCTRIM 0x08
#define ALM0SEC 0x0A
#define ALM0MIN 0x0B
#define ALM0HOUR 0x0C
#define ALM0WKDAY 0x0D
#define ALM0DATE 0x0E
#define ALM0MTH 0x0F
//#define ALM1SEC 0x11
//#define ALM1MIN 0x12
//#define ALM1HOUR 0x13
//#define ALM1WKDAY 0x14
//#define ALM1DATE 0x15
//#define ALM1MTH 0x16
//#define PWRDNMIN 0x18
//#define PWRDNHOUR 0x19
//#define PWRDNDATE 0x1A
//#define PWRDNMTH 0x1B
//#define PWRUPMIN 0x1C
//#define PWRUPHOUR 0x1D
//#define PWRUPDATE 0x1E
//#define PWRUPMTH 0x1F

//variables used here
byte rtcSeconds, rtcMinutes, rtcHours;
byte rtcWeekDay, rtcDay, rtcMonth, rtcYear;
boolean rtc12hrMode, rtcPM, rtcOscRunning, rtcPowerFail, rtcVbatEn;
String weekDay[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
boolean mfpPinTriggered = false;


void setup() {//setup
  //Serial.begin(9600);
  attachInterrupt(0, rtcMFP, FALLING);//interrupt for rtc Alarm
  rtcInit();//go initialize RTC

  for (int i = 0; i < 18; i++) {//set all pins LOW, to save power from floating pins
    if (i != 2) //using this pin for interrupt input
      pinMode(i, OUTPUT);
  }


  //Disable ADC - don't forget to flip back after waking up if using ADC in your application ADCSRA |= (1 << 7);
  ADCSRA &= ~(1 << 7);
  //ENABLE SLEEP - this enables the sleep mode
  SMCR |= (1 << 2); //power down mode
  SMCR |= 1;//enable sleep
}

void loop() {
  goToSleep(0x10);//seconds
  
  while (!checkAlarm()) {
    //BOD DISABLE - this must be called right before the __asm__ sleep instruction
    MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
    MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
    __asm__  __volatile__("sleep");//in line assembler to go to sleep
  }
  for (int i = 0; i < 10; i++) {
    digitalWrite(3, !digitalRead(3)); //LED flash
    delay(50);
  }
}




void goToSleep(byte sleepSeconds) {

  Wire.beginTransmission(RTCADDR);
  Wire.write(RTCSEC);
  Wire.write(0x80);//back to zero, but start counting
  Wire.endTransmission();

  Wire.beginTransmission(RTCADDR);//set the alarm time
  Wire.write(ALM0SEC);
  Wire.write(sleepSeconds);//seconds only
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x00);//clears flag
  Wire.endTransmission();

  //BOD DISABLE - this must be called right before the __asm__ sleep instruction
  MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
  __asm__  __volatile__("sleep");//in line assembler to go to sleep
}

void rtcInit() {// RTC Initialize
  //sets up I2C at 100kHz
  Wire.setClock(100000);
  Wire.begin();

  Wire.beginTransmission(RTCADDR);
  Wire.write(CONTROL);
  Wire.write(B00000000);//clear out the entire control register
  Wire.endTransmission();

  Wire.beginTransmission(RTCADDR);
  Wire.write(RTCWKDAY);
  Wire.endTransmission();
  Wire.requestFrom(RTCADDR, 1);
  delay(1);
  byte rtcWeekdayRegister = Wire.read();
  rtcWeekdayRegister |= 0x08;//enable Battery backup
  Wire.beginTransmission(RTCADDR);
  Wire.write(RTCWKDAY);
  Wire.write(rtcWeekdayRegister);
  Wire.endTransmission();


  Wire.beginTransmission(RTCADDR);
  Wire.write(RTCSEC);
  Wire.endTransmission();
  Wire.requestFrom(RTCADDR, 1);
  delay(1);
  byte rtcSecondRegister = Wire.read();//read out seconds
  rtcSecondRegister |= 0x80;// flip the start bit to ON
  Wire.beginTransmission(RTCADDR);
  Wire.write(RTCSEC);
  Wire.write(rtcSecondRegister);//write it back in... now the RTC is running
  Wire.endTransmission();

  Wire.beginTransmission(RTCADDR);//Enable Alarm0
  Wire.write(CONTROL);
  Wire.endTransmission();
  Wire.requestFrom(RTCADDR, 1);
  delay(1);
  byte rtcControlRegeister = Wire.read();
  rtcControlRegeister |= 0x10;//enable alm0
  Wire.beginTransmission(RTCADDR);
  Wire.write(CONTROL);
  Wire.write(rtcControlRegeister);
  Wire.endTransmission();
}

boolean checkAlarm() {
  if (mfpPinTriggered == true) {// RTC pin fell LOW
    mfpPinTriggered = false;
    Wire.beginTransmission(RTCADDR);
    Wire.write(ALM0WKDAY);//check the RT Flag
    Wire.endTransmission();
    Wire.requestFrom(RTCADDR, 1);
    delay(1);
    byte alarm0Check = Wire.read();
    if (((alarm0Check >> 3) & 0x01) == 1)//Set?
      return 1;
    else
      return 0;
  }
}

void rtcMFP() {
  mfpPinTriggered = true;
}



//void rtcGetTime() {
//  Wire.beginTransmission(RTCADDR);
//  Wire.write(RTCSEC);
//  Wire.endTransmission();
//  Wire.requestFrom(RTCADDR, 7);//pull out all timekeeping registers
//  delay(1);//little delay
//
//  //now read each byte in and clear off bits we don't need, hence the AND operations
//  rtcSeconds = Wire.read() & 0x7F;
//  rtcMinutes = Wire.read() & 0x7F;
//  rtcHours = Wire.read() & 0x7F;
//  rtcWeekDay = Wire.read() & 0x3F;
//  rtcDay = Wire.read() & 0x3F;
//  rtcMonth = Wire.read() & 0x3F;
//  rtcYear = Wire.read();
//
//  //now format the data, combine lower and upper parts of byte to give decimal number
//  rtcSeconds = (rtcSeconds >> 4) * 10 + (rtcSeconds & 0x0F);
//  rtcMinutes = (rtcMinutes >> 4) * 10 + (rtcMinutes & 0x0F);
//
//  if ((rtcHours >> 6) == 1)//check for 12hr mode
//    rtc12hrMode = true;
//  else rtc12hrMode = false;
//
//  // 12hr check and formatting of Hours
//  if (rtc12hrMode) { //12 hr mode so get PM/AM
//    if ((rtcHours >> 5) & 0x01 == 1)
//      rtcPM = true;
//    else rtcPM = false;
//    rtcHours = ((rtcHours >> 4) & 0x01) * 10 + (rtcHours & 0x0F);//only up to 12
//  }
//  else { //24hr mode
//    rtcPM = false;
//    rtcHours = ((rtcHours >> 4) & 0x03) * 10 + (rtcHours & 0x0F);//uses both Tens digits, '23'
//  }
//
//  //weekday register has some other bits in it, that are pulled out here
//  if ((rtcWeekDay >> 5) & 0x01 == 1)
//    rtcOscRunning = true;// good thing to check to make sure the RTC is running
//  else rtcOscRunning = false;
//  if ((rtcWeekDay >> 4) & 0x01 == 1)
//    rtcPowerFail = true;// if the power fail bit is set, we can then go pull the timestamp for when it happened
//  else rtcPowerFail = false;
//  if ((rtcWeekDay >> 3) & 0x01 == 1)//check to make sure the battery backup is enabled
//    rtcVbatEn = true;
//  else rtcVbatEn = false;
//
//  rtcWeekDay = rtcWeekDay & 0x07;//only the bottom 3 bits for the actual weekday value
//
//  //more formatting bytes into decimal numbers
//  rtcDay = (rtcDay >> 4) * 10 + (rtcDay & 0x0F);
//  rtcMonth = ((rtcMonth >> 4) & 0x01) * 10 + (rtcMonth & 0x0F);
//  rtcYear = (rtcYear >> 4) * 10 + (rtcYear & 0x0F);
//
//  //print everything out
//  Serial.print(rtcHours);
//  Serial.print(":");
//  Serial.print(rtcMinutes);
//  Serial.print(":");
//  Serial.print(rtcSeconds);
//
//  if (rtc12hrMode == true && rtcPM == true)
//    Serial.print(" PM ");
//  else if (rtc12hrMode == true && rtcPM == false)
//    Serial.print(" AM ");
//
//  if (rtc12hrMode == false)
//    Serial.print(" 24hr ");
//
//  //Serial.print("WeekDay=");
//  //Serial.print(rtcWeekDay);
//  Serial.print(weekDay[rtcWeekDay - 1]);
//  Serial.print(" ");
//  Serial.print(rtcMonth);
//  Serial.print("/");
//  Serial.print(rtcDay);
//  Serial.print("/");
//  Serial.print(rtcYear);
//  Serial.println("");
//}
//
//void checkSerial() {
//  //variables used just for setting the time and alarms
//  byte rtcNewHour, rtcNewMinute, rtcNewSecond;
//  byte rtcNewMonth, rtcNewDay, rtcNewYear, rtcNewWeekDay;
//  byte rtcNew12hrMode, rtcNewPM;
//  byte rtcClockStopTest;
//  byte rtcAlarm0Sec, rtcAlarm0Min, rtcAlarm0Hour, rtcAlarm0mask;
//  byte rtcAlarmWeekday, rtcAlarmDay, rtcAlarmMonth;
//
//  //if something is available - make sure you have line feed enabled
//  if (Serial.available()) {
//    //format set a to 1 if 12hr mode, then b to 1 if PM, otherwise set to 00 for 24hr mode
//    String fromSerial = "Thh:mm:22 ab mm/dd/yr w";
//    //T03:41:00 11 02/04/17 7
//    fromSerial = Serial.readStringUntil('\n');//read until line  feed
//    Serial.println(fromSerial);//for debugging, just echo back what you got
//    if (fromSerial[0] == 'T' && fromSerial.length() == 23) { //SETTING TIME - must match length
//
//      //first pull down the string, convert from ASCII to decimal, and combine the bytes
//      rtcNewHour = (((fromSerial[1] - 48) << 4) & 0xF0) + ((fromSerial[2] - 48) & 0x0F);
//      rtcNewMinute = (((fromSerial[4] - 48) << 4) & 0xF0) + ((fromSerial[5] - 48) & 0x0F);
//      rtcNewSecond = (((fromSerial[7] - 48) << 4) & 0xF0) + ((fromSerial[8] - 48) & 0x0F);
//      rtcNew12hrMode = fromSerial[10] - 48;
//      rtcNewHour |= (rtcNew12hrMode << 6);
//      rtcNewPM = fromSerial[11] - 48;
//      rtcNewHour |= (rtcNewPM << 5);
//
//      rtcNewMonth = (((fromSerial[13] - 48) << 4) & 0xF0) + ((fromSerial[14] - 48) & 0x0F);
//
//      rtcNewDay = (((fromSerial[16] - 48) << 4) & 0xF0) + ((fromSerial[17] - 48) & 0x0F);
//
//      rtcNewYear = (((fromSerial[19] - 48) << 4) & 0xF0) + ((fromSerial[20] - 48) & 0x0F);
//
//      rtcNewWeekDay = (fromSerial[22] - 48) | (1 << 3); //keep VBAT enabled
//
//      //let's go change the time:
//      //first stop the clock:
//      Wire.beginTransmission(RTCADDR);
//      Wire.write(RTCSEC);
//      Wire.write(0x00);
//      Wire.endTransmission();
//
//
//      rtcGetTime();//go grab the time again just to make sure the osc stopped
//
//
//      if (rtcOscRunning == false) { //oscillator stopped, we're good
//        Serial.println("RTC has stopped - Changing Time");
//        Wire.beginTransmission(RTCADDR);//set the time/date
//        Wire.write(RTCSEC);
//        Wire.write(rtcNewSecond);
//        Wire.write(rtcNewMinute);
//        Wire.write(rtcNewHour);
//        Wire.write(rtcNewWeekDay);
//        Wire.write(rtcNewDay);
//        Wire.write(rtcNewMonth);
//        Wire.write(rtcNewYear);
//        Wire.endTransmission();
//
//        Wire.beginTransmission(RTCADDR);//start back up
//        Wire.write(RTCSEC);
//        Wire.endTransmission();
//        Wire.requestFrom(RTCADDR, 1);
//        delay(1);
//        byte rtcSecondRegister = Wire.read();
//        rtcSecondRegister |= 0x80;//start!
//        Wire.beginTransmission(RTCADDR);
//        Wire.write(RTCSEC);
//        Wire.write(rtcSecondRegister);
//        Wire.endTransmission();
//      }
//      else return;
//    }
//    if (fromSerial[0] == 'A' && fromSerial[1] == '0' && fromSerial.length() == 24) { //SET ALARM 0
//      //A0hh:mm:ss abc d mm/dd w
//      //A012:13:00 000 0 02/07 7
//      //abc = alarm mask, d = AM/PM 1=PM
//      //000=seconds
//      //001=minutes
//      //010=hours
//      //011=day of week
//      //111=sec,min,hours,day of week, date, month
//      //rtcAlarm0Sec, rtcAlarm0Min, rtcAlarm0Hour
//      //rtcAlarmWeekday, rtcAlarmDay, rtcAlarmMonth;
//      Serial.println("Setting Alarm");
//      rtcAlarm0Hour = ((fromSerial[15] - 48) << 5) + (((fromSerial[2] - 48) << 4) & 0xF0) + ((fromSerial[3] - 48) & 0x0F);
//      rtcAlarm0Min = (((fromSerial[5] - 48) << 4) & 0xF0) + ((fromSerial[6] - 48) & 0x0F);
//      rtcAlarm0Sec = (((fromSerial[8] - 48) << 4) & 0xF0) + ((fromSerial[9] - 48) & 0x0F);
//      rtcAlarm0mask = ((fromSerial[11] - 48) << 6) + ((fromSerial[12] - 48) << 5) + ((fromSerial[13] - 48) << 4);
//      rtcAlarmMonth = (((fromSerial[17] - 48) << 4) & 0xF0) + ((fromSerial[18] - 48) & 0x0F);
//      rtcAlarmDay = (((fromSerial[20] - 48) << 4) & 0xF0) + ((fromSerial[21] - 48) & 0x0F);
//      rtcAlarmWeekday = (fromSerial[23] - 48) + rtcAlarm0mask;
//
//      Wire.beginTransmission(RTCADDR);//set the time/date
//      Wire.write(ALM0SEC);
//      Wire.write(rtcAlarm0Sec);
//      Wire.write(rtcAlarm0Min);
//      Wire.write(rtcAlarm0Hour);
//      Wire.write(rtcAlarmWeekday);
//      Wire.write(rtcAlarmDay);
//      Wire.write(rtcAlarmMonth);
//      Wire.endTransmission();
//
//      Wire.beginTransmission(RTCADDR);//Enable Alarm0
//      Wire.write(CONTROL);
//      Wire.endTransmission();
//      Wire.requestFrom(RTCADDR, 1);
//      delay(1);
//      byte rtcControlRegeister = Wire.read();
//      rtcControlRegeister |= 0x10;//enable alm0
//      Wire.beginTransmission(RTCADDR);
//      Wire.write(CONTROL);
//      Wire.write(rtcControlRegeister);
//      Wire.endTransmission();
//    }
//  }
//}
//
//
//
