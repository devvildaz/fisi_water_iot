#include <ArduinoJson.h>
#include <string.h>
#include "Adafruit_Keypad.h"
#include <TinyGPS++.h>

#include <AltSoftSerial.h>

#define USONIC_PING 3
#define USONIC_ECHO 2
#define H_LEN 4
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

enum MODE { CONFIG = 0, RUNNING = 1, FAILED = 2 } mode;

byte rowPins[ROWS] = {11,10,9,8};
byte colPins[COLS] = {4,5,6};

Adafruit_Keypad  customKeypad = Adafruit_Keypad (makeKeymap(keys), rowPins, colPins, ROWS,COLS);

byte idx_read = 0;
char BUFF_HEIGHT[H_LEN];
const double TOTAL_HEIGHT = 100.0;
double percentage = 0.0;
bool up = false;

long duration, inches, cm;

TinyGPSPlus gps;
AltSoftSerial SoftSerial;

struct {
  double lati = 0.0;
  double lon = 0.0;
  double alt = 0.0;
  double percentage = -1.0;
} infoStruct;

void setup() {
  mode = RUNNING;
  BUFF_HEIGHT[3] = '\0';
  SoftSerial.begin(9600);
  Serial.begin(38400);  
  customKeypad.begin();
}



void loop() {
  switch(mode){
    case CONFIG: 
      // keypad configuration
      break;
    case RUNNING: 
      displayInfo();
      triggerSignal();
      
      if (infoStruct.lati != 0.0 && infoStruct.lon != 0.0 && infoStruct.alt != 0.0 && (infoStruct.percentage >= 0.0)) {
        Serial.print("Lat: ");
        Serial.println(infoStruct.lati);
        Serial.print("Lon: ");
        Serial.println(infoStruct.lon);
        Serial.print("Alt: ");
        Serial.println(infoStruct.alt);
        Serial.print("Percentage: ");
        Serial.println(infoStruct.percentage);
        Serial.println();
        Serial.println();
        delay(1500);
      }
      break;
    case FAILED: 
      break;
  }
}

void read_height() {
  customKeypad.tick();
  while(customKeypad.available()){
    keypadEvent e = customKeypad.read();
    
    if(e.bit.EVENT == KEY_JUST_PRESSED) {
      BUFF_HEIGHT[idx_read] = (char)e.bit.KEY;
      idx_read++;
    }
    if(idx_read >= 3){
      mode = RUNNING;
      idx_read = 0;
    }
  }
  Serial.println(BUFF_HEIGHT);
  delay(10);
}


void triggerSignal() {
  pinMode(USONIC_PING, OUTPUT);
  digitalWrite(USONIC_PING, LOW);
  delayMicroseconds(2);
  digitalWrite(USONIC_PING, HIGH);
  delayMicroseconds(10);
  digitalWrite(USONIC_PING, LOW);
  pinMode(USONIC_ECHO, INPUT);
  duration = pulseIn(USONIC_ECHO, HIGH);
  cm = microsecondsToCentimeters(duration);
  percentage= (1.0 - cm/TOTAL_HEIGHT)*100.0;
  if(percentage < 0.0){
    infoStruct.percentage = 0.0;
  } else {
    infoStruct.percentage = percentage;
  }
  //infoStruct.percentage = percentage < 0.0? 0.0: percentage;
  // Serial.println(infoStruct.percentage);
  // Serial.print("Percentage: ");
  // Serial.print(percentage < 0.0? 0.0: percentage );
  //infoStruct.percentage = percentage;
  //Serial.print("%");
  // Serial.println();
}

void displayInfo() {
 
  while (SoftSerial.available() > 0) {
 
    if (gps.encode(SoftSerial.read())) {
 
      if (gps.location.isValid()) {
        infoStruct.lati = gps.location.lat();
        infoStruct.lon = gps.location.lng();
      }
      
 
      if (gps.altitude.isValid()) {
        infoStruct.alt = gps.altitude.meters();
      }
      
    }
  }
  
}


long microsecondsToCentimeters(long microseconds){
  return microseconds * 0.034 / 2;
}
