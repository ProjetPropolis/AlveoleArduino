// hardware libraries to access use the shields

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
#define NUM_LEDS_PER_STRIP  60
#define NUM_LEDS_ATOM       30
#define NUM_STRIPS          10
#define NUM_AVAILABLE       10
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

/*=== buttons Variables ===*/
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
int digitalPin0 = 0;
int digitalPin1 = 3;
int digitalPin2 = 1;
int digitalPin3 = 9;
int digitalPin4 = 11;
int digitalPin5 = 10;
int digitalPin6 = 15;
int digitalPin7 = 17;
int digitalPin8 = 19;
int digitalPin9 = 23;
int digitalPin[] = {digitalPin0 ,digitalPin1 ,digitalPin2 ,digitalPin3 ,digitalPin4 ,digitalPin5 ,digitalPin6, digitalPin7, digitalPin8, digitalPin9};
int referenceDigitalPin[] = {0,1,2,3,4,5,6,7,8,9};

/*=== Recettes Variables ===*/
int ledIndex_Recette[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int stateAnim_Recette[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int delayIndex_Recette = 10;

/*=== blue() Variables ===*/
int hue_Blue[] = {130, 130, 130, 130, 130, 130, 130, 130, 130, 130};
bool state_Blue[] = {true, true, true, true, true, true, true, true, true, true};

Chrono blueChrono0;
Chrono blueChrono1;
Chrono blueChrono2;
Chrono blueChrono3;
Chrono blueChrono4;
Chrono blueChrono5;
Chrono blueChrono6;
Chrono blueChrono7;
Chrono blueChrono8;
Chrono blueChrono9;
Chrono blueChrono[] = {blueChrono0, blueChrono1, blueChrono2, blueChrono3, blueChrono4, blueChrono5, blueChrono6, blueChrono7, blueChrono8, blueChrono9};
bool preBlueAnim_Blue[] = {true, true, true, true, true, true, true, true, true, true};
int ledIndex_Blue[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int sat_Blue[] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200};
int stateAnim_Blue[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int delayAnim_Blue = 250;

/*=== corrupt() Variables ===*/
int ledIndexGlitch1_Corrupt[NUM_STRIPS];
int ledIndexGlitch2_Corrupt[NUM_STRIPS];
int ledIndexGlitch3_Corrupt[NUM_STRIPS];
int ledIndexGlitch4_Corrupt[NUM_STRIPS];
int ledIndexGlitch5_Corrupt[NUM_STRIPS];
int ledIndexGlitch6_Corrupt[NUM_STRIPS];
int ledIndexGlitch7_Corrupt[NUM_STRIPS];
int ledIndexGlitch8_Corrupt[NUM_STRIPS];
int ledIndexGlitch9_Corrupt[NUM_STRIPS];
int ledIndexGlitch10_Corrupt[NUM_STRIPS];
int dashLenght_Corrupt = 3;
int stepRandom_Corrupt = 1;
int maxCorruptLenght = dashLenght_Corrupt+30;

/*=== ultracorrupt() Variables ===*/
int ledIndexGlitch1_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch2_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch3_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch4_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch5_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch6_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch7_Ultracorrupt[NUM_STRIPS];
int ledIndexGlitch8_Ultracorrupt[NUM_STRIPS];
int dashLenght_Ultracorrupt = 4;
int maxUltracorruptLenght = dashLenght_Ultracorrupt+30;

/*=== shield_Off Variables ===*/
int sat_ShieldOff[] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
int a_ShieldOff[] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
int delayBrightness_ShieldOff = 2;

/*=== shield_On Variables ===*/
int ledIndexGlitch1_ShieldOn[NUM_STRIPS];
int ledIndexGlitch2_ShieldOn[NUM_STRIPS];
int ledIndexGlitch3_ShieldOn[NUM_STRIPS];
int ledIndexGlitch4_ShieldOn[NUM_STRIPS];
int ledIndexGlitch5_ShieldOn[NUM_STRIPS];
int ledIndexGlitch6_ShieldOn[NUM_STRIPS];
int ledIndexGlitch7_ShieldOn[NUM_STRIPS];
int ledIndexGlitch8_ShieldOn[NUM_STRIPS];

/*=== ANIM_TURQUOISE_FADE() Variables ===*/
int hue_TURQUOISE_FADE[NUM_STRIPS] = {185, 185, 185, 185, 185, 185, 185, 185, 185, 185};
int delayHue_TURQUOISE_FADE = 1;

/*=== ANIM_SNAKE_TURQUOISE() Variables ===*/
float val_SNAKE_TURQUOISE[NUM_STRIPS] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
float delayBrightness_SNAKE_TURQUOISE = 1.5;

/*=== ANIM_SNAKE_YELLOW() Variables ===*/
int hue_SNAKE_YELLOW[NUM_STRIPS] = {64, 64, 64, 64, 64, 64, 64, 64, 64, 64};
bool state_SNAKE_YELLOW[] = {true, true, true, true, true, true, true, true, true, true};
bool stateAnimSNAKE_YELLOW[] = {true, true, true, true, true, true, true, true, true, true};
int delayHue_SNAKE_YELLOW = 1;

bool moleculeStatus[10];
int delayMoleculeStatus = 100;
int stateColorMolecule[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool isPressedMolecule[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool boolStateMolecule[] = {false, false, false, false, false, false, false, false, false, false};
bool prevStateMolecule[] = {false, false, false, false, false, false, false, false, false, false};
int indexState[NUM_STRIPS];
int referenceState[] = {5, 6, 7};
int indexShield = NUM_STRIPS-1;
bool needReset[NUM_STRIPS] = {0,0,0,0,0,0,0,0,0,0};
bool unlocked[] = {true, true, true, true, true, true, true, true, true, true};

unsigned int intervalRead = 25 ;

unsigned int prevRead = 0;

uint32_t maxReadableValue = 70000;

int colorArray[7][3] = { {0,0,0}, {0, 0, 255}, {200, 50, 0}, {255, 35, 40}, {0,150,255}, {100,0,255}, {50,0,80} };
int stripState = 0;

/*=== colors Variables ===*/
CRGB empty_off(0, 0, 0);
CHSV blue_Recette(135, 200, 190);
CRGB orange_Recette(200, 50, 0);
CRGB pink_Recette(255, 35, 40);
CHSV purple_Corrupt(210, 255, 255);
CHSV palePurple_Corrupt(210, 50, 255);
CRGB palePurple_Recette(100, 0, 255);
CRGB darkPurple_Recette(50, 0, 80);
CRGB cyan_ShieldOff(0, 24, 40);
//CRGB cyan_ShieldOn(0, 50, 170);
CHSV cyan_ShieldOn(125, 255, 255);
CRGB red_waveCorrupted(255, 0, 0);
CHSV blue_Cleanser(140, 255, 255);
CHSV paleBlue_Cleansing(140, 200, 255);
CRGB yellow_On(200, 110, 15);
CRGB orange_ANIM(200, 80, 0);
CRGB green_ANIM(0, 220, 100);
CRGB greenTurquoise_ANIM(0, 200, 125);
CHSV purpleLit_PURPLE_WIPE(180, 100, 255);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Settings up the led strips
  FastLED.addLeds<CHIPSET, 2, COLOR_ORDER>(leds[0], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 14, COLOR_ORDER>(leds[1], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 4, COLOR_ORDER>(leds[2], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 7, COLOR_ORDER>(leds[3], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 8, COLOR_ORDER>(leds[4], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 12, COLOR_ORDER>(leds[5], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 20, COLOR_ORDER>(leds[6], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 6, COLOR_ORDER>(leds[7], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 21, COLOR_ORDER>(leds[8], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 5, COLOR_ORDER>(leds[9], NUM_LEDS_PER_STRIP).setCorrection( TypicalLEDStrip );

  // Setting up the buttons
  pinMode(digitalPin[0], INPUT_PULLUP);
  pinMode(digitalPin[1], INPUT_PULLUP);
  pinMode(digitalPin[2], INPUT_PULLUP);
  pinMode(digitalPin[3], INPUT_PULLUP);
  pinMode(digitalPin[4], INPUT_PULLUP);
  pinMode(digitalPin[5], INPUT_PULLUP);
  pinMode(digitalPin[6], INPUT_PULLUP);
  pinMode(digitalPin[7], INPUT_PULLUP);
  pinMode(digitalPin[8], INPUT_PULLUP);
  pinMode(digitalPin[9], INPUT_PULLUP);
  
  for(int i = 0; i < NUM_STRIPS; i++){
    moleculeStatus[i] = digitalRead(digitalPin[i]);
    receiveState[i] = 0; 
    prevReceiveState[i] = 0;  
    stateCtrl(i, receiveState[i], prevReceiveState[i]);
  }

  FastLED.show();

}

void loop() {
  // put your main code here, to run repeatedly:
  readButtonStatus();  

  readTheData();
  for(int i = 0; i < NUM_STRIPS; i++){
    int stripState = receiveState[i];
    //Serial.println("update led: " + String(receiveState[i]));
    if(i != indexShield && stripState != 15 && stripState != 16){
      stateCtrl(i, stripState, prevReceiveState[i]);
    }else if(i == indexShield && (stripState == 13 || stripState == 14 || stripState == 17 || stripState == 18 || stripState == 19 || stripState == 20 || stripState == 21 || stripState == 22 || stripState == 23 || stripState == 24 || stripState == 25 || stripState == 26)){
      stateCtrl(i, stripState, prevReceiveState[i]);
    }
    
    if(receiveState[i] != prevReceiveState[i]){
      prevReceiveState[i] = receiveState[i];
    }
  }

  FastLED.show();
  
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
    for(int i = 0; i < NUM_AVAILABLE; i++){
      //Writes unity's states in indexState[] and manages readButtonStatus' locking
      if(receiveState[i] == 5){
        indexState[i] = 0;  
      }else if(receiveState[i] == 6){
        indexState[i] = 1;
      }else if(receiveState[i] == 7){
        indexState[i] = 2;
      }

      if(receiveState[i] != prevReceiveState[i]){
        if(receiveState[i] == 15){
        //DETECTION_OFF
          unlocked[i] = false;
        }else if(receiveState[i] == 16){
          //DETECTION_ON
          unlocked[i] = true;
        }
        int stripState = receiveState[i];
        //Serial.println("update led: " + String(receiveState[i]));
        if(i != indexShield && stripState != 15 && stripState != 16){
          stateCtrl(referenceDigitalPin[i], stripState, prevReceiveState[i]);
        }else if(i == indexShield && (stripState == 13 || stripState == 14 || stripState == 17 || stripState == 18 || stripState == 19 || stripState == 20 || stripState == 21 || stripState == 22 || stripState == 23 || stripState == 24 || stripState == 25 || stripState == 26)){
          // if(stripState != 2) add to the else if needed(were testing the condition)
          stateCtrl(referenceDigitalPin[i], stripState, prevReceiveState[i]);
        }
        prevReceiveState[i] = receiveState[i];
      }
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
    if(unlocked[i]){

      int index = referenceDigitalPin[i];
      
      moleculeStatus[index] = digitalRead(digitalPin[index]);
      
      if(moleculeStatus[index] == LOW){
        if(boolStateMolecule[index] == false && buttonChrono[index].hasPassed(delayMoleculeStatus) && indexState[index] < 2 && (receiveState[index] == 5 || receiveState[index] == 6 || receiveState[index] == 7 || receiveState[index] == 13 || receiveState[index] == 14 || receiveState[index] == 15 || receiveState[index] == 16)){
          if(index == indexShield && prevStateMolecule[index] != moleculeStatus[index]){
            boolStateMolecule[index] = true;
            receiveState[index] = 13;
            stateCtrl(index, receiveState[index], prevReceiveState[index]);
            sendHexStatus(index, 1);
          }else if(index != indexShield){
            //Serial.print("Enter if:");
            //Serial.println(receiveState[index]);
            buttonChrono[index].restart();
            boolStateMolecule[index] = true;
            indexState[index]++;
            receiveState[index] = referenceState[indexState[index]];
            isPressedMolecule[referenceState[i]] = 1;
            stateCtrl(index, receiveState[index], prevReceiveState[index]);
            sendHexStatus(index, 1);
          }
        }else if(boolStateMolecule[index] == false && buttonChrono[index].hasPassed(delayMoleculeStatus && index != indexShield)){
          //Serial.print("Enter else if:");
          //Serial.println(receiveState[index]);
          buttonChrono[index].restart();
          boolStateMolecule[index] = true;
          indexState[index] = 0;
          receiveState[index] = referenceState[indexState[index]];
          isPressedMolecule[referenceState[i]] = 1;
          stateCtrl(index, receiveState[index], prevReceiveState[index]);
          sendHexStatus(index, 1);
        }
      }else if (moleculeStatus[index] == HIGH) {
        if(index == indexShield && prevStateMolecule[index] != moleculeStatus[index]){
          boolStateMolecule[index] = false;
          receiveState[index] = 14;
          stateCtrl(index, receiveState[index], prevReceiveState[index]);
          sendHexStatus(index, 0);
        }else if(index != indexShield){
          boolStateMolecule[index] = false;
          if(isPressedMolecule[referenceState[i]] == 1){
            sendHexStatus(index, 0);
            isPressedMolecule[referenceState[i]] = 0;
          }
        }
      }
      //prevReceiveState[index] = receiveState[index];
      prevStateMolecule[index] = moleculeStatus[index];
    }
  }
  FastLED.show();
}

void stateCtrl(int id, int state, int prevState){
  
  //Resets climax's anim when it's the alveole's prevState
  
  if(prevState == 7 && state != 7){
    ledIndex_Blue[id] = 0;
    stateAnim_Blue[id] = 0;
    preBlueAnim_Blue[id] = true;
  }else if(prevState == 23 && state!= 23){
    hue_TURQUOISE_FADE[id] = 185;
  }else if(prevState == 24 && state != 24){
    val_SNAKE_TURQUOISE[id] = 255;
  }else if(prevState == 25 && state!= 25){
    hue_SNAKE_YELLOW[id] = 64;
    stateAnimSNAKE_YELLOW[id] = true;
  }
  
  if(state < 30 && state >= 0){
    switch( state ) {
      case 0: 
        off(id);
        break;
      case 1: 
        needReset[id] = 1;
        on(id);
        break;
      case 2: 
        if(needReset[id]){
          needReset[id] = 0;
          off(id);
        }
        corrupt(id);
        break;
      case 4: 
        shield_On(id);
        //cleanser
        break;
      case 5: 
        needReset[id] = 1;
        orange(id);
        break;
      case 6:
        needReset[id] = 1; 
        pink(id);
        break;
      case 7: 
        needReset[id] = 1;
        if(preBlueAnim_Blue[id]){
          preBlue(id);
        }else{
          blue(id);  
        }
        break;
      case 8: 
        shield_On(id);
        //cleansing
        break;
      case 9: 
        needReset[id] = 1;
        waveCorrupted(id);
        break;
      case 13: 
        shield_On(id);
        break;
      case 14:
        shield_Off(id);
        break;
      case 15:
        corrupt(id);
        break;
      case 16:
        corrupt(id);
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
        off(id);
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
}

void off(int id){
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i].setRGB(0,0,0);
  }
}

void on(int id){
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = CRGB::White;
  }
}

void preBlue(int id){
  
  preBlueAnim_Blue[id] = false;
  sat_Blue[id] = 200;
  
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = blue_Recette;
  }
  
}

void blue(int id){
  //CHSV blue_Recette(135, 200, 190);
  
  if(blueChrono[id].hasPassed(delayAnim_Blue)){
    if(sat_Blue[id] > 0 && stateAnim_Blue[id] == 0){
      sat_Blue[id]--;
    }else if(stateAnim_Blue[id] == 0){
      sat_Blue[id] = 0;
      stateAnim_Blue[id] = 1;
    }else if(sat_Blue[id] < 200 && stateAnim_Blue[id] == 1){
      sat_Blue[id]++;
    }else if(stateAnim_Blue[id] == 1){
      sat_Blue[id] = 200;
      stateAnim_Blue[id] = 0;
    }
    for(int i = 0; i < NUM_LEDS_ATOM; i++){
      leds[id][ledIndex_Blue[id]].setHSV(135, sat_Blue[id], 190);
    }
    blueChrono[id].restart();
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

void corrupt(int id){

    //Writing BLACK for the 5 Dashes
    for(int x = 0; x < dashLenght_Corrupt; x++){
      if(x != 2){
        leds[id][ledIndexGlitch1_Corrupt[id] + x].setHSV(210, 255, 0);
        leds[id][ledIndexGlitch2_Corrupt[id] + x].setHSV(210, 255, 0);
      }
      leds[id][ledIndexGlitch3_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch4_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch5_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch6_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch7_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch8_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch9_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[id][ledIndexGlitch10_Corrupt[id] + x].setHSV(210, 255, 0);
    }
    leds[id][ledIndexGlitch1_Corrupt[id]+2].setHSV(210, 255, 0);
    leds[id][ledIndexGlitch2_Corrupt[id]+2].setHSV(210, 255, 0);

    //Changing 5 Dashes starting index
    ledIndexGlitch1_Corrupt[id] = constrain(random8(ledIndexGlitch1_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch1_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch2_Corrupt[id] = constrain(random8(ledIndexGlitch2_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch2_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch3_Corrupt[id] = constrain(random8(ledIndexGlitch3_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch3_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch4_Corrupt[id] = constrain(random8(ledIndexGlitch4_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch4_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch5_Corrupt[id] = constrain(random8(ledIndexGlitch5_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch5_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch6_Corrupt[id] = constrain(random8(ledIndexGlitch6_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch6_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch7_Corrupt[id] = constrain(random8(ledIndexGlitch7_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch7_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch8_Corrupt[id] = constrain(random8(ledIndexGlitch8_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch8_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch9_Corrupt[id] = constrain(random8(ledIndexGlitch9_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch9_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    ledIndexGlitch10_Corrupt[id] = constrain(random8(ledIndexGlitch10_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch10_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS_PER_STRIP-(maxCorruptLenght));
    
    //Writing PURPLE for the 5 Dashes
    for(int x = 0; x < dashLenght_Corrupt; x++){
      if(x != 2){
        leds[id][ledIndexGlitch1_Corrupt[id] + x] = purple_Corrupt;
        leds[id][ledIndexGlitch2_Corrupt[id] + x] = purple_Corrupt;
      }
      leds[id][ledIndexGlitch3_Corrupt[id] + x] = purple_Corrupt;
      leds[id][ledIndexGlitch4_Corrupt[id] + x] = purple_Corrupt;
      leds[id][ledIndexGlitch5_Corrupt[id] + x] = purple_Corrupt;
      leds[id][ledIndexGlitch6_Corrupt[id] + x] = purple_Corrupt;
      leds[id][ledIndexGlitch7_Corrupt[id] + x] = purple_Corrupt;
      leds[id][ledIndexGlitch8_Corrupt[id] + x] = purple_Corrupt;
      leds[id][ledIndexGlitch9_Corrupt[id] + x] = purple_Corrupt;
      leds[id][ledIndexGlitch10_Corrupt[id] + x] = purple_Corrupt;
    }
    leds[id][ledIndexGlitch1_Corrupt[id]+2] = palePurple_Corrupt;
    leds[id][ledIndexGlitch2_Corrupt[id]+2] = palePurple_Corrupt;
    
}

void waveCorrupted(int id){

    //Writing RED for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      leds[id][ledIndexGlitch1_Ultracorrupt[id] + x].setRGB(0, 0, 0);
      leds[id][ledIndexGlitch2_Ultracorrupt[id] + x].setRGB(0, 0, 0);
      leds[id][ledIndexGlitch3_Ultracorrupt[id] + x].setRGB(0, 0, 0);
      leds[id][ledIndexGlitch4_Ultracorrupt[id] + x].setRGB(0, 0, 0);
      leds[id][ledIndexGlitch5_Ultracorrupt[id] + x].setRGB(0, 0, 0);
      leds[id][ledIndexGlitch6_Ultracorrupt[id] + x].setRGB(0, 0, 0);
      leds[id][ledIndexGlitch7_Ultracorrupt[id] + x].setRGB(0, 0, 0);
      leds[id][ledIndexGlitch8_Ultracorrupt[id] + x].setRGB(0, 0, 0);
    }

    //Changing 5 Dashes starting index
    ledIndexGlitch1_Ultracorrupt[id] = random8(NUM_LEDS_PER_STRIP-(maxUltracorruptLenght));
    ledIndexGlitch2_Ultracorrupt[id] = random8(NUM_LEDS_PER_STRIP-(maxUltracorruptLenght));
    ledIndexGlitch3_Ultracorrupt[id] = random8(NUM_LEDS_PER_STRIP-(maxUltracorruptLenght));
    ledIndexGlitch4_Ultracorrupt[id] = random8(NUM_LEDS_PER_STRIP-(maxUltracorruptLenght));
    ledIndexGlitch5_Ultracorrupt[id] = random8(NUM_LEDS_PER_STRIP-(maxUltracorruptLenght));
    ledIndexGlitch6_Ultracorrupt[id] = random8(NUM_LEDS_PER_STRIP-(maxUltracorruptLenght));
    ledIndexGlitch7_Ultracorrupt[id] = random8(NUM_LEDS_PER_STRIP-(maxUltracorruptLenght));
    ledIndexGlitch8_Ultracorrupt[id] = random8(NUM_LEDS_PER_STRIP-(maxUltracorruptLenght));

    //Writing RED for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      leds[id][ledIndexGlitch1_Ultracorrupt[id] + x] = red_waveCorrupted;
      leds[id][ledIndexGlitch2_Ultracorrupt[id] + x] = red_waveCorrupted;
      leds[id][ledIndexGlitch3_Ultracorrupt[id] + x] = red_waveCorrupted;
      leds[id][ledIndexGlitch4_Ultracorrupt[id] + x] = red_waveCorrupted;
      leds[id][ledIndexGlitch5_Ultracorrupt[id] + x] = red_waveCorrupted;
      leds[id][ledIndexGlitch6_Ultracorrupt[id] + x] = red_waveCorrupted;
      leds[id][ledIndexGlitch7_Ultracorrupt[id] + x] = red_waveCorrupted;
      leds[id][ledIndexGlitch8_Ultracorrupt[id] + x] = red_waveCorrupted;
    }
}


void shield_Off(int id){
    /*
    if(g_ShieldOff[id] >= 14){
      g_ShieldOff[id]-=delayBrightness_ShieldOff;
    }

    if(b_ShieldOff[id] >= 30){
      b_ShieldOff[id]-=delayBrightness_ShieldOff;
    }
    */
    if(sat_ShieldOff[id] >= 100){
      sat_ShieldOff[id]-=delayBrightness_ShieldOff;
    }
    
    if(a_ShieldOff[id] >= 100){
      a_ShieldOff[id]-=delayBrightness_ShieldOff;
    }
    
    for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      leds[id][i].setHSV(125, sat_ShieldOff[id], a_ShieldOff[id]);
    }
}

void shield_On(int id){
    
    sat_ShieldOff[id] = 255;
    a_ShieldOff[id] = 255;

    /*
    ledIndexGlitch1_ShieldOn[id] = random8(NUM_LEDS_PER_STRIP);
    ledIndexGlitch2_ShieldOn[id] = random8(NUM_LEDS_PER_STRIP);
    ledIndexGlitch3_ShieldOn[id] = random8(NUM_LEDS_PER_STRIP);
    ledIndexGlitch4_ShieldOn[id] = random8(NUM_LEDS_PER_STRIP);
    ledIndexGlitch5_ShieldOn[id] = random8(NUM_LEDS_PER_STRIP);
    ledIndexGlitch6_ShieldOn[id] = random8(NUM_LEDS_PER_STRIP);
    ledIndexGlitch7_ShieldOn[id] = random8(NUM_LEDS_PER_STRIP);
    ledIndexGlitch8_ShieldOn[id] = random8(NUM_LEDS_PER_STRIP);
    */
    
    for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      //if(i == ledIndexGlitch1_ShieldOn[id] || i == ledIndexGlitch2_ShieldOn[id] || i == ledIndexGlitch3_ShieldOn[id] || i == ledIndexGlitch4_ShieldOn[id] || i == ledIndexGlitch5_ShieldOn[id] || i == ledIndexGlitch6_ShieldOn[id] || i == ledIndexGlitch7_ShieldOn[id] || i == ledIndexGlitch8_ShieldOn[id]){
        //leds[id][i].setRGB(255, 255, 255);  
      //}else{
      leds[id][i] = cyan_ShieldOn;  
      //}
    }
}

void ANIM_TURQUOISE(int id){
  //17
  
  for(int i=0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = paleBlue_Cleansing;
  }
}

void ANIM_PURPLE(int id){
  //18
  
  for(int i=0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = purple_Corrupt;
  }
}

void ANIM_YELLOW(int id){
  //19
  
  for(int i=0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = yellow_On;
  }
}

void ANIM_ORANGE(int id){
  //20
  
  for(int i=0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = orange_ANIM;
  }
}

void ANIM_PURPLE_WIPE(int id){
  //21

  for(int i=0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = purple_Corrupt;
  }
}

void ANIM_BLACK(int id){
  //22
  
  for(int i=0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = empty_off;
  }
}

void ANIM_TURQUOISE_FADE(int id){
  //23 : fade PURPLE to CYAN .3 sec.

  if(hue_TURQUOISE_FADE[id] >= 120){
    hue_TURQUOISE_FADE[id]-=delayHue_TURQUOISE_FADE;
  }
  
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i].setHSV(hue_TURQUOISE_FADE[id], 255, 255);
  }
}

void ANIM_SNAKE_TURQUOISE(int id){
  //24 : .6 sec. CYAN to BLACK
  
  //Brightness Manager
  if(val_SNAKE_TURQUOISE[id] > 0){
    val_SNAKE_TURQUOISE[id]-=delayBrightness_SNAKE_TURQUOISE;
  }
  
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i].setHSV(120, 200, val_SNAKE_TURQUOISE[id]);
  }
}

void ANIM_SNAKE_YELLOW(int id){
  //25 : 1 sec. YELLOW to PURPLE
  
  //Brightness Manager

  if(hue_SNAKE_YELLOW[id] >= 0 && stateAnimSNAKE_YELLOW[id] == true){
    hue_SNAKE_YELLOW[id]-=delayHue_SNAKE_YELLOW;
  }else if(stateAnimSNAKE_YELLOW[id] == true){
    hue_SNAKE_YELLOW[id] = 255;
    stateAnimSNAKE_YELLOW[id] = false;
  }

  if(hue_SNAKE_YELLOW[id] >= 185 && stateAnimSNAKE_YELLOW[id] == false){
    hue_SNAKE_YELLOW[id]-=delayHue_SNAKE_YELLOW;
  }else{
    hue_SNAKE_YELLOW[id] = 185;
  }
  
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i].setHSV(hue_SNAKE_YELLOW[id], 255, 255);
  }
}

void ANIM_GREEN_TURQUOISE(int id){
  //26
  
  for(int i=0; i < NUM_LEDS_PER_STRIP; i++){
    leds[id][i] = greenTurquoise_ANIM;
  }
}
