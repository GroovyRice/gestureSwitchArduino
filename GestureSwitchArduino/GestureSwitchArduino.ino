#include <Wire.h>
#include "paj7620.h"
#include <Firebase_Arduino_WiFiNINA.h>
#define FIREBASE_HOST "gesture-switch-445f4-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AZMMUzW8jhmr31s5tY6kASyaPGcXsH8netRNDjPV"
#define WIFI_SSID "Telstra43B1"
#define WIFI_PASSWORD "4173935201"

FirebaseData firebaseData;

/*
  Notice: When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
        You also can adjust the reaction time according to the actual circumstance.
*/
#define GES_REACTION_TIME   500       // You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME      800       // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
#define GES_QUIT_TIME     500

void setup() {
  uint8_t error = 0;

  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);

 /******************************************************************************************************************/
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

//   if (!Firebase.beginStream(firebaseData, path)) {
//    Serial.println("------Can't begin stream connection------");
//    Serial.println("REASON: " + firebaseData.errorReason());
//    Serial.println();
//  }

  setPoles(0,"Up");
  setPoles(0,"Down");
  setPoles(0,"Left");
  setPoles(0,"Right");

  Serial.println("\nPAJ7620U2 TEST DEMO: Recognize 9 gestures.");

  error = paj7620Init();      // initialize Paj7620 registers
  if (error) {
    Serial.print("INIT ERROR,CODE:");
    Serial.println(error);
  }
  else {
    Serial.println("INIT OK");
  }
  Serial.println("Please input your gestures:\n");
}

void loop() {
  Gestures gesture = getGesture();
  doGesture(gesture);
  delay(100);
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
    case GES_FORWARD_FLAG:
      value = "Forward";
      break;
    case GES_BACKWARD_FLAG:
      value = "Backward";
      break;
    case GES_CLOCKWISE_FLAG:
      value = "ClockWise";
      break;
    case GES_COUNT_CLOCKWISE_FLAG:
      value = "AnticlockWise";
      break;
//    case GES_WAVE_FLAG:
//      value = "Wave";
//      break;
    default:
      break;
  }
  delay(GES_QUIT_TIME);

  return value;
}
