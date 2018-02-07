#include "HX711.h"

#define NUMCHANNEL 7
bool sensorOrientation[NUMCHANNEL]; //IF TRUE = pressure plate is positive and go negative. IF FALSE = pressure plate is negative and go positive
long int sensorStartValue[NUMCHANNEL];
long int sensorThreshold[NUMCHANNEL]; //originaly not an array and has 10 000 has value
int resistance = 2; //Number of time the threshold is multiply. Basic threshold are 10% of their initial value and each resistance multiply that number
bool calibrationState = 0;
int countCalibrated = 0;
int prevTileStatus[7];
int tileStatus[7];
bool isCalibrated[7];

bool prevTilePressure[NUMCHANNEL];
bool tilePressure[NUMCHANNEL];
// HX711.DOUT	- pin #A1
// HX711.PD_SCK	- pin #A0

int indexs[7] = {0, 1, 2, 3, 4, 5, 6};

HX711 scale0(0, 1);		// parameter "gain" is ommited; the default value 128 is used by the library
HX711 scale1(3, 4);
HX711 scale2(9, 10);
HX711 scale3(11, 12);
HX711 scale4(15, 16);
HX711 scale5(17, 18);
HX711 scale6(19, 22);

unsigned long lastTime;

unsigned long blinkLastTime;

void setup() {


  
  delay(3000);
  Serial.begin(115200);
  calibrateHex();
  for (int i = 0; i < NUMCHANNEL; i++) {
    //initialize data array
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

}

int currentSensor = 0;

void loop() {

  Serial.println("reading sensor:" + String(currentSensor));
  readSensor(currentSensor);
  currentSensor = (currentSensor + 1) % NUMCHANNEL;
  //Serial.println(calibrationState);
  
  if(calibrationState == 0){
    calibrateHex();
  }
  if(currentSensor == (NUMCHANNEL - 1)){
    Serial.println("loop  ---------------------------------------------------");
    delay(500);
  } 
}

void calibrateHex() {
  for (int i = 0; i < 7; i++) {
    long int sensorValue = 0;
    switch (i) {
      case 0:
        if ( scale0.is_ready() == false ) break;
        sensorValue = scale0.read();
        calibrateSensor(sensorValue, i);
        break;
      case 1:
        if ( scale1.is_ready() == false ) break;
        sensorValue = scale1.read();
        calibrateSensor(sensorValue, i);
        break;
      case 2:
        if ( scale2.is_ready() == false ) break;
        sensorValue = scale2.read();
        calibrateSensor(sensorValue, i);
        break;
      case 3:
        if ( scale3.is_ready() == false ) break;
        sensorValue = scale3.read();
        calibrateSensor(sensorValue, i);
        break;
      case 4:
        if ( scale4.is_ready() == false ) break;
        sensorValue = scale4.read();
        calibrateSensor(sensorValue, i);
        break;
      case 5:
        if ( scale5.is_ready() == false ) break;
        sensorValue = scale5.read();
        calibrateSensor(sensorValue, i);
        break;
      case 6:
        if ( scale6.is_ready() == false ) break;
        sensorValue = scale6.read();
        calibrateSensor(sensorValue, i);
        break;
    }
    if(sensorValue != 0){
      countCalibrated = countCalibrated + 1;
    }
  }
  if(countCalibrated == 7){
    Serial.println("all calibrated");
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
  if (sensorValue > 0) {
    theThreshold = abs((sensorValue * 0.1) * resistance) ;
    sensorOrientation[sensorId] = true;
    sensorThreshold[sensorId] = theThreshold;
  } else {

    theThreshold = abs((sensorValue * 0.1) * resistance) ;
    sensorOrientation[sensorId] = false;
    sensorThreshold[sensorId] = theThreshold;
  }
  sensorStartValue[sensorId] = sensorValue;
}

int readSensor(int sensorId) {
  long int value;

  bool valid = false;
  switch (sensorId) {
    case 0:
      //Serial.println("preRead :" + String(scale0.readNonBlocking(&value)));
      valid = scale0.readNonBlocking(&value);
      break;
    case 1:
      //Serial.println("preRead :" + String(scale1.readNonBlocking(&value)));
      valid = scale1.readNonBlocking(&value);
      break;
    case 2:
      //Serial.println("preRead :" + String(scale2.readNonBlocking(&value)));
      valid = scale2.readNonBlocking(&value);
      break;
    case 3:
      //Serial.println("preRead :" + String(scale3.readNonBlocking(&value)));
      valid = scale3.readNonBlocking(&value);
      break;
    case 4:
      //Serial.println("preRead :" + String(scale4.readNonBlocking(&value)));
      valid = scale4.readNonBlocking(&value);
      break;
    case 5:
      //Serial.println("preRead :" + String(scale5.readNonBlocking(&value)));
      valid = scale5.readNonBlocking(&value);
      break;
    case 6:
      //Serial.println("preRead :" + String(scale6.readNonBlocking(&value)));
      valid = scale6.readNonBlocking(&value);
      break;
  }
  if ( valid ) checkThreshold(value, sensorId);
}
void checkThreshold(long int val, int id) {
  int index = id;
  long int currentValue = val;
  long int difference;
  Serial.print("------------------------------------value of sensor #");
  Serial.print(index);
  Serial.print(" : ");
  Serial.println(currentValue);
  if (sensorOrientation[index]) {
    difference = sensorStartValue[index] - currentValue;
    if (difference > sensorThreshold[index]) {
      tilePressure[index] = true;
    }
    else {
      tilePressure[index] = false;
    }
  } else {
    long int startValue = abs(sensorStartValue[index]);
    difference =  currentValue + startValue ;
    if (difference > sensorThreshold[index]) {
      tilePressure[index] = true;
    }
    else {
      tilePressure[index] = false;
    }
  }
  if (tilePressure[index] != prevTilePressure[index]) {
    if (tilePressure[index]) {
      tileStatus[index] = 1;
    } else {
      tileStatus[index] = 0;
    }
  }
  prevTileStatus[index] = tileStatus[index];
  prevTilePressure[index] = tilePressure[index];
}
