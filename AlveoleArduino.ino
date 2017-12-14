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
#define CHIPSET     WS2811
#define NUM_LEDS    84
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

int resistance = 2; //Number of time the threshold is multiply. Basic threshold are 10% of their initial value and each resistance multiply that number
int currentSensor = 0;

int prevTileStatus[7];
int tileStatus[7];
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

CRGB leds[NUM_LEDS];
int currentId = 0;

//variable related to the sensor data analyse

uint32_t maxReadableValue = 2000000; // value to be change need testing
int sensorRawValue[7];
int pressureState[7];

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
  FastLED.addLeds<CHIPSET, 2, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 14, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 7, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 8, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 20, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 6, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 21, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  
  //tare();
  //testPattern();

}

//int tick = 0;
void loop() {
  // important! non-blocking listen routine
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
      updateLed(i,stripState);
    }
  }
  
  delay(2); 
}

void calibrateHex(){
for(int i = 0; i< NUM_STRIPS; i++){
    long int sensorValue;
    switch(i){
      case 0:
        sensorValue = scale0.read();
        calibrateSensor(sensorValue,i);
        break;
      case 1:
        sensorValue = scale1.read();
        calibrateSensor(sensorValue,i);
        break;
      case 2:
        sensorValue = scale2.read();
        calibrateSensor(sensorValue,i);
        break;
      case 3:
        sensorValue = scale3.read();
        calibrateSensor(sensorValue,i);
        break;
      case 4:
        sensorValue = scale4.read();
        calibrateSensor(sensorValue,i);
        break;
      case 5:
        sensorValue = scale5.read();
        calibrateSensor(sensorValue,i);
        break;
      case 6:
        sensorValue = scale6.read();
        calibrateSensor(sensorValue,i);
        break;
    }
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
    sensorThreshold[sensorId] = theThreshold;
  }else{
    
    theThreshold = (sensorValue * 0.1) * resistance ;
    theThreshold = abs(theThreshold);
    sensorOrientation[sensorId] = false;
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
  if (valid){
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
      
      for(int i = 0; i<dataArray2[3]; i++){
        dataArray2[i] = NULL;
      }
      
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
        int stripState = receiveState[i];
        Serial.println("update led: " + String(receiveState[i]));
        updateLed(i, stripState);
      }
      
      //prevReceiveState[i] = receiveState[i];
    }
    //Serial.println("dataId : " + String(dataId));
    //Serial.println("dataState : " + String(dataState));
  }
}

void updateLed(int stripId, int state){
  //strip = id of the strip that need update 
  //state = the state that we need to change the led strip to
  int r = colorArray[state][0];
  int g = colorArray[state][1];
  int b = colorArray[state][2];
  fill_solid(leds, NUM_LEDS,CRGB(r,g,b));
  FastLED[stripId].showLeds(BRIGHTNESS);  
  prevReceiveState[stripId] = receiveState[stripId];
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
    difference = sensorStartValue[index] - currentValue;
    if(difference > sensorThreshold[index]){
      tilePressure[index] = true;
    }
    else{ 
      tilePressure[index] = false; 
    }
  }else{
    long int startValue = abs(sensorStartValue[index]);
    difference =  currentValue + startValue ;
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
      stateCtrl(index,tileStatus[index]);
      sendHexStatus(indexs[index],tileStatus[index]);
    }else{
      Serial.print("sensor #");
      Serial.print(index);
      Serial.print("is no longer trigger with difference of : ");
      Serial.print(difference);
      Serial.print(" > then the threshold of: ");
      Serial.println(sensorThreshold[index]);
      tileStatus[index] = 0;
      stateCtrl(index,tileStatus[index]);
      sendHexStatus(indexs[index],tileStatus[index]);
    }
  }
  prevTileStatus[index] = tileStatus[index];
  prevTilePressure[index] = tilePressure[index];
}

void stateCtrl(int id, int prevState){
  Serial.print("stateCtrl has ben call on id: ");
  Serial.println(id);
  switch( prevState ) {
  case 1:
    //off state
    off_state(id);
  case 2:
    //on state
    on_state(id);
  case 3:
    //corrupted state
    corrupted_state(id);
  case 4:
    //ultra corrupted state
    ultra_corrupted_state(id);
  case 5:
    //cleanse state
    cleanse_state(id);
  }
}

void off_state(int id){
  updateLed(id,1);
}

void on_state(int id){
  //do nothing for now
}

void corrupted_state(int id){
  updateLed(id,1);
}

void ultra_corrupted_state(int id){
 //do nothing for now 
}

void cleanse_state(int id){
  //TODO update central tile to white and then wait for unity for the status change of surrounding tiles
}
