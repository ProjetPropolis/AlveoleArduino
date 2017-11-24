// hardware libraries to access use the shield

#include "FastLED.h"
#include <HX711.h>
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
#define FRAMES_PER_SECOND 10 

//Pressure Plate related variable
#define CLK 4
#define CLK2 5
//--first group of clock time hook to CLK1 
#define DOUT1 22
#define DOUT2 21
#define DOUT3 20
#define DOUT4 19
//--second group of clock time hook to CLK2
#define DOUT5 18
#define DOUT6 17
#define DOUT7 16
//time out of the tare function ?is it use?
#define TARE_TIMEOUT_SECONDS 4 

//byte DOUTS1[4] = {DOUT1,DOUT2,DOUT3,DOUT4};
//byte DOUTS2[3] = {DOUT5,DOUT6,DOUT7};

byte DOUTS1[0] = {};
byte DOUTS2[0] = {};
//--array of all the index that are going to be send, needed for unity to understand if we dont have all the tiles
int indexs[7] = {0,1,2,3,4,5,6};

//--pre calculate the different buffer needed
#define CHANNEL1 sizeof(DOUTS1) /sizeof(byte)
#define CHANNEL2 sizeof(DOUTS2) /sizeof(byte)
#define CHANNEL_COUNT CHANNEL1 + CHANNEL2

//--serial communication related variable
byte dataArray1[3] = {NULL, NULL, NULL};//the bigest number we will receive is in the 3 number;
byte dataArray2[3] = {NULL, NULL, NULL};//the bigest number we will receive is in the 3 number;
bool record =0;
int dataBufferIndex = 0;
int lengthOfNbr = 0;

long int results[CHANNEL_COUNT];
long int results2[CHANNEL_COUNT];
//int channelCount = CHANNEL1 + CHANNEL2;

//--define the channels
HX711MULTITEENSY scales(CHANNEL1, DOUTS1, CLK);
HX711MULTITEENSY scales2(CHANNEL2, DOUTS2, CLK2);

//datas array use for calculating the outcome of the receive data
int threshold = 7;
bool prevStatus[CHANNEL_COUNT];
bool hasChanged[CHANNEL_COUNT];
bool tileStatus[CHANNEL_COUNT];
uint32_t prevValues[CHANNEL_COUNT];
int dataId;
int dataState;

CRGB leds[NUM_STRIPS][NUM_LEDS];
int currentId = 0;

unsigned int intervalRead = 25 ;

unsigned int prevRead = 0;

uint32_t maxReadableValue = 70000;

int idSensor = 4;
int stateSensor = 1;

void setup() {
  Serial.begin(115200);
  //setup ethernet part
  //delay(3000); // sanity delay
  Serial.println("serial begin");
  
  
  for(int i = 0; i < CHANNEL_COUNT; i++){
    tileStatus[i] = 0; //Initialise status to 0
    prevStatus[i] = 0; //Initialise previous status to 0
    hasChanged[i] = 0; //Initialise the hasChange states
    prevValues[i] = 0;
  }

  // Settings up the led strips
  FastLED.addLeds<CHIPSET, 6, COLOR_ORDER>(leds[0], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 7, COLOR_ORDER>(leds[1], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 8, COLOR_ORDER>(leds[2], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 9, COLOR_ORDER>(leds[3], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 10, COLOR_ORDER>(leds[4], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 11, COLOR_ORDER>(leds[5], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 12, COLOR_ORDER>(leds[6], NUM_LEDS).setCorrection( TypicalLEDStrip );

  FastLED.setBrightness( BRIGHTNESS );

  //tare();
  //testPattern();

}

int tick = 0;
void loop() {
  // important! non-blocking listen routine
  //Serial.println("loop");
  readTheData();
  if(tick == 500){
    sendHexStatus(6, 3);
    tick = 0;
  }
  if(mustReadPressure){
    //Serial.println("mustReadPressure");
    readPressurePlate();
  }
  tick++;
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
//----!!!!!finish this function and call the other one with array in arg!----
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
  int hexStatus = dataState;

  switch (hexStatus) {
    case 0: colorWipe(CRGB(50,0,80));    // Black/off
      break;
    case 1: colorWipe(CRGB(75,75,0)); 
      break;
    case 2: colorWipe(CRGB::Red);
      break;
  }
}

void decipherPacket(){
  int id;
  int state;
  while (Serial.available() > 0) {
    byte c = Serial.read();
    //if we catch the ascii code for "a" character aka 97
    if(c == 97){
      Serial.println("packet start");
      record = 1;
      dataBufferIndex = 1;
      //reset the nbrArray array to null element
    }
    else if(c == 99){
      Serial.println("packet parser");
      //Serial.println("reading id");
      int bufferId = String((char*)dataArray1).toInt();
      memcpy (&id, &bufferId, sizeof(bufferId));
      //Serial.println("id: " + String(id));
      
      for(int i = 0; i<dataArray1[3]; i++){
        dataArray1[i] = NULL;
      }
      lengthOfNbr = 0;
      dataBufferIndex = 2;
    }
    else if(c == 122){
      Serial.println("packet end");
      int bufferState = String((char*)dataArray2).toInt();
      memcpy (&state, &bufferState, sizeof(bufferState));
      
      for(int i = 0; i<dataArray2[3]; i++){
        dataArray2[i] = NULL;
      }
      
      lengthOfNbr = 0;
      record = 0;
      dataBufferIndex = 0;
    }
    else if(record != 0){
      if(dataBufferIndex == 1){
        dataArray1[lengthOfNbr] = c;
        lengthOfNbr = lengthOfNbr + 1;
      }
      else if(dataBufferIndex == 2){
        dataArray2[lengthOfNbr] = c;
        lengthOfNbr = lengthOfNbr + 1;
      }
    }

    dataId = id;
    dataState = state;
  }
}

void sendHexStatus(int ID, int state){
  int hexId = ID;
  int hexState = indexs[state];
  Serial.println((String)hexId + "," + hexState + "e/hexData");  
}

/*void sendData(int ID, int state) {
    int hexId = ID;
    int hexState = indexs[state];
    OSCMessage msg("/hex");
    msg.add((int32_t)hexId);
    msg.add((int32_t)hexState);

    Udp.beginPacket(outIp, outPort);
      msg.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message

    //delay(20);
    //currentId = hexId;
}*/


void readPressurePlate(){
  //combine the two analogue pin result state into one array
  long int buffer1[CHANNEL1];
  long int buffer2[CHANNEL2];
  scales.read(results);
  scales2.read(results2);
  for(int u=0; u<CHANNEL_COUNT; u++){
    Serial.println(results[u]);
  }
  //Serial.println("size of results:");
  int chn2Index = 0;
  uint32_t value;
  for (int i=0; i<CHANNEL_COUNT; i++) {
      int theIndex = indexs[i];
      if(i < CHANNEL1){
        value = abs(results[i]) * 0.0001; // can we do something more clear and unforgetable then this division
      }else{
        value = abs(results2[chn2Index]) * 0.0001; // can we do something more clear and unforgetable then this division  
        chn2Index = chn2Index + 1;
      }
      if(value < maxReadableValue && value >= 0){
        int32_t delta =value - prevValues[i];
        tileStatus[i] = (delta > threshold) ? 1 :0;
        
        if(delta < maxReadableValue && abs(delta) >= threshold && tileStatus[i] != prevStatus[i] ){
          sendHexStatus(theIndex,tileStatus[i]);
        }
        
        prevValues[i] = value; 
        prevStatus[i] = tileStatus[i];
        
      }
  }   
}

void testPattern() {
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

}


// Fill the dots one after the other with a color
void colorWipe(CRGB color) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[currentId][i] = color;
  }
  FastLED.show();
}

