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

;enum Gestures {
  unknown = 0,
  Left,
  Right,
  Up,
  Down,
  Forward,
  Backward,
  ClockWise,
  AnticlockWise,
  Wave
};

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

   if (!Firebase.beginStream(firebaseData, path)) {
    Serial.println("------Can't begin stream connection------");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println();
  }

  setPoles(0,Up);
  setPoles(0,Down);
  setPoles(0,Left);
  setPoles(0,Right);

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
  Gestures gesture = identifyGesture();
  doGesture(String(gesture));
  delay(100);
}

void doGesture(String temp) {
  call = getSwipe("swipe" + temp + "Num");
  switch (call) {
    case 1:
      digitalWrite(A1,1);
    case 2:
    case 3:
    case 4:
    default:
      break;
  }
}

void setPoles(int num, String temp) {
  if (Firebase.setFloat(firebaseData, "/setPoles/" + temp, num)) {
  if (firebaseData.dataType() == "float")
    Serial.println(firebaseData.floatData());
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

int getSwipe(String path) {
  if (Firebase.getInt(firebaseData, "/setGestures/" + path)) {
    if (firebaseData.dataType() == "int")) {
      temp = firebaseData.intData();
      Serial.println(temp);
      return temp
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }
}

Gestures identifyGesture() {
  uint8_t data = 0;
  uint8_t data1 = 0;
  uint8_t error = 0;

  Gestures returnVal = unknown;

  error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if (error) return returnVal;

  switch (data) {                  // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
    case GES_RIGHT_FLAG:
      delay(GES_ENTRY_TIME);
      paj7620ReadReg(0x43, 1, &data);
      if (data == GES_FORWARD_FLAG) {
        Serial.println("Forward");
        r=1;
        l=1;
        u=1;
        d=1;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        Serial.println(r);
        Serial.println(l);
        Serial.println(u);
        Serial.println(d);
        return Wave;
        delay(GES_QUIT_TIME);
      }
      else if (data == GES_BACKWARD_FLAG)
      {
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
      }
      else
      {
        Serial.println("Right");

        r=(r+1)%2;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        Serial.println(r);
        return Right;
      }
      break;
    case GES_LEFT_FLAG:
      delay(GES_ENTRY_TIME);
      paj7620ReadReg(0x43, 1, &data);
      if (data == GES_FORWARD_FLAG)
      {
        Serial.println("Forward");
        r=1;
        l=1;
        u=1;
        d=1;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        Serial.println(r);
        Serial.println(l);
        Serial.println(u);
        Serial.println(d);
        delay(GES_QUIT_TIME);
      }
      else if (data == GES_BACKWARD_FLAG)
      {
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
      }
      else
      {
        Serial.println("Left");
        l=(l+1)%2;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        Serial.println(l);
        return Left;
      }
      break;
    case GES_UP_FLAG:
      delay(GES_ENTRY_TIME);
      paj7620ReadReg(0x43, 1, &data);
      if (data == GES_FORWARD_FLAG)
      {
        Serial.println("Forward");
        r=1;
        l=1;
        u=1;
        d=1;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        Serial.println(r);
        Serial.println(l);
        Serial.println(u);
        Serial.println(d);
        delay(GES_QUIT_TIME);
      }
      else if (data == GES_BACKWARD_FLAG)
      {
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
      }
      else
      {
        Serial.println("Up");
        u=(u+1)%2;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        Serial.println(u);
        return Up;
      }
      break;
    case GES_DOWN_FLAG:
      delay(GES_ENTRY_TIME);
      paj7620ReadReg(0x43, 1, &data);
      if (data == GES_FORWARD_FLAG)
      {
        Serial.println("Forward");
        r=1;
        l=1;
        u=1;
        d=1;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        Serial.println(r);
        Serial.println(l);
        Serial.println(u);
        Serial.println(d);
        delay(GES_QUIT_TIME);
      }
      else if (data == GES_BACKWARD_FLAG)
      {
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
      }
      else
      {
        Serial.println("Down");
        d=(d+1)%2;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        Serial.println(d);
        return Down;
      }
      break;
    case GES_FORWARD_FLAG:
      Serial.println("Forward");
      delay(GES_QUIT_TIME);
      return Forward;
      break;
    case GES_BACKWARD_FLAG:
      Serial.println("Backward");
      delay(GES_QUIT_TIME);
      return Backward;
      break;
    case GES_CLOCKWISE_FLAG:
      Serial.println("Clockwise");
      return ClockWise;
      break;
    case GES_COUNT_CLOCKWISE_FLAG:
      Serial.println("anti-clockwise");
      return AnticlockWise;
      break;
    default:
      paj7620ReadReg(0x44, 1, &data1);
      if (data1 == GES_WAVE_FLAG) {
        Serial.println("wave");
        r=0;
        l=0;
        u=0;
        d=0;
        digitalWrite(A1, r); //right
        digitalWrite(A2, l); //left
        digitalWrite(A3, u); //up
        digitalWrite(A4, d); //down
        return Wave;
      }
      break;
  }
}
