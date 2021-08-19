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

/*******************************************************************************/
void setup() {
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

  uint8_t error = paj7620Init();      // initialize Paj7620 registers
  if (error) {
    Serial.print("INIT ERROR,CODE:");
    Serial.println(error);
  } else {
    Serial.println("INIT OK");
  }

  setPoles(0,"One");
  setPoles(0,"Two");
  setPoles(0,"Three");
  setPoles(0,"Four");
  setOverride();

  Serial.println("GESTURE SWITCH SETUP COMPLETE");
}
/*******************************************************************************/

int count;
String gesture;
byte data;
int Poles[] = {0,0,0,0};

/*******************************************************************************/
//MAIN PROGRAM LOOP
void loop() {
  getGesture(&gesture,&count); //returns a count and the gesture as a String
  //if a gesture was found and it enters the case and leaves with default values.
  if(gesture != "unknown" && count == 1) {
    switchPole(getSwipe(gesture));
    count--;
    gesture = "unknown";
  }
  checkOverride();
  Serial.println("Pole Values");
  Serial.println(Poles[0]);
  Serial.println(Poles[1]);
  Serial.println(Poles[2]);
  Serial.println(Poles[3]);
}
/*******************************************************************************/

void checkOverride() {
  if (Firebase.getInt(firebaseData, "/alterPoles/overrideFlag")) {
    if (firebaseData.dataType() == "int") {
      int temp = firebaseData.intData();
      if(temp == 1) {
        if(Poles[0] == getPoles("One")) {switchPole("One");}
        if(Poles[1] == getPoles("Two")) {switchPole("Two");}
        if(Poles[2] == getPoles("Three")) {switchPole("Three");}
        if(Poles[3] == getPoles("Four")) {switchPole("Four");}
        setOverride();
        return;
      } else {
        return;
      }
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

/*******************************************************************************/

void setOverride() {
  if (Firebase.setInt(firebaseData, "/alterPoles/overrideFlag", 0)) {
  if (firebaseData.dataType() == "int")
    Serial.println(firebaseData.intData());
  } else {
    Serial.println(firebaseData.errorReason());
  }
}
/*******************************************************************************/
//Takes the string of the pole that will be changed and sets it to the opposite
//of its current state.
void switchPole(String path) {
  int call = getPoles(path);
  call = invert(call);
  if (path == "One") {
    digitalWrite(A1,call);
    setPoles(call,path);
    Poles[0] = call;
  } else if (path == "Two") {
    digitalWrite(A2,call);
    setPoles(call,path);
    Poles[1] = call;
  } else if (path == "Three") {
    digitalWrite(A3,call);
    setPoles(call,path);
    Poles[2] = call;
  } else if (path == "Four") {
    digitalWrite(A4,call);
    setPoles(call,path);
    Poles[3] = call;
  } else {
    return;
  }
  return;
}

/*******************************************************************************/
//Inverts 0 or 1 to 0 or 1
int invert(int num) {
  if(num == 1) {
    return 0;
  } else {
    return 1;
  }
}

/*******************************************************************************/

int getPoles(String path) {
  bool temp;
  if (Firebase.getBool(firebaseData, "/alterPoles/pole" + path)) {
    if (firebaseData.dataType() == "boolean") {
      temp = firebaseData.boolData();
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

/*******************************************************************************/

void setPoles(int num, String path) {
  bool call;
  if(num == 1) {
    call = true;
  } else {
    call = false;
  }
  if (Firebase.setBool(firebaseData, "/alterPoles/pole" + path, call)) {
  if (firebaseData.dataType() == "boolean")
    Serial.println(firebaseData.boolData());
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

/*******************************************************************************/

String getSwipe(String path) {
  if (Firebase.getString(firebaseData, "/setGestures/swipe" + path +"Num")) {
    if (firebaseData.dataType() == "string") {
      String temp = firebaseData.stringData();
      return temp;
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

/*******************************************************************************/

void getGesture(String* value,int* num) {
  *value = "unknown";
  uint8_t error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if(error) {return;}

  delay(GES_ENTRY_TIME);
  paj7620ReadReg(0x43, 1, &data);

  switch (data) {                 // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
    case GES_RIGHT_FLAG:
      *value = "Right";
      *num = 1;
      break;
    case GES_LEFT_FLAG:
      *value = "Left";
      *num = 1;
      break;
    case GES_UP_FLAG:
      *value = "Up";
      *num = 1;
      break;
    case GES_DOWN_FLAG:
      *value = "Down";
      *num = 1;
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
  return;
}
