#include <Wire.h>
#include "paj7620.h"
#include <Firebase_Arduino_WiFiNINA.h>
#include "secrets.h"

FirebaseData firebaseData;

/*
  Notice: When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
        You also can adjust the reaction time according to the actual circumstance.
*/
#define GES_REACTION_TIME 500       // You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME 800       // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
#define GES_QUIT_TIME 500

/*******************************************************************************/

int count;
String gesture;
byte data;
int Poles[] = {0,0,0,0};
int Gestures[] = {0,0,0,0}; //{UP,DOWN,LEFT,RIGHT}

void getGesture(String* value,int* num);
void checkOverride();
void setOverride();
void statePole(String path);
void switchPole(String path);
int getPoles(String path);
void setPoles(int num, String path);
String getSwipe(String path);
boolean getReset();
void setReset();

/*******************************************************************************/
void setup() {

  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);
  pinMode(A5,OUTPUT);
  pinMode(0,OUTPUT);
  pinMode(A6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(6,OUTPUT);

  digitalWrite(A5,HIGH);

  Serial.begin(9600);
  delay(500);
  Serial.println();

  Serial.print("Connecting to WiFi…");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(SECRET_WIFISSID, SECRET_WIFIPASSWORD);
    Serial.print(".");
    delay(300);
  }
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(SECRET_FIREBASEHOST, SECRET_FIREBASEAUTH, SECRET_WIFISSID, SECRET_WIFIPASSWORD);
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
//MAIN PROGRAM LOOP
void loop() {
  if(true == getReset()) {
      setReset();
      Serial.println("Reset in progress...");
      delay(1000);
      digitalWrite(A5, LOW);
  }
  while(1) {
    getGesture(&gesture,&count); //returns a count and the gesture as a String
    //if a gesture was found and it enters the case and leaves with default values.
    if(count == 1) {
      Serial.println("Gesture: " + gesture);
      switchPole(getSwipe(gesture));
      count--;
      }
      checkOverride();
  }

}
/*******************************************************************************/

void checkOverride() {
  if (Firebase.getInt(firebaseData, "/alterPoles/overrideFlag")) {
    if (firebaseData.dataType() == "int") {
      int temp = firebaseData.intData();
      if(temp == 1) {
        if(Poles[0] != getPoles("One")) { statePole("One");}
        if(Poles[1] != getPoles("Two")) { statePole("Two");}
        if(Poles[2] != getPoles("Three")) { statePole("Three");}
        if(Poles[3] != getPoles("Four")) { statePole("Four");}
        setOverride();
        return;
      } else {
        return;
      }
    }
  } else {
    Serial.println("FireBase Err 1: " +firebaseData.errorReason());
  }
}

/*******************************************************************************/

void setOverride() {
  if (Firebase.setInt(firebaseData, "/alterPoles/overrideFlag", 0)) {
    if (firebaseData.dataType() == "int") {
      //Serial.println(firebaseData.intData());
    } else {
      Serial.println("FireBase Err 2: " +firebaseData.errorReason());
    }
  }
} 

/*******************************************************************************/
//Takes the string of the pole that will be changed and sets it to the opposite
//of its current state.
void statePole(String path) {
  int call = getPoles(path);
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
//Takes the string of the pole that will be changed and sets it to the opposite
//of its current state.
void switchPole(String path) {
  if(path == "None") {return;}
  Serial.println("SwitchPoles was Run. The Path is " + path);
  int call = getPoles(path);
  call = !call;
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
    Serial.println("FireBase Err 3: " +firebaseData.errorReason());
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
    if (firebaseData.dataType() == "boolean") {
      //Serial.println(firebaseData.boolData());
    } else {
      Serial.println("FireBase Err 4: " +firebaseData.errorReason());
    }
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
    Serial.println("FireBase Err 5: " +firebaseData.errorReason());
  }
}

/*******************************************************************************/

boolean getReset() {
  bool temp;
  if (Firebase.getBool(firebaseData, "/Reset/reset")) {
    if (firebaseData.dataType() == "boolean") {
      temp = firebaseData.boolData();
      return temp;
    }
  } else {
    Serial.println("FireBase Err 6: " +firebaseData.errorReason());
  }
}

/*******************************************************************************/

void setReset() {
  if (Firebase.setBool(firebaseData, "/Reset/reset", false)) {
    if (firebaseData.dataType() == "boolean") {
      //Serial.println(firebaseData.boolData());
    } else {
      Serial.println("FireBase Err 7: " +firebaseData.errorReason());
    }
  }
}

/*******************************************************************************/

void getGesture(String* value,int* num) {
  *value = "unknown";
  uint8_t error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if(error) {return;}
  switch (data) {                 // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
    case GES_RIGHT_FLAG:
      *value = "Right";
      Gestures[4] = !Gestures[4];
      digitalWrite(6,Gestures[4]);
      *num = 1;
      break;
    case GES_LEFT_FLAG:
      *value = "Left";
      Gestures[3] = !Gestures[3];
      digitalWrite(7,Gestures[3]);
      *num = 1;
      break;
    case GES_UP_FLAG:
      *value = "Up";
      Gestures[1] = !Gestures[1];
      digitalWrite(0,Gestures[1]);
      *num = 1;
      break;
    case GES_DOWN_FLAG:
      *value = "Down";
      Gestures[2] = !Gestures[2];
      Serial.println(Gestures[2]);
      digitalWrite(A6,Gestures[2]);
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
  return;
}
