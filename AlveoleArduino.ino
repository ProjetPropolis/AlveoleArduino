// hardware libraries to access use the shield
#include "FastLED.h"
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include "HX711.h"
#include <Chrono.h>
#include <LightChrono.h>

//LED related variables
#define LED_PIN    22
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B
#define NUM_LEDS    80
#define NUM_STRIPS 7
#define BRIGHTNESS  255
#define FASTLED_ALLOW_INTERRUPTS 0
#define LATENCY_DELAY 400

//--time out of the tare function ?is it use?
#define TARE_TIMEOUT_SECONDS 4 

//--declare sensor pin and index for unity comprehension of received data

//byte DOUTS1[0] = {};
//byte DOUTS2[0] = {};

//--array of all the index that are going to be send, needed for unity to understand if we dont have all the tiles
int indexs[7] = {0,1,2,3,4,5,6};

//--pre calculate the different buffer needed

//--serial communication related variable
byte dataArray1[3] = {NULL, NULL, NULL};//the bigest number we will receive is in the 3 number;
byte dataArray2[3] = {NULL, NULL, NULL};//the bigest number we will receive is in the 3 number;
bool record = 0;
int dataBufferIndex = 0;
int lengthOfNbr1 = 0;
int lengthOfNbr2 = 0;

int receiveState[NUM_STRIPS];
int prevReceiveState[NUM_STRIPS];

//--define the channels
HX711 scale0(0,1);
HX711 scale1(3,4);
HX711 scale2(9,10);
HX711 scale3(11,12);
HX711 scale4(15,16);
HX711 scale5(17,18);
HX711 scale6(19,22);

//datas array use for calculating the outcome of the receive data
int threshold = 10000;
uint32_t prevValues[NUM_STRIPS];
int dataId = 419;
int dataState = 419;

bool sensorOrientation[NUM_STRIPS]; //IF TRUE = pressure plate is positive and go negative. IF FALSE = pressure plate is negative and go positive
long int sensorStartValue[NUM_STRIPS];
long int sensorThreshold[NUM_STRIPS]; //originaly not an array and has 10 000 has value
bool calibrationState = 0;
int countCalibrated = 0;

int resistance = 5; //Number of time the threshold is multiply. Basic threshold are 10% of their initial value and each resistance multiply that number
int currentSensor = 0;

int prevTileStatus[NUM_STRIPS];
int tileStatus[NUM_STRIPS];
int prevTilePressure[NUM_STRIPS];
int tilePressure[NUM_STRIPS];

//variable related to the behavior of the different state receive

/*
color array index signification: 
---index 1: off state
---index 2: on state
---index 3: corrupted state
---index 4: ultra corrupted state 
---index 5: cleanse state
*/
int colorArray[5][3] = { {25,25,25}, {75,75,0}, {50,0,80}, {155,25,25}, {0,85,112} };

CRGB leds[NUM_STRIPS][NUM_LEDS];
int currentId = 0;

//variable related to the sensor data analyse

uint32_t maxReadableValue = 2000000; // value to be change need testing
int sensorRawValue[NUM_STRIPS];
int pressureState[NUM_STRIPS];

/*==== sub-hexStatus manager ===*/
Chrono masterChrono0_On;
Chrono masterChrono1_On;
Chrono masterChrono2_On;
Chrono masterChrono3_On;
Chrono masterChrono4_On;
Chrono masterChrono5_On;
Chrono masterChrono6_On;
Chrono masterChrono_On[NUM_STRIPS] = {masterChrono0_On, masterChrono1_On, masterChrono2_On, masterChrono3_On, masterChrono4_On, masterChrono5_On, masterChrono6_On};
Chrono masterChrono0_Corrupt;
Chrono masterChrono1_Corrupt;
Chrono masterChrono2_Corrupt;
Chrono masterChrono3_Corrupt;
Chrono masterChrono4_Corrupt;
Chrono masterChrono5_Corrupt;
Chrono masterChrono6_Corrupt;
Chrono masterChrono_Corrupt[NUM_STRIPS] = {masterChrono0_Corrupt, masterChrono1_Corrupt, masterChrono2_Corrupt, masterChrono3_Corrupt, masterChrono4_Corrupt, masterChrono5_Corrupt, masterChrono6_Corrupt};
Chrono masterChrono0_Ultracorrupt;
Chrono masterChrono1_Ultracorrupt;
Chrono masterChrono2_Ultracorrupt;
Chrono masterChrono3_Ultracorrupt;
Chrono masterChrono4_Ultracorrupt;
Chrono masterChrono5_Ultracorrupt;
Chrono masterChrono6_Ultracorrupt;
Chrono masterChrono_Ultracorrupt[NUM_STRIPS] = {masterChrono0_Ultracorrupt, masterChrono1_Ultracorrupt, masterChrono2_Ultracorrupt, masterChrono3_Ultracorrupt, masterChrono4_Ultracorrupt, masterChrono5_Ultracorrupt, masterChrono6_Ultracorrupt};
int pastHexStatus = 0;
bool needReset[NUM_STRIPS] = {0,0,0,0,0,0,0};
bool unlocked[] = {true, true, true, true, true, true, true};

/*==== preOn() Variables ===*/
int ledIndex_PreOn[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};
int ledAnimIndex_PreOn[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};

/*==== on() Variables ===*/
int ledIndex_On[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};
int ledAnimIndex_On[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};

/*==== off() Variables ===*/
int ledIndex_Off[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};

/*==== corrupt() Variables ===*/
Chrono myChrono0_Corrupt;
Chrono myChrono1_Corrupt;
Chrono myChrono2_Corrupt;
Chrono myChrono3_Corrupt;
Chrono myChrono4_Corrupt;
Chrono myChrono5_Corrupt;
Chrono myChrono6_Corrupt;
Chrono myChrono_Corrupt[NUM_STRIPS] = {myChrono0_Corrupt, myChrono1_Corrupt, myChrono2_Corrupt, myChrono3_Corrupt, myChrono4_Corrupt, myChrono5_Corrupt, myChrono6_Corrupt};
float val_Corrupt[NUM_STRIPS] = {255, 255, 255, 255, 255, 255, 255};
int stateBrightness_Corrupt[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};
int ledIndexGlitch1_Corrupt[NUM_STRIPS];
int ledIndexGlitch2_Corrupt[NUM_STRIPS];
int ledIndexGlitch3_Corrupt[NUM_STRIPS];
int ledIndexGlitch4_Corrupt[NUM_STRIPS];
int ledIndexGlitch5_Corrupt[NUM_STRIPS];
float delayBrightness_Corrupt = 0.3;
int delayIndex_Corrupt = 30;
int dashLenght_Corrupt = 5;
int stepRandom_Corrupt = 1;

/*==== ultracorrupt() Variables ===*/
Chrono myChrono0_Ultracorrupt;
Chrono myChrono1_Ultracorrupt;
Chrono myChrono2_Ultracorrupt;
Chrono myChrono3_Ultracorrupt;
Chrono myChrono4_Ultracorrupt;
Chrono myChrono5_Ultracorrupt;
Chrono myChrono6_Ultracorrupt;
Chrono myChrono_Ultracorrupt[NUM_STRIPS] = {myChrono0_Ultracorrupt, myChrono1_Ultracorrupt, myChrono2_Ultracorrupt, myChrono3_Ultracorrupt, myChrono4_Ultracorrupt, myChrono5_Ultracorrupt, myChrono6_Ultracorrupt};
float val_Ultracorrupt[NUM_STRIPS] = {255, 255, 255, 255, 255, 255, 255};
//float brightness_Ultracorrupt[7] = {0, 0, 0, 0, 0, 0, 0};
int stateBrightness_Ultracorrupt[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};
int ledIndexGlitch1_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch2_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch3_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch4_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch5_Ultracorrupt[NUM_STRIPS];
int delayIndex_Ultracorrupt = 750;
float delayBrightness_Ultracorrupt = 0.75;
int dashLenght_Ultracorrupt = 4;

/*==== ultracorruptPressed() Variables ===*/
Chrono myChrono0_UltracorruptPressed;
Chrono myChrono1_UltracorruptPressed;
Chrono myChrono2_UltracorruptPressed;
Chrono myChrono3_UltracorruptPressed;
Chrono myChrono4_UltracorruptPressed;
Chrono myChrono5_UltracorruptPressed;
Chrono myChrono6_UltracorruptPressed;
Chrono myChrono_UltracorruptPressed[NUM_STRIPS] = {myChrono0_UltracorruptPressed, myChrono1_UltracorruptPressed, myChrono2_UltracorruptPressed, myChrono3_UltracorruptPressed, myChrono4_UltracorruptPressed, myChrono5_UltracorruptPressed, myChrono6_UltracorruptPressed};
float val_UltracorruptPressed[NUM_STRIPS] = {255, 255, 255, 255, 255, 255, 255};
//float brightness_Ultracorrupt[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};
int delayIndex_UltracorruptPressed = 50;
int isPressed_UltracorruptPressed[] = {0, 0, 0, 0, 0, 0, 0};

/*==== cleanser() Variables ===*/
Chrono myChrono0_Cleanser;
Chrono myChrono1_Cleanser;
Chrono myChrono2_Cleanser;
Chrono myChrono3_Cleanser;
Chrono myChrono4_Cleanser;
Chrono myChrono5_Cleanser;
Chrono myChrono6_Cleanser;
Chrono myChrono_Cleanser[NUM_STRIPS] = {myChrono0_Cleanser, myChrono1_Cleanser, myChrono2_Cleanser, myChrono3_Cleanser, myChrono4_Cleanser, myChrono5_Cleanser, myChrono6_Cleanser};
int ledIndex1_Cleanser[NUM_STRIPS];
int ledIndex2_Cleanser[NUM_STRIPS];
int stateLedIndex1_Cleanser[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};
//int stateLedIndex2_Cleanser[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};
int delayProgressLed_Cleanser = 10;
bool state1First_Cleanser = true;
bool state2First_Cleanser = true;
bool state3First_Cleanser = true;
bool state4First_Cleanser = true;
bool state1Second_Cleanser = true;
bool state2Second_Cleanser = true;
bool state3Second_Cleanser = true;
bool state4Second_Cleanser = true;

/*==== cleansing() Variables ===*/
int ledIndex1_Cleansing[NUM_STRIPS];
int ledIndex2_Cleansing[NUM_STRIPS];
int stateLedIndex1_Cleansing[NUM_STRIPS] = {0, 0, 0, 0, 0, 0, 0};
int delayProgressLed_Cleansing = 10;
bool state1First_Cleansing = true;
bool state2First_Cleansing = true;
bool state3First_Cleansing = true;
bool state4First_Cleansing = true;
bool state1Second_Cleansing = true;
bool state2Second_Cleansing = true;
bool state3Second_Cleansing = true;
bool state4Second_Cleansing = true;

int ledIndex_ANIM_TURQUOISE[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_PURPLE[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_YELLOW[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_ORANGE[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_GREEN[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_BLACK[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_TURQUOISE_FADE[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_SNAKE_TURQUOISE[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_SNAKE_YELLOW[] = {0, 0, 0, 0, 0, 0, 0};
int ledIndex_ANIM_GREEN_TURQUOISE[] = {0, 0, 0, 0, 0, 0, 0};

/*==== ANIM_PURPLELIT() Variables ===*/
bool switch_PURPLE_WIPE[] = {true, true, true, true, true, true, true};

/*==== ANIM_TURQUOISE_FADE() Variables ===*/
float hue_TURQUOISE_FADE[NUM_STRIPS] = {185, 185, 185, 185, 185, 185, 185};
float delayHue_TURQUOISE_FADE = 1;

/*==== ANIM_SNAKE_TURQUOISE() Variables ===*/
float val_SNAKE_TURQUOISE[NUM_STRIPS] = {255, 255, 255, 255, 255, 255, 255};
float delayBrightness_SNAKE_TURQUOISE = 1.5;

/*==== ANIM_SNAKE_YELLOW() Variables ===*/
float hue_SNAKE_YELLOW[NUM_STRIPS] = {64, 64, 64, 64, 64, 64, 64};
bool state_SNAKE_YELLOW[] = {true, true, true, true, true, true, true};
float delayHue_SNAKE_YELLOW = 1;

/*==== colors Variables ===*/
CRGB empty_off(0, 0, 0);
CRGB yellow_On(190, 155, 15);
//CRGB yellow_On(200, 110, 15);
CRGB orange_On(70, 0, 0);
//CRGB orange_On(70, 0, 0);
CHSV purple_Corrupt(185, 255, 255);
//CHSV purple_Corrupt(210, 255, 255);
CHSV palePurple_Corrupt(195, 50, 255);
//CHSV palePurple_Corrupt(210, 50, 255);
CRGB red_Ultracorrupt(255, 0, 0);
CRGB white_Ultracorrupt(255, 255, 255);
CHSV blue_Cleanser(140, 255, 255);
CRGB cyan_ShieldOn(0, 50, 170);
CHSV paleBlue_Cleansing(140, 200, 255);
CRGB orange_ANIM(200, 80, 0);
CHSV purpleLit_PURPLE_WIPE(180, 100, 255);
CRGB greenTurquoise_ANIM(0, 200, 125);
//CRGB greenTurquoise_ANIM(0, 200, 125);

void setup() {
  Serial.begin(115200);
  //setup ethernet part
  delay(3000); // sanity delay
  //Serial.println("serial begin");
  calibrateHex();
  for(int i = 0; i < NUM_STRIPS; i++){
    //Initialise array of value
    tileStatus[i] = 0;
    prevTileStatus[i] = 0;
    prevValues[i] = 0;
    prevTilePressure[i] = 0;
    tilePressure[i] = 0;

    //print the calibration stats
    Serial.print("value #");
    Serial.print(i);
    Serial.print(" is set as : ");
    Serial.print(sensorStartValue[i]);
    Serial.print(" , while go in the direction: ");
    Serial.print(int(sensorOrientation[i]));
    Serial.print(" and while have a threshold of: ");
    Serial.println(sensorThreshold[i]);
  }
  Serial.flush();
  
  for(int i = 0; i < 7; i++){
    receiveState[i] = 0; 
    prevReceiveState[i] = 0;   
  }

  // Settings up the led strips
  FastLED.addLeds<CHIPSET, 2, COLOR_ORDER>(leds[0], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 14, COLOR_ORDER>(leds[1], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 7, COLOR_ORDER>(leds[2], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 8, COLOR_ORDER>(leds[3], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 20, COLOR_ORDER>(leds[4], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 6, COLOR_ORDER>(leds[5], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 21, COLOR_ORDER>(leds[6], NUM_LEDS).setCorrection( TypicalLEDStrip );

  
  //tare();
  //testPattern();

}

//int tick = 0;
void loop() {
  // important! non-blocking listen routine
  
  if(calibrationState == 0){
    calibrateHex();
  }
  if(receiveState[dataId] < 0 || receiveState[dataId] > 35){
    receiveState[dataId] = 0;
  }
  mustReadPressure(currentSensor);
  currentSensor = (currentSensor + 1) % NUM_STRIPS;
  readTheData();
  for(int i = 0; i < NUM_STRIPS; i++){
    if(receiveState[i] != prevReceiveState[i]){
      int stripState = receiveState[i];
      Serial.println("update led: " + String(receiveState[i]));
      //stateCtrl(i,stripState, prevReceiveState[i]);
    }
    stateCtrl(i,receiveState[i], prevReceiveState[i]);
    if(receiveState[i] != prevReceiveState[i]){
      prevReceiveState[i] = receiveState[i];
    }
  }
  
  FastLED.show();
  delay(2); 
}

void calibrateHex(){
for(int i = 0; i< NUM_STRIPS; i++){
    long int sensorValue;
    switch(i){
      case 0:
        if ( scale0.is_ready() == false ) break;
        sensorValue = scale0.read();
        calibrateSensor(sensorValue,i);
        break;
      case 1:
        if ( scale1.is_ready() == false ) break;
        sensorValue = scale1.read();
        calibrateSensor(sensorValue,i);
        break;
      case 2:
        if ( scale2.is_ready() == false ) break;
        sensorValue = scale2.read();
        calibrateSensor(sensorValue,i);
        break;
      case 3:
        if ( scale3.is_ready() == false ) break;
        sensorValue = scale3.read();
        calibrateSensor(sensorValue,i);
        break;
      case 4:
        if ( scale4.is_ready() == false ) break;
        sensorValue = scale4.read();
        calibrateSensor(sensorValue,i);
        break;
      case 5:
        if ( scale5.is_ready() == false ) break;
        sensorValue = scale5.read();
        calibrateSensor(sensorValue,i);
        break;
      case 6:
        if ( scale6.is_ready() == false ) break;
        sensorValue = scale6.read();
        calibrateSensor(sensorValue,i);
        break;
    }
    if(sensorValue != 0){
      countCalibrated = countCalibrated + 1;
    }
  }
  if(countCalibrated == 7){
    calibrationState = 1;
  } else{
    countCalibrated = 0;
  }
}

void calibrateSensor(long int value, int id) {
  long int sensorValue = value;
  long int theThreshold;
  long int toRemove;
  int sensorId = id;
  if(sensorValue > 0){
    theThreshold = (sensorValue * 0.1) * resistance ;
    theThreshold = abs(theThreshold);
    sensorOrientation[sensorId] = true;
  }else{
    
    theThreshold = (sensorValue * 0.1) * resistance ;
    theThreshold = abs(theThreshold);
    sensorOrientation[sensorId] = false;
  }
  if(theThreshold < 15000){
    sensorThreshold[sensorId] = int(threshold);
  }else{
    sensorThreshold[sensorId] = theThreshold;
  }
  
  sensorStartValue[sensorId] = sensorValue;
}

bool mustReadPressure (int sensorId){
  long int value;
  bool valid = false;
  switch(sensorId){
      case 0:
        valid = scale0.readNonBlocking(&value);
        break;
      case 1:
        valid = scale1.readNonBlocking(&value);
        break;
      case 2:
        valid = scale2.readNonBlocking(&value);
        break;
      case 3:
        valid = scale3.readNonBlocking(&value);
        break;
      case 4:
        valid = scale4.readNonBlocking(&value);
        break;
      case 5:
        valid = scale5.readNonBlocking(&value);
        break;
      case 6:
        valid = scale6.readNonBlocking(&value);
        break;
  }
  if (valid && unlocked[sensorId]){
    readPressurePlate(value, sensorId);
  }
}
void readTheData() {
  decipherPacket();
  int theIndex = dataId;
  //---- a verifier -----
  for(int i = 0; i < NUM_STRIPS; i++){
    int indexReceive = theIndex;
    int indexToSend = indexs[i];
    if(indexReceive == indexToSend){
      currentId = indexs[i]; 
    } 
  }
  int hexStatus = receiveState[dataId];
}

void decipherPacket(){
  int state;
  while (Serial.available() > 0) {
    byte c = Serial.read();
    if(c == 97){  //97 ascii = a
      //Serial.println("packet start");
      record = 1;
      dataBufferIndex = 1;
    }
    else if(c == 99){  //99 ascii = c
      int bufferId = String((char*)dataArray1).toInt();
      memcpy (&dataId, &bufferId, sizeof(bufferId));
      for(int i = 0; i<dataArray1[3]; i++){
        dataArray1[i] = NULL;
      }
      dataBufferIndex = 2;
    }
    else if(c == 122){  //122 ascii = z
      //Serial.println("packet end");
      int bufferState = String((char*)dataArray2).toInt();
      //Serial.println("packetEnd value of buffer: " + bufferState);
      memcpy (&receiveState[dataId], &bufferState, sizeof(bufferState));

      for(int i = 0; i<sizeof(dataArray2); i++){
        dataArray2[i] = NULL;
      }
      /*
      //TODO Confirm Ok : changed this loop for above
      for(int i = 0; i<dataArray2[3]; i++){
        dataArray2[i] = NULL;
      }
      */
      lengthOfNbr1 = 0;
      lengthOfNbr2 = 0;
      record = 0;
      dataBufferIndex = 0;
    }
    else if(record != 0){
      byte theValue;
      if(dataBufferIndex == 1){
        dataArray1[lengthOfNbr1] = c;
        lengthOfNbr1 = lengthOfNbr1 + 1;
      }
      else if(dataBufferIndex == 2){
        dataArray2[lengthOfNbr2] = c;
        lengthOfNbr2 = lengthOfNbr2 + 1;
      }
    }
    
    for(int i = 0; i < NUM_STRIPS; i++){
      
      if(receiveState[i] != prevReceiveState[i]){
        //Check if locked
        if(receiveState[i] == 15){
          //DETECTION_OFF
          unlocked[i] = false;
        }else if(receiveState[i] == 16){
          //DETECTION_ON
          unlocked[i] = true;
        }
        resetTile(i);  
        int stripState = receiveState[i];
        Serial.println("update led: " + String(stripState));
        //if(stripState != 15 && stripState != 16){
        stateCtrl(i, stripState, prevReceiveState[i]);
        //}
      }
      //prevReceiveState[i] = receiveState[i];
    }
    FastLED.show();
    //Serial.println("dataId : " + String(dataId));
    //Serial.println("dataState : " + String(dataState));
  }
}

void sendHexStatus(int ID, int state){
  int hexId = ID;
  int hexState = indexs[state];
  Serial.println((String)hexId + "," + hexState + "e/hexData");  
}

void readPressurePlate(long int val, int id){
  int index = id;
  long int currentValue = val;
  long int difference;
  
  if(sensorOrientation[index]){
    if(currentValue > 0){
      difference = sensorStartValue[index] - currentValue;
      difference = abs(difference);
    }else{
      difference = sensorStartValue[index] + abs(currentValue);
      difference = abs(difference);
    }
    if(difference > sensorThreshold[index]){
      tilePressure[index] = true;
    }
    else{ 
      tilePressure[index] = false; 
    }
  }else{
    long int startValue = abs(sensorStartValue[index]);
    if(currentValue > 0){
      difference =  currentValue + startValue ;
      difference = abs(difference);
    }else{
      difference = sensorStartValue[index] - currentValue;
      difference = abs(difference);
    }
    difference =  currentValue + startValue ;
    difference = abs(difference);
    if(difference > sensorThreshold[index]){
      tilePressure[index] = true;
    }
    else{ 
      tilePressure[index] = false; 
    }
  }
  
  if(tilePressure[index] != prevTilePressure[index]){
    if(tilePressure[index]){
      Serial.print("sensor #");
      Serial.print(index);
      Serial.print("is trigger with difference of : ");
      Serial.print(difference);
      Serial.print(" > then the threshold of: ");
      Serial.println(sensorThreshold[index]);
      tileStatus[index] = 1;
      isPressed_UltracorruptPressed[index] = 1;
      sendHexStatus(index,1);
      predictGameplay(index,prevReceiveState[index]);
    }else{
      Serial.print("sensor #");
      Serial.print(index);
      Serial.print("is no longer trigger with difference of : ");
      Serial.print(difference);
      Serial.print(" > then the threshold of: ");
      Serial.println(sensorThreshold[index]);
      tileStatus[index] = 0;
      isPressed_UltracorruptPressed[index] = 0;
      sendHexStatus(index,0);
      predictGameplay(index,prevReceiveState[index]);
    }
  }
  prevTileStatus[index] = tileStatus[index];
  prevTilePressure[index] = tilePressure[index];

  if(tileStatus[index] == 1 && tilePressure[index] != 1){
    Serial.println("entering error catcher returning 0 to unity");
    sendHexStatus(index,0);
  }
}

void predictGameplay(int id, int prevState){
  Serial.print("enter predict with id of ");
  Serial.print(id);
  Serial.print(" and prevState of ");
  Serial.println(prevState);
  int theState;
  switch(prevState){
    case 0:
      Serial.println("enter case0");
      theState =1;
      receiveState[id] = theState;
      stateCtrl(id,theState,prevState);
      FastLED.show();
      return;
    case 1:
      Serial.println("enter case1");
      theState =1;
      receiveState[id] = theState;
      stateCtrl(id,theState,prevState);
      FastLED.show();
      return;
    case 2:
      Serial.println("enter case2");
      theState = 1;
      receiveState[id] = theState;
      stateCtrl(id,theState,prevState);
      FastLED.show();
      return;
    case 3:
      Serial.print("enter case3: ");
      Serial.println(id);
      //ultracorruptPressed(id);
      return;
    case 4:
      Serial.println("enter case4");
      theState = 1;
      receiveState[id] = theState;
      stateCtrl(id,theState,prevState);
      FastLED.show();
      return;
  }
}

void stateCtrl(int id, int state, int prevState){

  //Resets climax's anim when it's the alveole's prevState
  if(prevState == 23 && state != 23){
    hue_TURQUOISE_FADE[id] = 185;  
  }
  if(prevState == 24 && state != 24){
    val_SNAKE_TURQUOISE[id] = 255;
  }
  if(prevState == 25 && state != 25){
    hue_SNAKE_YELLOW[id] = 64;
    state_SNAKE_YELLOW[id] = true;
  }
  
  switch( state ) {
    case 0: 
      off(id);
      break;
    case 1: 
      needReset[id] = 1;
      if(prevState == 2){
        masterChrono_On[id].restart();
      }
      if(!masterChrono_On[id].hasPassed(500)){
        preOn(id);
      }else{
        on(id);
      } 
      break;
    case 2:  
      if(prevState == 1){
        masterChrono_Corrupt[id].restart();
        needReset[id] = 1;
      }
      if(!masterChrono_Corrupt[id].hasPassed(3000)){
        on(id);
        corrupt(id);
      }else{
        if(needReset[id] != 0){
          resetTile(id);
          needReset[id] = 0;
        }else{
          corrupt(id);
        }
      }
      break;
    case 3:
      if(needReset[id] != 0){
        resetTile(id);
        needReset[id] = 0;
      }
      if(isPressed_UltracorruptPressed[id] == 0){
        ultracorrupt(id);
      }else{
        ultracorruptPressed(id);
      }
      break;
    case 4: 
      cleanser(id);
      break;
    case 5: 
      //Serial.print("call on id: "); Serial.print(id); Serial.print(" with state: "); Serial.print(state); Serial.print(" and prevState of: "); Serial.print(prevState); Serial.println(" in case 5");
      cleansing(id);
      break;
    case 15: 
      if(needReset[id] != 0){
        resetTile(id);
        needReset[id] = 0;
      }else{
        corrupt(id);
      }
      break;
    case 16: 
      if(needReset[id] != 0){
        resetTile(id);
        needReset[id] = 0;
      }else{
        corrupt(id);
      }
      break;
    case 17: 
      ANIM_TURQUOISE(id);
      break;
    case 18: 
      ANIM_PURPLE(id);
      break;
    case 19: 
      ANIM_YELLOW(id);
      break;
    case 20: 
      ANIM_ORANGE(id);
      break;
    case 21: 
      ANIM_PURPLE_WIPE(id);
      break;
    case 22: 
      ANIM_BLACK(id);
      break;
    case 23: 
      ANIM_TURQUOISE_FADE(id);
      break;
    case 24: 
      ANIM_SNAKE_TURQUOISE(id);
      break;
    case 25: 
      ANIM_SNAKE_YELLOW(id);
      break;
    case 26: 
      ANIM_GREEN_TURQUOISE(id);
      break;
  }
}

void resetTile(int id){
  for(int i=0; i<NUM_LEDS; i++){
    leds[id][i] = empty_off;
  }
}

//all the led animation for the different state
void off(int id){

  //Writes a Full Black Strip 
  
  if(ledIndex_Off[id] < NUM_LEDS){
    leds[id][ledIndex_Off[id]] = yellow_On;
    ledIndex_Off[id]++;
  }else{
    ledIndex_Off[id] = 0;
  }
  
}

void preOn(int id){
    //Whipes 1 orange for 3 yellow all over the strip
    /*
    for(int i = 0; i < NUM_LEDS; i+=4){
      leds[id][i] = yellow_On;
      leds[id][i+1] = yellow_On;
      leds[id][i+2] = yellow_On;
      leds[id][i+3] = orange_On;
    }
    */
    if(ledIndex_PreOn[id] < NUM_LEDS){
      //Writes dash of yellow
      for(int i = 0; i < 15; i++){
        if(i < 8){
          leds[id][ledIndex_PreOn[id]+i] = yellow_On;  
        }else{
          leds[id][ledIndex_PreOn[id]+i] = orange_On; 
        }  
      }
      ledIndex_PreOn[id]+=16;
    }else{
      ledIndex_PreOn[id] = 0;
    }
}

void on(int id){

     //Writes yellow 1 LED after the other and 1 orange LED every 2 LEDs
     //val_SNAKE_TURQUOISE[id] = 255;
     
     if(ledIndex_On[id] < NUM_LEDS){
      leds[id][ledIndex_On[id]] = yellow_On;
      ledIndex_On[id]++;
     }else{
      ledIndex_On[id] = 0;
     }
     
     if(ledAnimIndex_On[id] < NUM_LEDS){
      leds[id][ledAnimIndex_On[id]] = orange_On;
      ledAnimIndex_On[id]+= 2;
     }else{
      ledAnimIndex_On[id] = 0;
     }
     
}


void corrupt(int id){

    //Writes 5 dashes of purple in the strip, 5 index are generated(with random drunk) to change the dashes' positions

    //Writing BLACK for the 5 Dashes
    for(int i = 0; i < dashLenght_Corrupt; i++){
      if(i != 2){
        leds[id][ledIndexGlitch1_Corrupt[id] + i].setHSV(210, 255, 0);
        leds[id][ledIndexGlitch2_Corrupt[id] + i].setHSV(210, 255, 0);
      }
      leds[id][ledIndexGlitch3_Corrupt[id] + i].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch4_Corrupt[id] + i].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch5_Corrupt[id] + i].setHSV(210, 255, 0);
    }
    leds[id][ledIndexGlitch1_Corrupt[id]+2].setHSV(210, 255, 0);
    leds[id][ledIndexGlitch2_Corrupt[id]+2].setHSV(210, 255, 0);
    
    //Changing 5 Dashes starting index
    //if(myChrono_Corrupt[dataId].hasPassed(delayIndex_Corrupt)){
    ledIndexGlitch1_Corrupt[id] = constrain(random8(ledIndexGlitch1_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch1_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+4));
    ledIndexGlitch2_Corrupt[id] = constrain(random8(ledIndexGlitch2_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch2_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+4));
    ledIndexGlitch3_Corrupt[id] = constrain(random8(ledIndexGlitch3_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch3_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+4));
    ledIndexGlitch4_Corrupt[id] = constrain(random8(ledIndexGlitch4_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch4_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+4));
    ledIndexGlitch5_Corrupt[id] = constrain(random8(ledIndexGlitch5_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch5_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+4));
    //myChrono_Corrupt[id].restart();
    //}

    //Writing PURPLE for the 5 Dashes
    for(int i = 0; i < dashLenght_Corrupt; i++){
      if(i != 2){
        leds[id][ledIndexGlitch1_Corrupt[id] + i] = purple_Corrupt;
        leds[id][ledIndexGlitch2_Corrupt[id] + i] = purple_Corrupt;
      }
      leds[id][ledIndexGlitch3_Corrupt[id] + i] = purple_Corrupt;
      leds[id][ledIndexGlitch4_Corrupt[id] + i] = purple_Corrupt;
      leds[id][ledIndexGlitch5_Corrupt[id] + i] = purple_Corrupt;
    }
    leds[id][ledIndexGlitch1_Corrupt[id]+2] = palePurple_Corrupt;
    leds[id][ledIndexGlitch2_Corrupt[id]+2] = palePurple_Corrupt;

}


void ultracorrupt(int id){

    //Writes 5 dashes of red in the strip, 5 index are generated(with random) to change the dashes' positions
    
    //Writing BLACK for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      leds[id][ledIndexGlitch1_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[id][ledIndexGlitch2_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[id][ledIndexGlitch3_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[id][ledIndexGlitch4_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[id][ledIndexGlitch5_Ultracorrupt[id] + x].setHSV(0, 255, 0);
    }

    //Changing 5 Dashes starting index
    if(myChrono_Ultracorrupt[id].hasPassed(delayIndex_Ultracorrupt)){
      ledIndexGlitch1_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      ledIndexGlitch2_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      ledIndexGlitch3_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      ledIndexGlitch4_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      ledIndexGlitch5_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      myChrono_Ultracorrupt[id].restart();
    }

    //Writing RED for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      leds[id][ledIndexGlitch1_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[id][ledIndexGlitch2_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[id][ledIndexGlitch3_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[id][ledIndexGlitch4_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[id][ledIndexGlitch5_Ultracorrupt[id] + x] = red_Ultracorrupt;
      //FastLED.show();
    }
    
}


void ultracorruptPressed(int id){

    //Writing BLACK for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      leds[id][ledIndexGlitch1_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[id][ledIndexGlitch2_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[id][ledIndexGlitch3_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[id][ledIndexGlitch4_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[id][ledIndexGlitch5_Ultracorrupt[id] + x].setHSV(0, 255, 0);
    }

    //Changing 5 Dashes starting index
    //if(myChrono_UltracorruptPressed[id].hasPassed(delayIndex_UltracorruptPressed)){
    ledIndexGlitch1_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
    ledIndexGlitch2_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
    ledIndexGlitch3_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
    ledIndexGlitch4_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
    ledIndexGlitch5_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      //myChrono_Ultracorrupt[id].restart();
    //}

    //Writing RED for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      if(x != 2){
        leds[id][ledIndexGlitch1_Ultracorrupt[id] + x] = red_Ultracorrupt;
        leds[id][ledIndexGlitch2_Ultracorrupt[id] + x] = red_Ultracorrupt;  
        leds[id][ledIndexGlitch3_Ultracorrupt[id] + x] = red_Ultracorrupt;
      }
      leds[id][ledIndexGlitch4_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[id][ledIndexGlitch5_Ultracorrupt[id] + x] = red_Ultracorrupt;
      //FastLED.show();
    }
    leds[id][ledIndexGlitch1_Ultracorrupt[id] + 2] = purple_Corrupt;
    leds[id][ledIndexGlitch2_Ultracorrupt[id] + 2] = purple_Corrupt;
    leds[id][ledIndexGlitch3_Ultracorrupt[id] + 2] = purple_Corrupt;  
}


void cleanser(int id){
    
    if(stateLedIndex1_Cleanser[id] == 0 || stateLedIndex1_Cleanser[id] == 1){
     //Writing BLUE for the strip's first half from firstLED
     if((ledIndex1_Cleanser[id] < (NUM_LEDS*0.5)-1) && state1First_Cleanser){
       leds[id][ledIndex1_Cleanser[id]] = blue_Cleanser;
       ledIndex1_Cleanser[id]++;
     }else{
       leds[id][ledIndex1_Cleanser[id]] = blue_Cleanser;
       ledIndex1_Cleanser[dataId] = 0;
       stateLedIndex1_Cleanser[id]++;
       state1First_Cleanser = false;
     }
     //Writing BLUE for the strip's second half from middleLED
     if((ledIndex2_Cleanser[id] < NUM_LEDS-1) && state1Second_Cleanser){
       leds[id][ledIndex2_Cleanser[id]] = blue_Cleanser;
       ledIndex2_Cleanser[id]++;
     }else{
       leds[id][ledIndex2_Cleanser[id]] = blue_Cleanser;
       ledIndex2_Cleanser[id] = NUM_LEDS*0.5;
       stateLedIndex1_Cleanser[id]++;
       state1Second_Cleanser = false;
     }
    }else if(stateLedIndex1_Cleanser[id] == 2 || stateLedIndex1_Cleanser[id] == 3){
     //Writing BLACK for the strip's first half from firstLED
     if((ledIndex1_Cleanser[id] < (NUM_LEDS*0.5)-1) && state2First_Cleanser){
       leds[id][ledIndex1_Cleanser[id]].setHSV(140, 255, 0);
       ledIndex1_Cleanser[id]++;
     }else{
       leds[id][ledIndex1_Cleanser[id]].setHSV(140, 255, 0);
       ledIndex1_Cleanser[id] = (NUM_LEDS*0.5)-1;
       stateLedIndex1_Cleanser[id]++;
       state2First_Cleanser = false;
     }
     //Writing BLACK for the strip's second half from middleLED
     if((ledIndex2_Cleanser[id] < NUM_LEDS-1) && state2Second_Cleanser){
       leds[id][ledIndex2_Cleanser[id]].setHSV(140, 255, 0);
       ledIndex2_Cleanser[id]++;
     }else{
       leds[id][ledIndex2_Cleanser[id]].setHSV(140, 255, 0);
       ledIndex2_Cleanser[id] = NUM_LEDS-1;
       stateLedIndex1_Cleanser[id]++;
       state2Second_Cleanser = false;
     }
    }else if(stateLedIndex1_Cleanser[id] == 4 || stateLedIndex1_Cleanser[id] == 5){
     //Writing BLUE for the strip's first half from middleLED
     if((ledIndex1_Cleanser[id] > 0) && state3First_Cleanser){
       leds[id][ledIndex1_Cleanser[id]] = blue_Cleanser;
       ledIndex1_Cleanser[id]--;
     }else{
       leds[id][ledIndex1_Cleanser[id]] = blue_Cleanser;
       ledIndex1_Cleanser[id] = (NUM_LEDS*0.5)-1;
       stateLedIndex1_Cleanser[id]++;
       state3First_Cleanser = false;
     }
     //Writing BLUE for the strip's second half from lastLED
     if((ledIndex2_Cleanser[id] > NUM_LEDS*0.5) && state3Second_Cleanser){
       leds[id][ledIndex2_Cleanser[id]] = blue_Cleanser;
       ledIndex2_Cleanser[id]--;
     }else{
       leds[id][ledIndex2_Cleanser[id]] = blue_Cleanser;
       ledIndex2_Cleanser[id] = NUM_LEDS-1;
       stateLedIndex1_Cleanser[id]++;
       state3Second_Cleanser = false;
     }
    }else if(stateLedIndex1_Cleanser[id] == 6 || stateLedIndex1_Cleanser[id] == 7){
     //Writing BLACK for the strip's second half from middleLED
     if((ledIndex1_Cleanser[id] > 0) && state4First_Cleanser){
       leds[id][ledIndex1_Cleanser[id]].setHSV(140, 255, 0);
       ledIndex1_Cleanser[id]--;
     }else{
       leds[id][ledIndex1_Cleanser[id]].setHSV(140, 255, 0);
       ledIndex1_Cleanser[id] = 0;
       stateLedIndex1_Cleanser[id]++;
       state4First_Cleanser = false;
     }
     //Writing BLACK for the strip's second half from lastLED
     if((ledIndex2_Cleanser[id] > NUM_LEDS*0.5) && state4Second_Cleanser){
       leds[id][ledIndex2_Cleanser[id]].setHSV(140, 255, 0);
       ledIndex2_Cleanser[id]--;
     }else{
       leds[id][ledIndex2_Cleanser[id]].setHSV(140, 255, 0);
       ledIndex2_Cleanser[id] = NUM_LEDS*0.5;
       stateLedIndex1_Cleanser[id]++;
       state4Second_Cleanser = false;
     }
    }
    //Reset
    if(stateLedIndex1_Cleanser[id] >= 8){
     stateLedIndex1_Cleanser[id] = 0;
     state1First_Cleanser = true;
     state2First_Cleanser = true;
     state3First_Cleanser = true;
     state4First_Cleanser = true;
     state1Second_Cleanser = true;
     state2Second_Cleanser = true;
     state3Second_Cleanser = true;
     state4Second_Cleanser = true;
    }
}


void cleansing(int id){
    if(stateLedIndex1_Cleansing[id] == 0 || stateLedIndex1_Cleansing[id] == 1){
     //Writing BLUE for the strip's first half from firstLED
     if((ledIndex1_Cleansing[id] < (NUM_LEDS*0.5)-1) && state1First_Cleansing){
       leds[id][ledIndex1_Cleansing[id]] = paleBlue_Cleansing;
       ledIndex1_Cleansing[id]++;
     }else{
       leds[id][ledIndex1_Cleansing[id]] = paleBlue_Cleansing;
       ledIndex1_Cleansing[dataId] = 0;
       stateLedIndex1_Cleansing[id]++;
       state1First_Cleansing = false;
     }
     //Writing BLUE for the strip's second half from middleLED
     if((ledIndex2_Cleansing[id] < NUM_LEDS-1) && state1Second_Cleansing){
       leds[id][ledIndex2_Cleansing[id]] = paleBlue_Cleansing;
       ledIndex2_Cleansing[id]++;
     }else{
       leds[id][ledIndex2_Cleansing[id]] = paleBlue_Cleansing;
       ledIndex2_Cleansing[id] = NUM_LEDS*0.5;
       stateLedIndex1_Cleansing[id]++;
       state1Second_Cleansing = false;
     }
    }else if(stateLedIndex1_Cleansing[id] == 2 || stateLedIndex1_Cleansing[id] == 3){
     //Writing YELLOW for the strip's first half from firstLED
     if((ledIndex1_Cleansing[id] < (NUM_LEDS*0.5)-1) && state2First_Cleansing){
       leds[id][ledIndex1_Cleansing[id]] = yellow_On;
       ledIndex1_Cleansing[id]++;
     }else{
       leds[id][ledIndex1_Cleansing[id]] = yellow_On;
       ledIndex1_Cleansing[id] = (NUM_LEDS*0.5)-1;
       stateLedIndex1_Cleansing[id]++;
       state2First_Cleansing = false;
     }
     //Writing YELLOW for the strip's second half from middleLED
     if((ledIndex2_Cleansing[id] < NUM_LEDS-1) && state2Second_Cleansing){
       leds[id][ledIndex2_Cleansing[id]] = yellow_On;
       ledIndex2_Cleansing[id]++;
     }else{
       leds[id][ledIndex2_Cleansing[id]] = yellow_On;
       ledIndex2_Cleansing[id] = NUM_LEDS-1;
       stateLedIndex1_Cleansing[id]++;
       state2Second_Cleansing = false;
     }
    }else if(stateLedIndex1_Cleansing[id] == 4 || stateLedIndex1_Cleansing[id] == 5){
     //Writing BLUE for the strip's first half from middleLED
     if((ledIndex1_Cleansing[id] > 0) && state3First_Cleansing){
       leds[id][ledIndex1_Cleansing[id]] = paleBlue_Cleansing;
       ledIndex1_Cleansing[id]--;
     }else{
       leds[id][ledIndex1_Cleansing[id]] = paleBlue_Cleansing;
       ledIndex1_Cleansing[id] = (NUM_LEDS*0.5)-1;
       stateLedIndex1_Cleansing[id]++;
       state3First_Cleansing = false;
     }
     //Writing BLUE for the strip's second half from lastLED
     if((ledIndex2_Cleansing[id] > NUM_LEDS*0.5) && state3Second_Cleansing){
       leds[id][ledIndex2_Cleansing[id]] = paleBlue_Cleansing;
       ledIndex2_Cleansing[id]--;
     }else{
       leds[id][ledIndex2_Cleansing[id]] = paleBlue_Cleansing;
       ledIndex2_Cleansing[id] = NUM_LEDS-1;
       stateLedIndex1_Cleansing[id]++;
       state3Second_Cleansing = false;
     }
    }else if(stateLedIndex1_Cleansing[id] == 6 || stateLedIndex1_Cleansing[id] == 7){
     //Writing YELLOW for the strip's second half from middleLED
     if((ledIndex1_Cleansing[id] > 0) && state4First_Cleansing){
       leds[id][ledIndex1_Cleansing[id]] = yellow_On;
       ledIndex1_Cleansing[id]--;
     }else{
       leds[id][ledIndex1_Cleansing[id]] = yellow_On;
       ledIndex1_Cleansing[id] = 0;
       stateLedIndex1_Cleansing[id]++;
       state4First_Cleansing = false;
     }
     //Writing YELLOW for the strip's second half from lastLED
     if((ledIndex2_Cleansing[id] > NUM_LEDS*0.5) && state4Second_Cleansing){
       leds[id][ledIndex2_Cleansing[id]] = yellow_On;
       ledIndex2_Cleansing[id]--;
     }else{
       leds[id][ledIndex2_Cleansing[id]] = yellow_On;
       ledIndex2_Cleansing[id] = NUM_LEDS*0.5;
       stateLedIndex1_Cleansing[id]++;
       state4Second_Cleansing = false;
     }
    }
    //Reset
    if(stateLedIndex1_Cleansing[id] >= 8){
     stateLedIndex1_Cleansing[id] = 0;
     state1First_Cleansing = true;
     state2First_Cleansing = true;
     state3First_Cleansing = true;
     state4First_Cleansing = true;
     state1Second_Cleansing = true;
     state2Second_Cleansing = true;
     state3Second_Cleansing = true;
     state4Second_Cleansing = true;
    }

}

void ANIM_TURQUOISE(int id){
  //17
  
  for(int i=0; i<NUM_LEDS; i++){
    leds[id][i] = paleBlue_Cleansing;
  }
  /*
  if(ledIndex_ANIM_TURQUOISE[id] < NUM_LEDS){
    leds[id][ledIndex_ANIM_TURQUOISE[id]].setHSV(130, 255, 255);
    ledIndex_ANIM_TURQUOISE[id]++;
  }else{
    ledIndex_ANIM_TURQUOISE[id] = 0;
  }
  */
}

void ANIM_PURPLE(int id){
  //18
  //reset to purple

  for(int i=0; i<NUM_LEDS; i++){
    leds[id][i] = purple_Corrupt;
  }
}

void ANIM_YELLOW(int id){
  //19
  
  for(int i=0; i<NUM_LEDS; i++){
    leds[id][i] = yellow_On;
  }
  
}

void ANIM_ORANGE(int id){
  //20
  
  for(int i=0; i<NUM_LEDS; i++){
    leds[id][i] = orange_ANIM;
  }

}

void ANIM_PURPLE_WIPE(int id){
  //21
  
  for(int i=0; i<NUM_LEDS; i++){
    if(switch_PURPLE_WIPE[id]){
      if(i <= NUM_LEDS * 0.5){
        leds[id][i] = purple_Corrupt;
      }else{
        leds[id][i] = purpleLit_PURPLE_WIPE;
      }
    }else{
      if(i <= NUM_LEDS * 0.5){
        leds[id][i] = purpleLit_PURPLE_WIPE;
      }else{
        leds[id][i] = purple_Corrupt;
      }
      switch_PURPLE_WIPE[id] = false;
    }
  }
}

void ANIM_BLACK(int id){
  //22
  
  for(int i=0; i<NUM_LEDS; i++){
    leds[id][i] = empty_off;
  }
  
}

void ANIM_TURQUOISE_FADE(int id){
  //23 : fade PURPLE to CYAN .3 sec. 

  //Brightness Manager
  if(hue_TURQUOISE_FADE[id] >= 120){
    hue_TURQUOISE_FADE[id]-=delayHue_TURQUOISE_FADE;
  }

  for(int i = 0; i < NUM_LEDS; i++){
    leds[id][i].setHSV(hue_TURQUOISE_FADE[id], 255, 255);
  }
  
}

void ANIM_SNAKE_TURQUOISE(int id){
  //24 : .6 sec. CYAN to BLACK
  
  //Brightness Manager
  if(val_SNAKE_TURQUOISE[id] > 0){
    val_SNAKE_TURQUOISE[id]-=delayBrightness_SNAKE_TURQUOISE;
  }
  
  for(int i = 0; i < NUM_LEDS; i++){
    leds[id][i].setHSV(120, 200, val_SNAKE_TURQUOISE[id]);
  }

}

void ANIM_SNAKE_YELLOW(int id){
  //25 : 1 sec. YELLOW to PURPLE

  //Brightness Manager
  if(hue_SNAKE_YELLOW[id] >= 0 && state_SNAKE_YELLOW[id] == true){
    hue_SNAKE_YELLOW[id]-=delayHue_SNAKE_YELLOW;
  }else if(state_SNAKE_YELLOW[id] == true){
    hue_SNAKE_YELLOW[id] = 255;
    state_SNAKE_YELLOW[id] = false;
  }
  
  if(hue_SNAKE_YELLOW[id] >= 185 && state_SNAKE_YELLOW[id] == false){
    hue_SNAKE_YELLOW[id]-=delayHue_SNAKE_YELLOW;
  }else{
    hue_SNAKE_YELLOW[id] = 185;
  }

  for(int i = 0; i < NUM_LEDS; i++){
    leds[id][i].setHSV(hue_SNAKE_YELLOW[id], 255, 255);
  }

}

void ANIM_GREEN_TURQUOISE(int id){
  //26
  
  for(int i=0; i<NUM_LEDS; i++){
    leds[id][i] = greenTurquoise_ANIM;
  }

}
