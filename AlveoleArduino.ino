// hardware libraries to access use the shield

#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"
//#include <HX711.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include "HX711-multi-teensy.h"

//LED related variables
#define LED_PIN    22
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    84
#define NUM_STRIPS 7
#define BRIGHTNESS  255

//Pressure Plate related variable
#define CLK 0
#define CLK2 1
//--first group of clock time hook to CLK1 
#define DOUT1 2
#define DOUT2 3
#define DOUT3 4
#define DOUT4 5
//--second group of clock time hook to CLK2
#define DOUT5 6
#define DOUT6 7
#define DOUT7 9
//--time out of the tare function ?is it use?
#define TARE_TIMEOUT_SECONDS 4 

//--declare sensor pin and index for unity comprehension of received data
byte DOUTS1[4] = {DOUT1,DOUT2,DOUT3,DOUT4};
byte DOUTS2[3] = {DOUT5,DOUT6,DOUT7};

//byte DOUTS1[0] = {};
//byte DOUTS2[0] = {};
//--array of all the index that are going to be send, needed for unity to understand if we dont have all the tiles
int indexs[7] = {0,1,2,3,4,5,6};

//--pre calculate the different buffer needed
#define CHANNEL1 sizeof(DOUTS1) /sizeof(byte)
#define CHANNEL2 sizeof(DOUTS2) /sizeof(byte)
#define CHANNEL_COUNT CHANNEL1 + CHANNEL2

//--serial communication related variable
byte dataArray1[3] = {NULL, NULL, NULL};//the bigest number we will receive is in the 3 number;
byte dataArray2[3] = {NULL, NULL, NULL};//the bigest number we will receive is in the 3 number;
bool record = 0;
int dataBufferIndex = 0;
int lengthOfNbr1 = 0;
int lengthOfNbr2 = 0;

long int results[CHANNEL1];
long int results2[CHANNEL2];

//--define the channels
HX711MULTITEENSY scales(CHANNEL1, DOUTS1, CLK);
HX711MULTITEENSY scales2(CHANNEL2, DOUTS2, CLK2);

//datas array use for calculating the outcome of the receive data
int threshold = 7;
bool prevStatus[CHANNEL_COUNT];
bool hasChanged[CHANNEL_COUNT];
bool tileStatus[CHANNEL_COUNT];
uint32_t prevValues[CHANNEL_COUNT];
int dataId = 419;
int dataState = 419;
int receiveState[7];
int prevReceiveState[7];

//variable related to the behavior of the different state receive

int colorArray[5][3] = { {25,25,25}, {75,75,0}, {50,0,80}, {155,25,25}, {0,85,112} };/*
index 1: off state
index 2: on state
index 3: corrupted state
index 4: ultra corrupted state 
index 5: cleanse state
*/
CRGB leds[NUM_LEDS];
int currentId = 0;

unsigned int intervalRead = 25 ;

unsigned int prevRead = 0;

uint32_t maxReadableValue = 70000;

void setup() {
  Serial.begin(115200);
  //setup ethernet part
  delay(3000); // sanity delay
  //Serial.println("serial begin");
  
  
  for(int i = 0; i < CHANNEL_COUNT; i++){
    tileStatus[i] = 0; //Initialise status to 0
    prevStatus[i] = 0; //Initialise previous status to 0
    hasChanged[i] = 0; //Initialise the hasChange states
    prevValues[i] = 0;
  }

  for(int i = 0; i < 7; i++){
    receiveState[i] = 0; 
    prevReceiveState[i] = 0;   
  }

  // Settings up the led strips
  FastLED.addLeds<CHIPSET, 13, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 14, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 15, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 16, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 17, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 18, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 19, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  //tare();
  //testPattern();

}

//int tick = 0;
void loop() {
  // important! non-blocking listen routine
  if(receiveState[dataId] < 0 || receiveState[dataId] > 35){
    receiveState[dataId] = 0;
  }
  readTheData();
  if(mustReadPressure){
    //Serial.println("mustReadPressure");
    readPressurePlate(); // test without
  }
  for(int i = 0; i < NUM_STRIPS; i++){
    if(receiveState[i] != prevReceiveState[i]){
      int stripState = receiveState[i];
      Serial.println("update led: " + String(receiveState[i]));
      updateLed(i,stripState);
    }
  }
}


bool mustReadPressure (){
  unsigned int currentMillis = millis();

  if(currentMillis - prevRead > intervalRead){
    prevRead = currentMillis;
    return true;
  }else{
    return false;  
  }
  
}
void readTheData() {  // *note the & before msg
  decipherPacket();
  //Serial.println(dataId);
  //Serial.println(dataState);
  int theIndex = dataId;
  //---- a verifier -----
  for(int i = 0; i < CHANNEL_COUNT; i++){
    int indexReceive = theIndex;
    int indexToSend = indexs[i];
    if(indexReceive == indexToSend){
      currentId = indexs[i]; 
    } 
  }
  int hexStatus = receiveState[dataId];
  //Serial.println("receiveState: " + String(receiveState[dataId]));
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
      //Serial.println("finish id go to state");
      //Serial.println("reading state");
      int bufferId = String((char*)dataArray1).toInt();
      memcpy (&dataId, &bufferId, sizeof(bufferId));
      //Serial.println("id receive from python: " + String(dataId));
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
      /*int stripState = receiveState[dataId];
      int r = colorArray[stripState][0];
      int g = colorArray[stripState][1];
      int b = colorArray[stripState][2];
      fill_solid(leds, NUM_LEDS,CRGB(r,g,b));
      FastLED[dataId].showLeds(BRIGHTNESS);
      prevReceiveState[dataId] = receiveState[dataId];*/
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

void updateLed(int strip, int state){
  //strip = id of the strip that need update 
  //state = the state that we need to change the led strip to
  int r = colorArray[state][0];
  int g = colorArray[state][1];
  int b = colorArray[state][2];
  fill_solid(leds, NUM_LEDS,CRGB(r,g,b));
  FastLED[strip].showLeds(BRIGHTNESS);  
  prevReceiveState[strip] = receiveState[strip];
}

void sendHexStatus(int ID, int state){
  int hexId = ID;
  int hexState = indexs[state];
  Serial.println((String)hexId + "," + hexState + "e/hexData");  
}

void readPressurePlate(){
  //combine the two analogue pin result state into one array
  //Serial.println("read pressure plate:");
  scales.read(results);
  //Serial.println("size of results:");
  int chn2Index = 0;
  uint32_t value;
  for (int i=0; i<CHANNEL_COUNT; i++) {
      int theIndex = indexs[i];
      if(i < CHANNEL1){
        //Serial.println("channel1 iteration : " + i);
        value = abs(results[i]) * 0.0001; // can we do something more clear and unforgetable then this division
      }else{
        
        scales2.read(results2);
        //Serial.println("channel2 iteration : " + chn2Index);
        value = abs(results2[chn2Index]) * 0.0001; // can we do something more clear and unforgetable then this division  
        chn2Index = chn2Index + 1;
      }
      if(value < maxReadableValue && value >= 0){
        int32_t delta =value - prevValues[i];
        tileStatus[i] = (delta > threshold) ? 1 :0;
        
        if(delta < maxReadableValue && abs(delta) >= threshold && tileStatus[i] != prevStatus[i] ){
          //Serial.println("data send from duino: " + String(indexs[theIndex]));
          stateCtrl(indexs[theIndex], prevReceiveState[i]);
          //call the function that will send data with id and class as parameter
          sendHexStatus(indexs[theIndex],tileStatus[i]);
        }
        
        prevValues[i] = value;
        prevStatus[i] = tileStatus[i];
        
      }
  }   
}

void stateCtrl(int id, int prevState){
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
  updateLed(id,1);
}

void corrupted_state(int id){
 //do nothing for now
}

void ultra_corrupted_state(int id){
 //do nothing for now
}

void cleanse_state(int id){
  //do nothing for now
}
