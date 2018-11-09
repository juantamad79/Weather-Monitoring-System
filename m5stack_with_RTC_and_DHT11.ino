// Example sketch for interfacing with the DS1302 timekeeping chip.
//
// Copyright (c) 2009, Matt Sparks
// All rights reserved.
//
// http://quadpoint.org/projects/arduino-ds1302
#include <stdio.h>
#include <DS1302.h>
#include <TimeLib.h>
#include <M5Stack.h>
#include "utility/MPU9250.h"


namespace {

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
const int kCePin   = 18;                   // Chip Enable
const int kIoPin   = 21;                    // Input/Output
const int kSclkPin = 22;                    // Serial Clock
const int kGatePin = 19;                   // MI Pin
//#define DS1302_GND_PIN 18                   // Gnd Pin for RTC
//#define DS1302_VCC_PIN 23                   // VCC Pin for RTC

const int enable_rtc_time_set = 0;          // enable this if you want to set new time/date in RTC
int sensorPin = A0;                         // select the input pin for the voltage measurement signal
int sensorValue = 0;                        // variable to store the value coming from the sensor
float voltageValue = 0;                     // Initial value for voltage measurement
#define V_REF 5090                          // Reference value 
#define precision_val 16
int debug = 1;


volatile unsigned long duration=0;
unsigned char i[5];
unsigned int j[40];
unsigned char value=0;
unsigned answer=0;
int z=0;
int b=1;



DS1302 rtc(kCePin, kIoPin, kSclkPin);       // Create a DS1302 object.

void printTime() {                          // Function to display current time/date in serial monitor
  // Get the current time and date from the chip.
  time_t timeNow = now();
  // Name the day of the week.
  //const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           year(timeNow), month(timeNow), day(timeNow),
           hour(timeNow), minute(timeNow), second(timeNow));

  // Print the formatted string to serial so we can see the time.
  Serial.print(buf);
  M5.Lcd.setCursor(100, 220);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf(buf);
}

}  // namespace

void setup() {
  Serial.begin(9600);
  //pinMode(DS1302_GND_PIN, OUTPUT);
  //pinMode(DS1302_VCC_PIN, OUTPUT);  

  // Enable RTC clock vcc pin and gnd pin
  //digitalWrite(DS1302_VCC_PIN, HIGH);
  //digitalWrite(DS1302_GND_PIN, LOW);
  
  if (enable_rtc_time_set) { 
    // Initialize a new chip by turning off write protection and clearing the
    // clock halt flag. These methods needn't always be called. See the DS1302
    // datasheet for details.
    rtc.writeProtect(false);
    rtc.halt(false);
  
    // Make a new time object to set the date and time.
    // Sunday, September 22, 2013 at 01:38:50.
    Time t(2018, 11, 9, 0, 19, 15, Time::kThursday);
  
    // Set the time and date on the chip.
     rtc.time(t);
  }
  // Retrieve the time and date from RTC
  Time t = rtc.time();
  // Set arduino internal clock the time and date retrieved from RTC
  setTime(t.hr,t.min,t.sec,t.date,t.mon,t.yr);
  // Disable the power of RTC clock
  //digitalWrite(DS1302_VCC_PIN, LOW);
  //digitalWrite(DS1302_GND_PIN, LOW);
  // initialize the M5Stack object
  M5.begin();
  //M5.startupLogo();
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);

}

// Loop and print the time every second.

void loop() {
  M5.lcd.setBrightness(100);
  printTime();
  get_temp_hum();
  //sensorValue = analogRead(sensorPin);
  //voltageValue = get_voltage_meas(sensorPin);
  if (debug==1) {
    //Serial.print(" Analog Value: ");
    //Serial.print(sensorValue);
    //Serial.print(" Voltage Value: ");
    //Serial.println(voltageValue);
  }
  delay(1000);
  M5.Lcd.drawRect(90, 220, 130 ,10, BLACK);
  M5.Lcd.fillRect(90, 220, 130 ,10, BLACK);

}

float get_voltage_meas(int port){
  int rawvalue = 0;
  for (int i=0; i < precision_val; i++){
    rawvalue = rawvalue +  analogRead(port);
  }
  rawvalue = rawvalue / precision_val;
  float voltage_val = ((rawvalue / 1023.0)* V_REF * 5);
  voltage_val = voltage_val /1000.00; 
  return voltage_val;
}

void get_temp_hum(){
  while(1){
    delay(1000);
    pinMode(kGatePin,OUTPUT);
    digitalWrite(kGatePin,LOW);
    delay(20);
    digitalWrite(kGatePin,HIGH);
    pinMode(kGatePin,INPUT_PULLUP);//by default it will become high due to internal pull up
    // delayMicroseconds(40);
    
    
    duration=pulseIn(kGatePin, LOW);
    if(duration <= 84 && duration >= 72){
      while(1)
      {
        duration=pulseIn(kGatePin, HIGH);
        
        if(duration <= 26 && duration >= 20){
        value=0;}
        
        else if(duration <= 74 && duration >= 65){
        value=1;}
        
        else if(z==40){
        break;}
        
        i[z/8]|=value<<(7- (z%8));
        j[z]=value;
        z++;
      }
    }
    answer=i[0]+i[1]+i[2]+i[3];
      
    if(answer==i[4] && answer!=0){
      M5.Lcd.setTextSize(3);
      M5.Lcd.setCursor(20, 50);
      M5.Lcd.printf("Temperature: ");
      M5.Lcd.drawRect(235, 50, 27 ,54, BLACK);
      M5.Lcd.fillRect(235, 50, 27 ,54, BLACK);
      M5.Lcd.setCursor(235, 50);
      M5.Lcd.print(i[2]);
      M5.Lcd.setCursor(75, 80);
      M5.Lcd.printf("Humidity: ");
      M5.Lcd.drawRect(235, 80, 27 ,54, BLACK);
      M5.Lcd.fillRect(235, 80, 27 ,54, BLACK);
      M5.Lcd.setCursor(235, 80);
      M5.Lcd.print(i[0]);
      if (debug==1){ 
        Serial.print("Temp = ");
        Serial.println(i[2]);
        Serial.print("Humidity= ");
        Serial.println(i[0]);
      }
    }
    z=0;
    i[0]=i[1]=i[2]=i[3]=i[4]=0;
  }
}
