// hardware libraries to access use the shield

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <OSCBundle.h>
#include <OSCBoards.h>
#include "FastLED.h"
#include <HX711.h>
#include <string.h>
#include "HX711-multi.h"

//LED related variables
#define LED_PIN    22
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    84
#define NUM_STRIPS 7
#define BRIGHTNESS  255
#define FRAMES_PER_SECOND 10 

//Pressure Plate related variable
#define CLK A0
#define CLK2 A1
//--first group of clock time hook to CLK1 
#define DOUT1 A2
#define DOUT2 A3
#define DOUT3 A4
#define DOUT4 A5
//--second group of clock time hook to CLK2
#define DOUT5 A8
#define DOUT6 A9
#define DOUT7 A10
//time out of the tare function ?is it use?
#define TARE_TIMEOUT_SECONDS 4 

byte DOUTS1[4] = {DOUT1,DOUT2,DOUT3,DOUT4};
byte DOUTS2[3] = {DOUT5,DOUT6,DOUT7};
//--array of all the index that are going to be send, needed for unity to understand if we dont have all the tiles
int indexs[7] = {0,1,2,3,4,5,6};

//--pre calculate the different buffer needed
#define CHANNEL1 sizeof(DOUTS1) /sizeof(byte)
#define CHANNEL2 sizeof(DOUTS2) /sizeof(byte)
#define CHANNEL_COUNT CHANNEL1 + CHANNEL2


long int results[CHANNEL_COUNT];
long int results2[CHANNEL_COUNT];
//int channelCount = CHANNEL1 + CHANNEL2;

//--define the channels
HX711MULTI scales(CHANNEL1, DOUTS1, CLK);
HX711MULTI scales2(CHANNEL2, DOUTS2, CLK2);

//datas array use for calculating the outcome of the receive data
int threshold = 7;
bool prevStatus[CHANNEL_COUNT];
bool hasChanged[CHANNEL_COUNT];
bool tileStatus[CHANNEL_COUNT];
uint32_t prevValues[CHANNEL_COUNT];

EthernetUDP Udp;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

//arduino IP address
IPAddress ip(192, 168, 0, 3);
//destination IP adress
IPAddress outIp(192,168,0,10); 

CRGB leds[NUM_STRIPS][NUM_LEDS];

const unsigned int inPort = 12001;
const unsigned int outPort = 15001;
int currentId = 0;

unsigned int intervalRead = 25 ;

unsigned int prevRead = 0;

uint32_t maxReadableValue = 70000;


void setup() {
  Serial.begin(9600);
  //setup ethernet part
  Ethernet.begin(mac, ip);
  Udp.begin(inPort);

  Serial.println(Ethernet.localIP());

  Serial.println(CHANNEL_COUNT);
  
  sendLog(999);

  delay(3000); // sanity delay

  for(int i = 0; i < CHANNEL_COUNT; i++){
    tileStatus[i] = 0; //Initialise status to 0
    prevStatus[i] = 0; //Initialise previous status to 0
    hasChanged[i] = 0; //Initialise the hasChange states
    prevValues[i] = 0;
  }

  // Settings up the led strips
  FastLED.addLeds<CHIPSET, 22, COLOR_ORDER>(leds[0], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 24, COLOR_ORDER>(leds[1], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 26, COLOR_ORDER>(leds[2], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 28, COLOR_ORDER>(leds[3], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 30, COLOR_ORDER>(leds[4], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 32, COLOR_ORDER>(leds[5], NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, 34, COLOR_ORDER>(leds[6], NUM_LEDS).setCorrection( TypicalLEDStrip );

  FastLED.setBrightness( BRIGHTNESS );

  //tare();
  testPattern();

}

void loop() {
  // important! non-blocking listen routine
  Serial.println("loop");
  OSCBundle bundleIN;
  int size;
  if ( (size = Udp.parsePacket()) > 0)
  {

    while (size--)
      bundleIN.fill(Udp.read());
    Serial.println(bundleIN.hasError());

  }
  //if(!bundleIN.hasError()){
  bundleIN.route("/hex", receiveStatus);
  // }
  if(mustReadPressure){
    Serial.println("mustReadPressure");
    readPressurePlate();
  }
  
 // tare();
 //testPattern();
 //delay(30);
 
  delay(100);
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
void tare() {
  //check if the data has finished been read
  //bool tareSuccessful = false;

  //unsigned long tareStartTime = millis();
  //while (!tareSuccessful && millis()<(tareStartTime+TARE_TIMEOUT_SECONDS*10)) {
   // tareSuccessful = scales.tare(1,10000);  //reject 'tare' if still ringing
 // }
}
void receiveStatus(OSCMessage &msg) {  // *note the & before msg
  //testPattern();
  //Serial.println("status" + String(msg.getInt(0)));
  //Testin on 7 leds only
  /*switch(msg.getInt(0)){
    case 0: leds[currentId] = CRGB::Black;    // Black/off
            break;
    case 1: leds[currentId] = CRGB::White;
            break;
    case 2: leds[currentId] = CRGB::Red;
            break;
    }*/
  int theIndex = msg.getFloat(0);
  for(int i = 0; i < CHANNEL_COUNT; i++){
    int indexReceive = theIndex;
    int indexToSend = indexs[i];
    if(indexReceive == indexToSend){
      sendLog(indexs[i]);
      currentId = indexs[i]; 
    } 
  }
  
  int hexStatus = msg.getFloat(1);
  
  switch (hexStatus) {
    case 0: colorWipe(CRGB(50,0,80));    // Black/off
      break;
    case 1: colorWipe(CRGB(75,75,0)); 
      break;
    case 2: colorWipe(CRGB::Red);
      break;
  }


}

void sendData(int ID, int state) {
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
   /*switch (hexState) {
    case 0: colorWipe(CRGB::Black);    // Black/off
      break;
    case 1: colorWipe(CRGB(50,0,30));
      break;
    case 2: colorWipe(CRGB::Red);
      break;
  }*/
}

void sendLog(float data){
  
  float theMessage = data;
  OSCMessage msg("/debug");
  msg.add((int32_t)data);
  
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message

}

void readPressurePlate(){
  //combine the two analogue pin result state into one array
  long int buffer1[CHANNEL1];
  long int buffer2[CHANNEL2];
  scales.read(results);
  scales2.read(results2);
  //memcpy(results, buffer1, sizeof(buffer1));
  //memcpy(results, buffer2, sizeof(buffer2));
  for(int u=0; u<CHANNEL_COUNT; u++){
    Serial.println(results[u]);
  }
  Serial.println("size of results:");
  //Serial.println(CHANNEL_COUNT);
  //Serial.println(CHANNEL1);
  //Serial.println(CHANNEL2);
  //sendLog();
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
          sendData(theIndex,tileStatus[i]);
        }
        
        prevValues[i] = value; 
        prevStatus[i] = tileStatus[i];
        
      }
  }   
}

/*void animationCtrl(int ID){
  Serial.println("------- animationCtrl ------");
  scales.read(results);
  int tileToAnim = ID;
  for(int i = 0; i < scales.get_count(); i++){
    if(hasChanged[i] != 0) {
      Serial.println("has changed");
      if(tileStatus[i] == 1) {
        Serial.println("flash Blue");
        colorWipe(CRGB::Blue,tileToAnim);
      }
      else{
        Serial.println("close Light");
        colorWipe(CRGB::Black,tileToAnim);
      }
    }
  }
}*/

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

