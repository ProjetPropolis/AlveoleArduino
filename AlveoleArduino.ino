// hardware libraries to access use the shield

#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"
//#include <HX711.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include "HX711-multi-teensy.h"

#include <Chrono.h>
#include <LightChrono.h>

//LED related variables
#define LED_PIN             22
#define COLOR_ORDER         GRB
#define CHIPSET             WS2811
#define NUM_LEDS_PER_STRIP  30
#define NUM_STRIPS          10
#define NUM_AVAILABLE       4
#define BRIGHTNESS          255

CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];


/* Communication Variables*/
int dataId = 419;
int currentId = 0;

int receiveState[NUM_STRIPS];
int prevReceiveState[NUM_STRIPS];

//--serial communication related variable
byte dataArray1[3] = {NULL, NULL, NULL};//the bigest number we will receive is in the 3 number;
byte dataArray2[3] = {NULL, NULL, NULL};//the bigest number we will receive is in the 3 number;
bool record = 0;
int dataBufferIndex = 0;
int lengthOfNbr1 = 0;
int lengthOfNbr2 = 0;


/*==== purple molecules Variables ====*/
Chrono myChrono_Purple;
int receiveInfectId = 0;

/*==== buttons Variables ====*/
Chrono buttonChrono0;
Chrono buttonChrono1;
Chrono buttonChrono2;
Chrono buttonChrono3;
Chrono buttonChrono4;
Chrono buttonChrono5;
Chrono buttonChrono6;
Chrono buttonChrono7;
Chrono buttonChrono8;
Chrono buttonChrono9;
Chrono buttonChrono[] = {buttonChrono0, buttonChrono1, buttonChrono2, buttonChrono3, buttonChrono4, buttonChrono5, buttonChrono6, buttonChrono7, buttonChrono8, buttonChrono9};
int digitalPin0 = 1;
int digitalPin1 = 4;
int digitalPin2 = 10;
int digitalPin3 = 3;
int digitalPin4 = 4;
int digitalPin5 = 5;
int digitalPin6 = 6;
int digitalPin7 = 7;
int digitalPin8 = 8;
int digitalPin9 = 12;
int digitalPin[] = {digitalPin0 ,digitalPin1 ,digitalPin2 ,digitalPin3 ,digitalPin4 ,digitalPin5 ,digitalPin6, digitalPin7, digitalPin8, digitalPin9};
int referenceDigitalPin[] = {0,1,2,9};

bool moleculeStatus[10];
int stateColorMolecule[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
bool boolStateMolecule[] = {false, false, false, false, false, false, false, false, false, false};
int indexState[NUM_STRIPS];
int referenceState[] = {5, 6, 7};

unsigned int intervalRead = 25 ;

unsigned int prevRead = 0;

uint32_t maxReadableValue = 70000;

int colorArray[7][3] = { {0,0,0}, {0, 0, 255}, {200, 50, 0}, {255, 35, 40}, {0,150,255}, {100,0,255}, {50,0,80} };
int stripState = 0;

CRGB blue_Recette(0, 0, 255);
CRGB orange_Recette(200, 50, 0);
CRGB pink_Recette(255, 35, 40);
CRGB palePurple_Recette(100, 0, 255);
CRGB darkPurple_Recette(50, 0, 80);
CRGB cyan_ShieldOff(0, 100, 170);
CRGB cyan_ShieldOn(0, 150, 255);
CRGB red_waveCorrupted(255, 0, 0);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Settings up the led strips
  FastLED.addLeds<CHIPSET, 2, COLOR_ORDER>(leds[0], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 14, COLOR_ORDER>(leds[1], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 7, COLOR_ORDER>(leds[2], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 8, COLOR_ORDER>(leds[9], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 17, COLOR_ORDER>(leds[4], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 18, COLOR_ORDER>(leds[5], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 19, COLOR_ORDER>(leds[6], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 20, COLOR_ORDER>(leds[7], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 21, COLOR_ORDER>(leds[8], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 22, COLOR_ORDER>(leds[9], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );

  // Setting up the buttons
  pinMode(digitalPin[0], INPUT_PULLUP);
  pinMode(digitalPin[1], INPUT_PULLUP);
  pinMode(digitalPin[2], INPUT_PULLUP);
  pinMode(digitalPin[9], INPUT_PULLUP);
  /*
  pinMode(digitalPin[4], INPUT_PULLUP);
  pinMode(digitalPin[5], INPUT_PULLUP);
  pinMode(digitalPin[6], INPUT_PULLUP);
  pinMode(digitalPin[7], INPUT_PULLUP);
  pinMode(digitalPin[8], INPUT_PULLUP);
  pinMode(digitalPin[9], INPUT_PULLUP);
*/
  for(int i = 0; i < NUM_STRIPS; i++){
    moleculeStatus[i] = digitalRead(digitalPin[i]);
    stateCtrl(i, 0, 0);
  }
  FastLED.show();

}

void loop() {
  // put your main code here, to run repeatedly:
  readButtonStatus();

  readTheData();
  for(int i = 0; i < NUM_STRIPS; i++){
    if(receiveState[i] != prevReceiveState[i]){
      int stripState = receiveState[i];
      //Serial.println("update led: " + String(receiveState[i]));
      //stateCtrl(i,stripState, prevReceiveState[i]);
    }
    stateCtrl(i,receiveState[i], prevReceiveState[i]);
    //FastLED.show();
  }

  FastLED.show();

/*
  if(mustReadPressure){
    //Serial.println("mustReadPressure");
    readButtonStatus();
    //readPressurePlate(); // test without
  }
*/


  
  /*
  //Testing teensy
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
    for(int z = 0; z < NUM_STRIPS; z++){
    leds[z][i] = CRGB::Red;
    leds[z][i] = CRGB::Red;
    leds[z][i] = CRGB::Red;
    leds[z][i] = CRGB::Red;
    }
  }
  */
  
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

void readTheData() {
  decipherPacket();
  int theIndex = dataId;
  //---- a verifier -----
  for(int i = 0; i < NUM_STRIPS; i++){
    //int indexReceive = theIndex;
    currentId = theIndex;
    /*
    int indexToSend = indexs[i];
    if(indexReceive == indexToSend){
      currentId = indexs[i]; 
    } 
    */
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
      for(int i = 0; i<sizeof(dataArray1); i++){
        dataArray1[i] = NULL;
      }
      dataBufferIndex = 2;
    }
    else if(c == 122){  //122 ascii = z
      //Serial.println("packet end");
      int bufferState = String((char*)dataArray2).toInt();
      //Serial.println("packetEnd value of buffer: " + bufferState);
      Serial.print("receiveState_Before_memcpy:");
      Serial.println(receiveState[dataId]);
      memcpy (&receiveState[dataId], &bufferState, sizeof(bufferState));
      Serial.print("receiveState_After_memcpy:");
      Serial.println(receiveState[dataId]);
      for(int i = 0; i<sizeof(dataArray2); i++){
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
        //Serial.println("update led: " + String(receiveState[i]));
        stateCtrl(i, stripState, prevReceiveState[i]);
        
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
  int hexState = state;
  Serial.println((String)hexId + "," + hexState + "e/hexData");  
}

void readButtonStatus(){
  for(int i = 0; i < NUM_AVAILABLE; i++){

    int index = referenceDigitalPin[i];
    
    moleculeStatus[index] = digitalRead(digitalPin[index]);
    
    if(moleculeStatus[index] == LOW){
      if(boolStateMolecule[index] == false && indexState[index] < 2 && buttonChrono[index].hasPassed(500)){
        Serial.print("Enter if:");
        Serial.println(receiveState[index]);
        buttonChrono[index].restart();
        boolStateMolecule[index] = true;
        indexState[index]++;
        receiveState[index] = referenceState[indexState[index]];
        sendHexStatus(index, receiveState[index]);
        //Serial.println(receiveState[0]);
      }else if(boolStateMolecule[index] == false && buttonChrono[index].hasPassed(500)){
        Serial.print("Enter else if:");
        Serial.println(receiveState[index]);
        buttonChrono[index].restart();
        boolStateMolecule[index] = true;
        indexState[index] = 0;
        receiveState[index] = referenceState[indexState[index]];
        
        sendHexStatus(index, receiveState[index]);
      }
      stateCtrl(index, receiveState[index], prevReceiveState[index]);
    }else if (moleculeStatus[index] == HIGH) {
      boolStateMolecule[index] = false;
    }

  }
  FastLED.show();
}

void stateCtrl(int id, int state, int prevState){
  Serial.print("entering stateCtrl with id: ");
  Serial.print(id);
  Serial.print(" , state of: ");
  Serial.print(state);
  Serial.print(" and prevState of: ");
  Serial.print(prevState);
  if(state < 15 && state >= 0){
    Serial.print("/ enter if");
    switch (state) {
      case 0: off(id);
            break;
      case 2: corrupted(id);
            break;
      case 5: orange(id);
            break;
      case 6: pink(id);
            break;
      case 7: blue(id);
            break;
      case 9: waveCorrupted(id);
            break;
      case 13: shield_On(id);
            Serial.print("/ enter on");
            break;
      case 14: shield_Off(id);
            Serial.print("/ enter of");
            break;
    }
  }
  Serial.println(" --------end of stateCTRL");
}

void off(int id){
     for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      leds[id][i] = CRGB::Black;
     }
}

void blue(int id){
     for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      leds[id][i] = blue_Recette;
     }
}

void orange(int id){
     for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      leds[id][i] = orange_Recette;
     }
}

void pink(int id){
     for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      leds[id][i] = pink_Recette;
     }
}

void corrupted(int id){
     if(!myChrono_Purple.hasPassed(1000)){
       for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
        leds[id][i] = palePurple_Recette;
       }
     }else if(!myChrono_Purple.hasPassed(2000)){
       for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
        leds[id][i] = darkPurple_Recette;
       }
     }else{
       myChrono_Purple.restart();
     }
}

void waveCorrupted(int id){
     for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      leds[id][i] = red_waveCorrupted;
     }
}

void shield_Off(int id){
     for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      leds[id][i] = cyan_ShieldOff;
     }
}

void shield_On(int id){
     for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      leds[id][i] = cyan_ShieldOn;
     }
}
