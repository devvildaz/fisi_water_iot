#include <ArduinoJson.h>
#include <string.h>
#include "Adafruit_Keypad.h"
#include <SoftwareSerial.h>

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

SoftwareSerial gpsSerial(13,12);

void setup() {
  mode = CONFIG;
  BUFF_HEIGHT[3] = '\0';
  Serial.begin(19200);
  gpsSerial.begin(9600);
  customKeypad.begin();
}

void loop() {
  switch(mode){
    case CONFIG: 
      triggerSignal();
      break;
    case RUNNING: 
      Serial.println(gpsSerial.available());
      while(gpsSerial.available() > 0){
        char c = gpsSerial.read();
        Serial.print(c);
      }
      Serial.println();
      delay(10);
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
  Serial.print(percentage < 0.0? 0.0: percentage );
  Serial.print("%");
  Serial.println();
  delay(500);
}


long microsecondsToCentimeters(long microseconds){
  return microseconds * 0.034 / 2;
}
