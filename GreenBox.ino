
#include <Adafruit_NeoPixel.h>
#include "RTClib.h"
#include <Wire.h>

RTC_DS1307 RTC;

const int ledStrip = 11;           // the PWM pin the LED is attached to
const int fanPin = 9;
const int ledRingPin = 5;
const int numLedOnRing = 12;
const int buttonStartPin = 2;     // the number of the pushbutton pin
const int buttonTimePin = 4;     // the number of the pushbutton pin
const int amPin = 7;
const int pmPin = 8;
const int waterSensorPin = 0;

long buttonTime=0;

int buttonStartState = 0;         // variable for reading the pushbutton status
int prevButtonStartState = 0;
int buttonTimeState = 0;         // variable for reading the pushbutton status
int prevButtonTimeState = 0;
int timeMod = 0;
int startTime = 7;
 
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numLedOnRing, ledRingPin, NEO_GRB + NEO_KHZ800);

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // This will reflect the time that your sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  pinMode(ledStrip, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(amPin, OUTPUT);
  pinMode(pmPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonStartPin, INPUT);
  pinMode(buttonTimePin, INPUT);
  strip.begin();
  //strip.setBrightness(125); //adjust brightness here
  strip.show(); // Initialize all pixels to 'off'
 // colorWipe(strip.Color(255, 0, 0), 30); // white
 // colorWipe(strip.Color(0, 255, 0), 30); // white
 // colorWipe(strip.Color(0, 0, 255), 30); // white
 // colorWipe(strip.Color(255, 255, 255), 30); // white

  for(int j=0; j<200; j++) {
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0,j,0));
    }
    strip.show();
    delay(15);
  }
  delay(3000);
  for(int j=200; j>1; j--) {
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0,j,0));
    }
    strip.show();
    delay(5);
  }
 // colorWipe(strip.Color(0, 0, 0), 30);
 // strip.setPixelColor(0, strip.Color(0, 255, 0));
  strip.show();
  buttonTime = millis();
}

// the loop routine runs over and over again forever:
void loop() {
  DateTime now = RTC.now();
  analogWrite(fanPin, 60);
  if (isInLightTime()){
    digitalWrite(ledStrip, HIGH);
  }else{
    digitalWrite(ledStrip, LOW);
  }

  timeMod = timeMod + checkButton(buttonTimeState, prevButtonTimeState, buttonTimePin);
  if (timeMod > 23) {
    timeMod=timeMod-24;
  }
  Serial.println(analogRead(waterSensorPin));
  
  startTime = startTime + checkButton(buttonStartState, prevButtonStartState, buttonStartPin);
  if (startTime > 23) {
    startTime=startTime-24;
  }

  for (int i=0; i< numLedOnRing; i++){
    strip.setPixelColor(i, strip.Color(0,0,0));
  }
  
  displayLightTime();
  displayCurrentTime();
  displayWarningLight();
}

boolean isInLightTime(){
  DateTime now = RTC.now();
  if ((now.hour() >= startTime) && (now.hour() <= startTime+13)){
    return true;
  }else if ((startTime+13>23) && ((now.hour() >= startTime) || (now.hour() <= startTime-11))){
    return true;
  }else{
    return false;
  }
}
int checkButton(int buttonState, int prevButtonState, int buttonPin) {
  buttonState = digitalRead(buttonPin);
  int change = 0;
  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (buttonState == HIGH && prevButtonState == LOW && millis() - buttonTime > 500)
  {
    if(millis()-buttonTime < 5000){
    change = 1;
    }
    buttonTime = millis();
  }

  prevButtonState = buttonState;
  return change;
}

void displayLightTime(){
  DateTime now = RTC.now();
  int green = 120;
  int red = 126;

  if (millis() - buttonTime > 5000){
    green = green/8;
    red = red/8;
  }
  for(int i=startTime; i<startTime+13; i++){
    if (now.hour()+timeMod < 12 && i<12){
    strip.setPixelColor(i, strip.Color(red, green, 0));
    } else if (now.hour()+timeMod<12  && i>23){
      strip.setPixelColor(i-24, strip.Color(red, green, 0));
    } else if (now.hour()+timeMod>11  && i >11){
      strip.setPixelColor(i-12, strip.Color(red, green, 0));
    }
    green = green-8;
    if(millis() - buttonTime > 5000){
      green=green+7;
    }
  }
  strip.show();
}

void displayWarningLight(){
  if (analogRead(waterSensorPin) > 150){
    
  uint16_t i, j;
 
  for(j=0; j<255; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(j,0,0));
    }
    strip.show();
    delay(2);
  }
  
  for(j=255; j>0; j--) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(j,0,0));
    }
    strip.show();
    delay(10);
  }
  
  }
}
void displayCurrentTime(){
  DateTime now = RTC.now();
  int pixel = now.hour()+timeMod;
  if(pixel >23){
    pixel = pixel-24;
  }
  if (pixel > 11){
    pixel = pixel-12;
    digitalWrite(amPin, LOW);
    digitalWrite(pmPin, HIGH);
  }else {
    digitalWrite(amPin, HIGH);
    digitalWrite(pmPin, LOW);
  }
  strip.setPixelColor( pixel , strip.Color(0,127,0));
  if (millis() - buttonTime > 5000){
  strip.setPixelColor( pixel , strip.Color(0,15,0));
  }
  pixel = now.minute()/5;
  strip.setPixelColor( pixel , strip.Color(25,127,25));
  if (millis() - buttonTime > 5000){
  strip.setPixelColor( pixel , strip.Color(3,15,3));
  }
  pixel = now.second()/5;
  strip.setPixelColor( pixel , strip.Color(50,127,50));
  if (millis() - buttonTime > 5000){
  strip.setPixelColor( pixel , strip.Color(6,15,6));
  }

  delay(1);
  strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
 
void rainbow(uint8_t wait) {
  uint16_t i, j;
 
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
 
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
 
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
 
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
