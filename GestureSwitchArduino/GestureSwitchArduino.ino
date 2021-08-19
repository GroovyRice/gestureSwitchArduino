#include <Wire.h>
#include "paj7620.h"
#include <Firebase_Arduino_WiFiNINA.h>
#include <RTCZero.h>
#define FIREBASE_HOST "gesture-switch-445f4-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AZMMUzW8jhmr31s5tY6kASyaPGcXsH8netRNDjPV"
#define WIFI_SSID "Telstra43B1"
#define WIFI_PASSWORD "4173935201"


FirebaseData firebaseData;

//REAL TIME CLOCK
const int GMT = +10; //change this to adapt it to your time zone
const int myClock = 12;  // can be 24 or 12 hour clock
const int dateOrder = 0;  // 1 = MDY; 0 for DMY

RTCZero rtc; // create instance of real time clock
int status = WL_IDLE_STATUS;
int myhours, mins, secs, myday, mymonth, myyear;
bool IsPM = false;
/*
  Notice: When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
        You also can adjust the reaction time according to the actual circumstance.
*/
#define GES_REACTION_TIME 500       // You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME 800       // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
#define GES_QUIT_TIME 500

void setup() {
  uint8_t error = 0;

  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);

  Serial.begin(9600);
  delay(500);
  Serial.println();

  Serial.print("Connecting to WiFi…");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(".");
    delay(300);
  }
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);

  rtc.begin();
  setRTC();  // get Epoch time from Internet Time Service
  fixTimeZone();

  setPoles(0,"Up");
  setPoles(0,"Down");
  setPoles(0,"Left");
  setPoles(0,"Right");

  Serial.println("\nPAJ7620U2 TEST DEMO: Recognize 9 gestures.");

  error = paj7620Init();      // initialize Paj7620 registers
  if (error) {
    Serial.print("INIT ERROR,CODE:");
    Serial.println(error);
  } else {
    Serial.println("INIT OK");
  }
  Serial.println("Please input your gestures:\n");
  }

void loop() {
  String gesture = getGesture();
  if(gesture != "unknown") {
    doGesture(gesture);
  }
  delay(100);
  secs = rtc.getSeconds();

  //MAIN CLOCK LOOP
  if (secs == 0) fixTimeZone(); // when secs is 0, update everything and correct for time zone
  // otherwise everything else stays the same.
  printDate();
  printTime();
  Serial.println();
  while (secs == rtc.getSeconds())delay(10); // wait until seconds change
  if (mins==59 && secs ==0) setRTC(); // get NTP time every hour at minute 59
}

void doGesture(String temp) {
  int num, call;
  call = getSwipe("swipe" + temp + "Num");
  switch (call) {
    case 1:
      num = getPoles(temp);
      if(num == 0) {
        digitalWrite(A1,1);
        setPoles(1,temp);
      } else {
        digitalWrite(A1,0);
        setPoles(0,temp);
      }
      Serial.println("You swiped " + temp);
      break;
    case 2:
      num = getPoles(temp);
      if(num == 0) {
        digitalWrite(A2,1);
        setPoles(1,temp);
      } else {
        digitalWrite(A2,0);
        setPoles(0,temp);
      }
      Serial.println("You swiped " + temp);
      break;
    case 3:
      num = getPoles(temp);
      if(num == 0) {
        digitalWrite(A3,1);
        setPoles(1,temp);
      } else {
        digitalWrite(A3,0);
        setPoles(0,temp);
      }
      Serial.println("You swiped " + temp);
      break;
    case 4:
      num = getPoles(temp);
      if(num == 0) {
        digitalWrite(A4,1);
        setPoles(1,temp);
      } else {
        digitalWrite(A4,0);
        setPoles(0,temp);
      }
      Serial.println("You swiped " + temp);
      break;
    default:
      break;
  }
  return;
}

void setPoles(int num, String path) {
  if (Firebase.setFloat(firebaseData, "/setPoles/" + path, num)) {
  if (firebaseData.dataType() == "float")
    Serial.println(firebaseData.floatData());
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

int getPoles(String path) {
  int temp;
  if (Firebase.getInt(firebaseData, "/setPoles/" + path)) {
    if (firebaseData.dataType() == "int") {
      temp = firebaseData.intData();
      Serial.println(temp);
      return temp;
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

int getSwipe(String path) {
  int temp;
  if (Firebase.getInt(firebaseData, "/setGestures/" + path)) {
    if (firebaseData.dataType() == "int") {
      temp = firebaseData.intData();
      Serial.println(temp);
      return temp;
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

void alterPoles() {
  int poleOne, poleTwo, poleThree, poleFour;
  String strOne, strTwo, strThree, strFour;
  poleOne = getAlterPoles("One");
  poleTwo = getAlterPoles("Two");
  poleThree = getAlterPoles("Three");
  poleFour = getAlterPoles("Four");
  strOne = findPole(1);
  strTwo = findPole(2);
  strThree = findPole(3);
  strFour = findPole(4);
  digitalWrite(A1,poleOne);
  digitalWrite(A2,poleTwo);
  digitalWrite(A3,poleThree);
  digitalWrite(A4,poleFour);
}

int getAlterPoles(String path) {
  bool temp;
  if (Firebase.getInt(firebaseData, "/alterPoles/pole" + path)) {
    if (firebaseData.dataType() == "boolean") {
      temp = firebaseData.boolData();
      Serial.println(temp);
      if(temp) {
        return 1;
      } else {
        return 0;
      }
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

String findPole(int pole) {
  if (pole == getSwipe("swipeUpNum")) {
    return "Up";
  } else if (pole == getSwipe("swipeDownNum")) {
    return "Down";
  } else if (pole == getSwipe("swipeLeftNum")) {
    return "Left";
  } else if (pole == getSwipe("swipeRightNum")) {
    return "Right";
  } else {
    return "None"
  }
}

String getGesture() {
  uint8_t data = 0;
  uint8_t data1 = 0;
  uint8_t error = 0;

  error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if(error) {return "unknown";}

  String value = "unknown";

  delay(GES_ENTRY_TIME);
  paj7620ReadReg(0x43, 1, &data);

  switch (data) {                 // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
    case GES_RIGHT_FLAG:
      value = "Right";
      break;
    case GES_LEFT_FLAG:
      value = "Left";
      break;
    case GES_UP_FLAG:
      value = "Up";
      break;
    case GES_DOWN_FLAG:
      value = "Down";
      break;
//    case GES_FORWARD_FLAG:
//      value = "Forward";
//      break;
//    case GES_BACKWARD_FLAG:
//      value = "Backward";
//      break;
//    case GES_CLOCKWISE_FLAG:
//      value = "ClockWise";
//      break;
//    case GES_COUNT_CLOCKWISE_FLAG:
//      value = "AnticlockWise";
//      break;
//    case GES_WAVE_FLAG:
//      value = "Wave";
//      break;
    default:
      break;
  }
  delay(GES_QUIT_TIME);
  return value;
}

//CLOCK FUNCTIONS
void printDate()
{
  if (dateOrder == 0) {
    Serial.print(myday);
    Serial.print("/");
  }
  Serial.print(mymonth);
  Serial.print("/");
  if (dateOrder == 1) {
    Serial.print(myday);
    Serial.print("/");
  }
  Serial.print("20");
  Serial.print(myyear);
  Serial.print(" ");
}

void printTime()
{
  print2digits(myhours);
  Serial.print(":");
  print2digits(mins);
  Serial.print(":");
  print2digits(secs);
  if (myClock==12) {
    if(IsPM) Serial.print("  PM");
    else Serial.print("  AM");
  }
  Serial.println();
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

void setRTC() { // get the time from Internet Time Service
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime(); // The RTC is set to GMT or 0 Time Zone and stays at GMT.
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    while (1);  // hang
  }
  else {
    Serial.print("Epoch Time = ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);
    Serial.println();
  }
}

/* There is more to adjusting for time zone that just changing the hour.
   Sometimes it changes the day, which sometimes chnages the month, which
   requires knowing how many days are in each month, which is different
   in leap years, and on Near Year's Eve, it can even change the year! */
void fixTimeZone() {
  int daysMon[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (myyear % 4 == 0) daysMon[2] = 29; // fix for leap year
  myhours = rtc.getHours();
  mins = rtc.getMinutes();
  myday = rtc.getDay();
  mymonth = rtc.getMonth();
  myyear = rtc.getYear();
  myhours +=  GMT; // initial time zone change is here
  if (myhours < 0) {  // if hours rolls negative
    myhours += 24; // keep in range of 0-23
    myday--;  // fix the day
    if (myday < 1) {  // fix the month if necessary
      mymonth--;
      if (mymonth == 0) mymonth = 12;
      myday = daysMon[mymonth];
      if (mymonth == 12) myyear--; // fix the year if necessary
    }
  }
  if (myhours > 23) {  // if hours rolls over 23
    myhours -= 24; // keep in range of 0-23
    myday++; // fix the day
    if (myday > daysMon[mymonth]) {  // fix the month if necessary
      mymonth++;
      if (mymonth > 12) mymonth = 1;
      myday = 1;
      if (mymonth == 1)myyear++; // fix the year if necessary
    }
  }
  if (myClock == 12) {  // this is for 12 hour clock
    IsPM = false;
    if (myhours > 11)IsPM = true;
    myhours = myhours % 12; // convert to 12 hour clock
    if (myhours == 0) myhours = 12;  // show noon or midnight as 12
  }
}
