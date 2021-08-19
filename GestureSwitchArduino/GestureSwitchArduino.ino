#include <Wire.h>
#include "paj7620.h"
#include <Firebase_Arduino_WiFiNINA.h>
#include <RTCZero.h>
#define FIREBASE_HOST "gesture-switch-445f4-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AZMMUzW8jhmr31s5tY6kASyaPGcXsH8netRNDjPV"
#define WIFI_SSID "Telstra43B1"
#define WIFI_PASSWORD "4173935201"


FirebaseData firebaseData;
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

//  rtc.begin();
//  setRTC();  // get Epoch time from Internet Time Service
//  fixTimeZone();

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
  //MAKE getGesture() to return a number
  String gesture = getGesture();
  if(gesture != "unknown") {
    doGesture(gesture);
  }
  delay(100);
  alterPoles();
  
//  secs = rtc.getSeconds();
//
//  //MAIN CLOCK LOOP
//  if (secs == 0) fixTimeZone(); // when secs is 0, update everything and correct for time zone
//  // otherwise everything else stays the same.
//  printDate();
//  printTime();
//  Serial.println();
//  while (secs == rtc.getSeconds())delay(10); // wait until seconds change
//  if (mins==59 && secs ==0) setRTC(); // get NTP time every hour at minute 59
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
/*
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
//  } else if (){
//    return "";
  } else {
    return "None";
  }
}
*/
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
