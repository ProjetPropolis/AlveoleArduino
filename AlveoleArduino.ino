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
//time out of the tare function ?is it use?
#define TARE_TIMEOUT_SECONDS 4 

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
//int colorArray[5][3] = { {25,25,25}, {75,75,0}, {50,0,80}, {155,25,25}, {0,85,112} };
CRGB leds[NUM_LEDS];
int currentId = 0;

unsigned int intervalRead = 25 ;

unsigned int prevRead = 0;

uint32_t maxReadableValue = 70000;

/*==== sub-hexStatus manager ===*/
Chrono masterChrono0_Corrupt;
Chrono masterChrono1_Corrupt;
Chrono masterChrono2_Corrupt;
Chrono masterChrono3_Corrupt;
Chrono masterChrono4_Corrupt;
Chrono masterChrono5_Corrupt;
Chrono masterChrono6_Corrupt;
Chrono masterChrono_Corrupt[7] = {masterChrono0_Corrupt, masterChrono1_Corrupt, masterChrono2_Corrupt, masterChrono3_Corrupt, masterChrono4_Corrupt, masterChrono5_Corrupt, masterChrono6_Corrupt};
Chrono masterChrono0_Ultracorrupt;
Chrono masterChrono1_Ultracorrupt;
Chrono masterChrono2_Ultracorrupt;
Chrono masterChrono3_Ultracorrupt;
Chrono masterChrono4_Ultracorrupt;
Chrono masterChrono5_Ultracorrupt;
Chrono masterChrono6_Ultracorrupt;
Chrono masterChrono_Ultracorrupt[7] = {masterChrono0_Ultracorrupt, masterChrono1_Ultracorrupt, masterChrono2_Ultracorrupt, masterChrono3_Ultracorrupt, masterChrono4_Ultracorrupt, masterChrono5_Ultracorrupt, masterChrono6_Ultracorrupt};
int pastHexStatus = 0;

/*==== on() Variables ===*/
int ledIndex_On[7] = {0, 0, 0, 0, 0, 0, 0};
int ledAnimIndex_On[7] = {0, 0, 0, 0, 0, 0, 0};

/*==== off() Variables ===*/
int ledIndex_Off[7] = {0, 0, 0, 0, 0, 0, 0};

/*==== corrupt() Variables ===*/
Chrono myChrono0_Corrupt;
Chrono myChrono1_Corrupt;
Chrono myChrono2_Corrupt;
Chrono myChrono3_Corrupt;
Chrono myChrono4_Corrupt;
Chrono myChrono5_Corrupt;
Chrono myChrono6_Corrupt;
Chrono myChrono_Corrupt[7] = {myChrono0_Corrupt, myChrono1_Corrupt, myChrono2_Corrupt, myChrono3_Corrupt, myChrono4_Corrupt, myChrono5_Corrupt, myChrono6_Corrupt};
float val_Corrupt[7] = {255, 255, 255, 255, 255, 255, 255};
int stateBrightness_Corrupt[7] = {0, 0, 0, 0, 0, 0, 0};
int ledIndexGlitch1_Corrupt[7];
int ledIndexGlitch2_Corrupt[7];
int ledIndexGlitch3_Corrupt[7];
int ledIndexGlitch4_Corrupt[7];
int ledIndexGlitch5_Corrupt[7];
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
Chrono myChrono_Ultracorrupt[7] = {myChrono0_Ultracorrupt, myChrono1_Ultracorrupt, myChrono2_Ultracorrupt, myChrono3_Ultracorrupt, myChrono4_Ultracorrupt, myChrono5_Ultracorrupt, myChrono6_Ultracorrupt};
float val_Ultracorrupt[7] = {255, 255, 255, 255, 255, 255, 255};
//float brightness_Ultracorrupt[7] = {0, 0, 0, 0, 0, 0, 0};
int stateBrightness_Ultracorrupt[7] = {0, 0, 0, 0, 0, 0, 0};
int ledIndexGlitch1_Ultracorrupt[7];
int ledIndexGlitch2_Ultracorrupt[7];
int ledIndexGlitch3_Ultracorrupt[7];
int ledIndexGlitch4_Ultracorrupt[7];
int ledIndexGlitch5_Ultracorrupt[7];
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
Chrono myChrono_UltracorruptPressed[7] = {myChrono0_UltracorruptPressed, myChrono1_UltracorruptPressed, myChrono2_UltracorruptPressed, myChrono3_UltracorruptPressed, myChrono4_UltracorruptPressed, myChrono5_UltracorruptPressed, myChrono6_UltracorruptPressed};
float val_UltracorruptPressed[7] = {255, 255, 255, 255, 255, 255, 255};
//float brightness_Ultracorrupt[7] = {0, 0, 0, 0, 0, 0, 0};
int delayIndex_UltracorruptPressed = 50;

/*==== cleanser() Variables ===*/
Chrono myChrono0_Cleanser;
Chrono myChrono1_Cleanser;
Chrono myChrono2_Cleanser;
Chrono myChrono3_Cleanser;
Chrono myChrono4_Cleanser;
Chrono myChrono5_Cleanser;
Chrono myChrono6_Cleanser;
Chrono myChrono_Cleanser[7] = {myChrono0_Cleanser, myChrono1_Cleanser, myChrono2_Cleanser, myChrono3_Cleanser, myChrono4_Cleanser, myChrono5_Cleanser, myChrono6_Cleanser};
int ledIndex1_Cleanser[7];
int ledIndex2_Cleanser[7];
int stateLedIndex1_Cleanser[7] = {0, 0, 0, 0, 0, 0, 0};
int stateLedIndex2_Cleanser[7] = {0, 0, 0, 0, 0, 0, 0};
int delayProgressLed_Cleanser = 10;

/*==== cleansing() Variables ===*/
Chrono myChrono0_cleansing;
Chrono myChrono1_cleansing;
Chrono myChrono2_cleansing;
Chrono myChrono3_cleansing;
Chrono myChrono4_cleansing;
Chrono myChrono5_cleansing;
Chrono myChrono6_cleansing;
Chrono myChrono_cleansing[7] = {myChrono0_cleansing, myChrono1_cleansing, myChrono2_cleansing, myChrono3_cleansing, myChrono4_cleansing, myChrono5_cleansing, myChrono6_cleansing};
int ledIndex1_cleansing[7];
int ledIndex2_cleansing[7];
int stateLedIndex1_cleansing[7] = {0, 0, 0, 0, 0, 0, 0};
int stateLedIndex2_cleansing[7] = {0, 0, 0, 0, 0, 0, 0};
int delayProgressLed_cleansing = 10;

/*==== colors Variables ===*/
CRGB yellow_On(200, 100, 15);
CRGB orange_On(110, 0, 0);
CHSV purple_Corrupt(210, 255, 255);
CHSV palePurple_Corrupt(210, 50, 255);
CRGB red_Ultracorrupt(255, 0, 0);
CRGB white_Ultracorrupt(255, 255, 255);
CHSV blue_Cleanser(140, 255, 255);
CHSV paleBlue_cleansing(140, 200, 255);


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

  /*==== corrupt() Setup Variables ===*/
  for(int x = 0; x < 6; x++){
    ledIndexGlitch1_Corrupt[x] = random8(NUM_LEDS-(dashLenght_Corrupt+2));
    ledIndexGlitch2_Corrupt[x] = random8(NUM_LEDS-(dashLenght_Corrupt+2));
    ledIndexGlitch3_Corrupt[x] = random8(NUM_LEDS-(dashLenght_Corrupt+2));
    ledIndexGlitch4_Corrupt[x] = random8(NUM_LEDS-(dashLenght_Corrupt+2));
    ledIndexGlitch5_Corrupt[x] = random8(NUM_LEDS-(dashLenght_Corrupt+2));
  }
     
  /*==== ultracorrupt() Setup Variables ===*/
  for(int x = 0; x < 6; x++){
    ledIndexGlitch1_Ultracorrupt[x] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
    ledIndexGlitch2_Ultracorrupt[x] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
    ledIndexGlitch3_Ultracorrupt[x] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
    ledIndexGlitch4_Ultracorrupt[x] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
    ledIndexGlitch5_Ultracorrupt[x] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
  }

  /*==== ultracorruptPressed() Setup Variables ===*/
  //ledIndexGlitch1_UltracorruptPressed = random8(NUM_LEDS-2);
  //state_UltracorruptPressed = 0;

  /*==== cleanser() Setup Variables ===*/
  for(int x = 0; x < 6; x++){
    ledIndex1_Cleanser[x] = 0;
    ledIndex2_Cleanser[x] = NUM_LEDS*0.5;
  }

  /*==== cleansing() Setup Variables ===*/
  for(int x = 0; x < 6; x++){
    ledIndex1_cleansing[x] = 0;
    ledIndex2_cleansing[x] = NUM_LEDS*0.5;
  }
  
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
    readPressurePlate();
  }
  
  for(int i = 0; i < NUM_STRIPS; i++){
    if(receiveState[i] != prevReceiveState[i]){
      int stripState = receiveState[i];
      Serial.println("update led: " + String(receiveState[i]));
      /*int r = colorArray[stripState][0];
      int g = colorArray[stripState][1];
      int b = colorArray[stripState][2];
      Serial.println("receiveState[i] : " + String(receiveState[i]));
      fill_solid(leds, NUM_LEDS,CRGB(r,g,b));
      FastLED[i].showLeds(BRIGHTNESS);*/
      animCtrl(i, receiveState[dataId], prevReceiveState[dataId]);
      FastLED[i].showLeds(BRIGHTNESS);
    }
    
    prevReceiveState[i] = receiveState[i];
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
  for(int i = 0; i < CHANNEL_COUNT; i++){
    int indexReceive = theIndex;
    int indexToSend = indexs[i];
    if(indexReceive == indexToSend){
      currentId = indexs[i]; 
    } 
  }
  int hexStatus = receiveState[dataId];
  //Serial.println("receiveState: " + String(receiveState[dataId]));
  //if you add more state to this switch/case go update the variable nbrOfState and add the color in the colorArray
  /*switch (hexStatus) {
    case 0: colorWipe(CRGB(0,0,0),dataId);    // Black/off
      break;
    case 1: colorWipe(CRGB(75,75,0),dataId);  //yellow/on
      break;
    case 2: colorWipe(CRGB(50,0,80),dataId);  //purple/corruption
      break;
    case 3: colorWipe(CRGB(155,25,25),dataId);//red/ultra-corrupted
      break;
    case 4: colorWipe(CRGB(0,85,112),dataId); //blue/cleanse
      break;
  }*/
}

void decipherPacket(){
  int state;
  while (Serial.available() > 0) {
    byte c = Serial.read();
    //if we catch the ascii code for "a" character aka 97
    if(c == 97){
      //Serial.println("packet start");
      record = 1;
      dataBufferIndex = 1;
      //reset the nbrArray array to null element
    }
    else if(c == 99){
      //Serial.println("finish id go to state");
      //Serial.println("reading id");
      int bufferId = String((char*)dataArray1).toInt();
      memcpy (&dataId, &bufferId, sizeof(bufferId));
      //Serial.println("id receive from python: " + String(dataId));
      for(int i = 0; i<dataArray1[3]; i++){
        dataArray1[i] = NULL;
      }
      dataBufferIndex = 2;
    }
    else if(c == 122){
      //Serial.println("packet end");
      int bufferState = String((char*)dataArray2).toInt();
      //Serial.println("packetEnd value of buffer: " + bufferState);
      memcpy (&receiveState[dataId], &bufferState, sizeof(bufferState));
      //Serial.println("printing led #: " + String(dataId)+ " giving state " + String(receiveState));
      //Serial.println("state receive from python: " + String(receiveState));
      for(int i = 0; i<dataArray2[3]; i++){
        dataArray2[i] = NULL;
      }
      
      lengthOfNbr1 = 0;
      lengthOfNbr2 = 0;
      record = 0;
      dataBufferIndex = 0;
      //int r = colorArray[stripState][0];
      //int g = colorArray[stripState][1];
      //int b = colorArray[stripState][2];
      //fill_solid(leds, NUM_LEDS,CRGB(r,g,b));
      //FastLED[dataId].showLeds(BRIGHTNESS);
      //prevReceiveState[dataId] = receiveState[dataId];
      animCtrl(dataId, receiveState[dataId], prevReceiveState[dataId]);
      FastLED[dataId].showLeds(BRIGHTNESS);
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
        Serial.println("update led: " + String(receiveState[i]));
        /*int r = colorArray[stripState][0];
        int g = colorArray[stripState][1];
        int b = colorArray[stripState][2];
        Serial.println("receiveState[i] : " + String(receiveState[i]));
        fill_solid(leds, NUM_LEDS,CRGB(r,g,b));
        FastLED[i].showLeds(BRIGHTNESS);*/
      }
      
      prevReceiveState[i] = receiveState[i];
    }
    //Serial.println("dataId : " + String(dataId));
    //Serial.println("dataState : " + String(dataState));
  }
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
          Serial.println("prevStatus : " + String(prevStatus[i]));
          Serial.println("tileStatus : " + String(tileStatus[i]));
          sendHexStatus(indexs[theIndex],tileStatus[i]);
          prevStatus[i] = tileStatus[i];
        }
        
        prevValues[i] = value;
        prevStatus[i] = tileStatus[i];
        
      }
  }   
}
/*void testPattern() {
  for (int i = 0; i < 6; i ++) {
    currentId = i;
    colorWipe(CRGB::White);
  }


  delay(500);

  for (int i = 0; i < 6; i ++) {
    currentId = i;
    colorWipe(CRGB::Black);
  }

  currentId = 0;
  colorWipe(CRGB::Red);


  delay(500);
  currentId = 1;
  colorWipe(CRGB::Blue);


  delay(500);
  currentId = 2;
  colorWipe(CRGB::Red);

   delay(500);
  currentId = 3;
  colorWipe(CRGB::Blue);

   delay(500);
  currentId = 4;
  colorWipe(CRGB::Red);

  delay(500);
  currentId = 5;
  colorWipe(CRGB::Blue);

  
   delay(500);
  currentId = 6;
  colorWipe(CRGB::Red);

  delay(500);
  for (int i = 0; i < 6; i ++) {
    currentId = i;
    colorWipe(CRGB::Black);
  }

}*/


// Fill the dots one after the other with a color
/*void colorWipe(CRGB color, int id) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[id][i] = color;
  }
  FastLED.show();
}*/

void animCtrl(int id, int state, int prevState){
  
     switch (state) {
        case 0: off(id);    
          break;
        case 1: on(id);    
          break;
        case 2:  
        if(prevState == 1){
          masterChrono_Corrupt[id].restart();
        }
        if(!masterChrono_Corrupt[id].hasPassed(6000)){
          on(id);
          corrupt(id);
        }else{
          corrupt(id);
        }
          break;
        case 3:
          if(prevState == 3){
            masterChrono_Ultracorrupt[id].restart();
          }
          if(!masterChrono_Ultracorrupt[id].hasPassed(3000)){
            ultracorruptPressed(id); 
          }else{
            ultracorrupt(id); 
          }
          break;
        case 4: cleanser(id);
        case 5: cleansing(id);
    }
}

void off(int id){

     //Writes a Full Black Strip 
  
     if(ledIndex_Off[id] < NUM_LEDS){
      leds[ledIndex_Off[id]].setRGB(200, 100, 15);
      ledIndex_Off[id]++;
     }else{
      ledIndex_Off[id] = 0;
     }
     //FastLED.show();
     
}


void on(int id){

     //Writes yellow 1 LED after the other and 1 orange LED every 2 LEDs
   
     if(ledIndex_On[id] < NUM_LEDS){
      leds[ledIndex_On[id]] = yellow_On;
      ledIndex_On[id]++;
     }else{
      ledIndex_On[id] = 0;
     }
     
     if(ledAnimIndex_On[id] < NUM_LEDS){
      leds[ledAnimIndex_On[id]] = orange_On;
      ledAnimIndex_On[id]+= 2;
     }else{
      ledAnimIndex_On[id] = 0;
     }
     
}


void corrupt(int id){

    //Writes 5 dashes of purple in the strip, 5 index are generated(with random drunk) to change the dashes' positions
    
    //Brightness Manager
    if(stateBrightness_Corrupt[id] == 0){
      if(val_Corrupt[id] > 150){
        val_Corrupt[id]-=delayBrightness_Corrupt;
      }else {
        val_Corrupt[id] = 150;
        stateBrightness_Corrupt[id] = 1;
      }
    }else if(stateBrightness_Corrupt[id] == 1){
      if(val_Corrupt[id] < 255){
        val_Corrupt[id]+=delayBrightness_Corrupt;
      }else{
        val_Corrupt[id] = 255;
        stateBrightness_Corrupt[id] = 0;
      }
    }

    //Writing BLACK for the 5 Dashes
    for(int x = 0; x < dashLenght_Corrupt; x++){
      leds[ledIndexGlitch1_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[ledIndexGlitch2_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[ledIndexGlitch3_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[ledIndexGlitch4_Corrupt[id] + x].setHSV(210, 255, 0);
      leds[ledIndexGlitch5_Corrupt[id] + x].setHSV(210, 255, 0);
    }
    leds[ledIndexGlitch1_Corrupt[id]+2].setHSV(210, 255, 0);
    leds[ledIndexGlitch2_Corrupt[id]+2].setHSV(210, 255, 0);
    
    //Changing 5 Dashes starting index
    if(myChrono_Corrupt[dataId].hasPassed(delayIndex_Corrupt)){
      ledIndexGlitch1_Corrupt[id] = constrain(random8(ledIndexGlitch1_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch1_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+2));
      ledIndexGlitch2_Corrupt[id] = constrain(random8(ledIndexGlitch2_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch2_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+2));
      ledIndexGlitch3_Corrupt[id] = constrain(random8(ledIndexGlitch3_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch3_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+2));
      ledIndexGlitch4_Corrupt[id] = constrain(random8(ledIndexGlitch4_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch4_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+2));
      ledIndexGlitch5_Corrupt[id] = constrain(random8(ledIndexGlitch5_Corrupt[id]-stepRandom_Corrupt, ledIndexGlitch5_Corrupt[id]+stepRandom_Corrupt+1), 0, NUM_LEDS-(dashLenght_Corrupt+2));
      myChrono_Corrupt[id].restart();
    }

    //Writing PURPLE for the 5 Dashes
    for(int x = 0; x < dashLenght_Corrupt; x++){
      leds[ledIndexGlitch1_Corrupt[id] + x] = purple_Corrupt;
      leds[ledIndexGlitch2_Corrupt[id] + x] = purple_Corrupt;
      leds[ledIndexGlitch3_Corrupt[id] + x] = purple_Corrupt;
      leds[ledIndexGlitch4_Corrupt[id] + x] = purple_Corrupt;
      leds[ledIndexGlitch5_Corrupt[id] + x] = purple_Corrupt;
    }
    leds[ledIndexGlitch1_Corrupt[id]+2] = palePurple_Corrupt;
    leds[ledIndexGlitch2_Corrupt[id]+2] = palePurple_Corrupt;

}


void ultracorrupt(int id){

    //Writes 5 dashes of red in the strip, 5 index are generated(with random) to change the dashes' positions

    /*
    //Brightness Manager
    if(stateBrightness_Ultracorrupt == 0){
      if(val_Ultracorrupt > 100){
        val_Ultracorrupt-=delayBrightness_Ultracorrupt;
      }else{
        val_Ultracorrupt = 100;
        stateBrightness_Ultracorrupt = 1;
      }
    }else if(stateBrightness_Ultracorrupt == 1){
      if(val_Ultracorrupt < 255){
        val_Ultracorrupt+=delayBrightness_Ultracorrupt;
      }else{
        val_Ultracorrupt = 255;
        stateBrightness_Ultracorrupt = 0;
      }
    }
    */
    //Writing BLACK for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      leds[ledIndexGlitch1_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[ledIndexGlitch2_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[ledIndexGlitch3_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[ledIndexGlitch4_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[ledIndexGlitch5_Ultracorrupt[id] + x].setHSV(0, 255, 0);
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
      leds[ledIndexGlitch1_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[ledIndexGlitch2_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[ledIndexGlitch3_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[ledIndexGlitch4_Ultracorrupt[id] + x] = red_Ultracorrupt;
      leds[ledIndexGlitch5_Ultracorrupt[id] + x] = red_Ultracorrupt;
      //FastLED.show();
    }
    
}


void ultracorruptPressed(int id){
  
    //Writing BLACK for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      leds[ledIndexGlitch1_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[ledIndexGlitch2_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[ledIndexGlitch3_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[ledIndexGlitch4_Ultracorrupt[id] + x].setHSV(0, 255, 0);
      leds[ledIndexGlitch5_Ultracorrupt[id] + x].setHSV(0, 255, 0);
    }

    //Changing 5 Dashes starting index
    if(myChrono_UltracorruptPressed[id].hasPassed(delayIndex_UltracorruptPressed)){
      ledIndexGlitch1_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      ledIndexGlitch2_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      ledIndexGlitch3_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      ledIndexGlitch4_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      ledIndexGlitch5_Ultracorrupt[id] = random8(NUM_LEDS-(dashLenght_Ultracorrupt+2));
      myChrono_Ultracorrupt[id].restart();
    }

    //Writing RED for the 5 Dashes
    for(int x = 0; x < dashLenght_Ultracorrupt; x++){
      leds[ledIndexGlitch1_Ultracorrupt[id] + x] = white_Ultracorrupt;
      leds[ledIndexGlitch2_Ultracorrupt[id] + x] = white_Ultracorrupt;
      leds[ledIndexGlitch3_Ultracorrupt[id] + x] = white_Ultracorrupt;
      leds[ledIndexGlitch4_Ultracorrupt[id] + x] = white_Ultracorrupt;
      leds[ledIndexGlitch5_Ultracorrupt[id] + x] = white_Ultracorrupt;
      //FastLED.show();
    }
    
}


void cleanser(int id){
  
    if(myChrono_Cleanser[id].hasPassed(delayProgressLed_Cleanser)){
       if(stateLedIndex1_Cleanser[id] == 0 && stateLedIndex2_Cleanser[id] == 0){
         //Writing BLUE for the strip's first half from firstLED
         if(ledIndex1_Cleanser[id] < (NUM_LEDS*0.5)-1){
           leds[ledIndex1_Cleanser[id]] = blue_Cleanser;
           ledIndex1_Cleanser[id]++;
         }else{
           leds[ledIndex1_Cleanser[id]] = blue_Cleanser;
           ledIndex1_Cleanser[dataId] = 0;
           stateLedIndex1_Cleanser[id] = 1;
         }
         //Writing BLUE for the strip's second half from middleLED
         if(ledIndex2_Cleanser[id] < NUM_LEDS-1){
           leds[ledIndex2_Cleanser[id]] = blue_Cleanser;
           ledIndex2_Cleanser[id]++;
         }else{
           leds[ledIndex2_Cleanser[id]] = blue_Cleanser;
           ledIndex2_Cleanser[id] = NUM_LEDS*0.5;
           stateLedIndex2_Cleanser[id] = 1;
         }
       }else if(stateLedIndex1_Cleanser[id] == 1 && stateLedIndex2_Cleanser[id] == 1){
         //Writing BLACK for the strip's first half from firstLED
         if(ledIndex1_Cleanser[id] < (NUM_LEDS*0.5)-1){
           leds[ledIndex1_Cleanser[id]].setHSV(140, 255, 0);
           ledIndex1_Cleanser[id]++;
         }else{
           leds[ledIndex1_Cleanser[id]].setHSV(140, 255, 0);
           ledIndex1_Cleanser[id] = (NUM_LEDS*0.5)-1;
           stateLedIndex1_Cleanser[id] = 2;
         }
         //Writing BLACK for the strip's second half from middleLED
         if(ledIndex2_Cleanser[id] < NUM_LEDS-1){
           leds[ledIndex2_Cleanser[id]].setHSV(140, 255, 0);
           ledIndex2_Cleanser[id]++;
         }else{
           leds[ledIndex2_Cleanser[id]].setHSV(140, 255, 0);
           ledIndex2_Cleanser[id] = NUM_LEDS-1;
           stateLedIndex2_Cleanser[id] = 2;
         }
       }else if(stateLedIndex1_Cleanser[id] == 2 && stateLedIndex2_Cleanser[id] == 2){
         //Writing BLUE for the strip's first half from middleLED
         if(ledIndex1_Cleanser[id] > 0){
           leds[ledIndex1_Cleanser[id]] = blue_Cleanser;
           ledIndex1_Cleanser[id]--;
         }else{
           leds[ledIndex1_Cleanser[id]] = blue_Cleanser;
           ledIndex1_Cleanser[id] = (NUM_LEDS*0.5)-1;
           stateLedIndex1_Cleanser[id] = 3;
         }
         //Writing BLUE for the strip's second half from lastLED
         if(ledIndex2_Cleanser[id] > NUM_LEDS*0.5){
           leds[ledIndex2_Cleanser[id]] = blue_Cleanser;
           ledIndex2_Cleanser[id]--;
         }else{
           leds[ledIndex2_Cleanser[id]] = blue_Cleanser;
           ledIndex2_Cleanser[id] = NUM_LEDS-1;
           stateLedIndex2_Cleanser[id] = 3;
         }
       }else if(stateLedIndex1_Cleanser[id] == 3 && stateLedIndex2_Cleanser[id] == 3){
         //Writing BLACK for the strip's second half from middleLED
         if(ledIndex1_Cleanser[id] > 0){
           leds[ledIndex1_Cleanser[id]].setHSV(140, 255, 0);
           ledIndex1_Cleanser[id]--;
         }else{
           leds[ledIndex1_Cleanser[id]].setHSV(140, 255, 0);
           ledIndex1_Cleanser[id] = 0;
           stateLedIndex1_Cleanser[id] = 0;
         }
         //Writing BLACK for the strip's second half from lastLED
         if(ledIndex2_Cleanser[id] > NUM_LEDS*0.5){
           leds[ledIndex2_Cleanser[id]].setHSV(140, 255, 0);
           ledIndex2_Cleanser[id]--;
         }else{
           leds[ledIndex2_Cleanser[id]].setHSV(140, 255, 0);
           ledIndex2_Cleanser[id] = NUM_LEDS*0.5;
           stateLedIndex2_Cleanser[id] = 0;
         }
       }
       myChrono_Cleanser[id].restart();
    }
    //FastLED.show();
    
}


void cleansing(int id){

    if(myChrono_cleansing[id].hasPassed(delayProgressLed_cleansing)){
       if(stateLedIndex1_cleansing[id] == 0 && stateLedIndex2_cleansing[id] == 0){
         //Writing BLUE for the strip's first half from firstLED
         if(ledIndex1_cleansing[id] < (NUM_LEDS*0.5)-1){
           leds[ledIndex1_cleansing[id]] = paleBlue_cleansing;
           ledIndex1_cleansing[id]++;
         }else{
           leds[ledIndex1_cleansing[id]] = paleBlue_cleansing;
           ledIndex1_cleansing[id] = 0;
           stateLedIndex1_cleansing[id] = 1;
         }
         //Writing BLUE for the strip's second half from middleLED
         if(ledIndex2_cleansing[id] < NUM_LEDS-1){
           leds[ledIndex2_cleansing[id]] = paleBlue_cleansing;
           ledIndex2_cleansing[id]++;
         }else{
           leds[ledIndex2_cleansing[id]] = paleBlue_cleansing;
           ledIndex2_cleansing[id] = NUM_LEDS*0.5;
           stateLedIndex2_cleansing[id] = 1;
         }
       }else if(stateLedIndex1_cleansing[id] == 1 && stateLedIndex2_cleansing[id] == 1){
         //Writing YELLOW for the strip's first half from firstLED
         if(ledIndex1_cleansing[id] < (NUM_LEDS*0.5)-1){
           leds[ledIndex1_cleansing[id]] = yellow_On;
           ledIndex1_cleansing[id]++;
         }else{
           leds[ledIndex1_cleansing[id]] = yellow_On;
           ledIndex1_cleansing[id] = (NUM_LEDS*0.5)-1;
           stateLedIndex1_cleansing[id] = 2;
         }
         //Writing YELLOW for the strip's second half from middleLED
         if(ledIndex2_cleansing[id] < NUM_LEDS-1){
           leds[ledIndex2_cleansing[id]] = yellow_On;
           ledIndex2_cleansing[id]++;
         }else{
           leds[ledIndex2_cleansing[id]] = yellow_On;
           ledIndex2_cleansing[id] = NUM_LEDS-1;
           stateLedIndex2_cleansing[id] = 2;
         }
       }else if(stateLedIndex1_cleansing[id] == 2 && stateLedIndex2_cleansing[id] == 2){
         //Writing BLUE for the strip's first half from middleLED
         if(ledIndex1_cleansing[id] > 0){
           leds[ledIndex1_cleansing[id]] = paleBlue_cleansing;
           ledIndex1_cleansing[id]--;
         }else{
           leds[ledIndex1_cleansing[id]] = paleBlue_cleansing;
           ledIndex1_cleansing[id] = (NUM_LEDS*0.5)-1;
           stateLedIndex1_cleansing[id] = 3;
         }
         //Writing BLUE for the strip's second half from lastLED
         if(ledIndex2_cleansing[id] > NUM_LEDS*0.5){
           leds[ledIndex2_cleansing[id]] = paleBlue_cleansing;
           ledIndex2_cleansing[id]--;
         }else{
           leds[ledIndex2_cleansing[id]] = paleBlue_cleansing;
           ledIndex2_cleansing[id] = NUM_LEDS-1;
           stateLedIndex2_cleansing[id] = 3;
         }
       }else if(stateLedIndex1_cleansing[id] == 3 && stateLedIndex2_cleansing[id] == 3){
         //Writing YELLOW for the strip's first half from middleLED
         if(ledIndex1_cleansing[id] > 0){
           leds[ledIndex1_cleansing[id]] = yellow_On;
           ledIndex1_cleansing[id]--;
         }else{
           leds[ledIndex1_cleansing[id]] = yellow_On;
           ledIndex1_cleansing[id] = 0;
           stateLedIndex1_cleansing[id] = 0;
         }
         //Writing YELLOW for the strip's second half from lastLED
         if(ledIndex2_cleansing[id] > NUM_LEDS*0.5){
           leds[ledIndex2_cleansing[id]] = yellow_On;
           ledIndex2_cleansing[id]--;
         }else{
           leds[ledIndex2_cleansing[id]] = yellow_On;
           ledIndex2_cleansing[id] = NUM_LEDS*0.5;
           stateLedIndex2_cleansing[id] = 0;
         }
       }
       myChrono_cleansing[id].restart();
    }
    //FastLED.show();

}

