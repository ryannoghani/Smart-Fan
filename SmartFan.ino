#include <Arduino.h>
#include <dht.h>
#include <LiquidCrystal.h>
#include "Timer.h"

//Temperature humidity sensor variables
dht DHT;
double temperature;
double humidity;
int TIME_ELAPSED_RECORD_TEMP_HUMIDITY;
//Motor variables
const int clockwiseInput = 3;  //Input that causes the motor to turn clockwise
const int counterclockwiseInput = 4;  //Input that causes the motor to turn clockwise
int fan_velocity;
//Joystick variables
double x_joystick_pos;
unsigned char buttonPressed;
//Mode SM variables
unsigned char mode;
//LCD variables
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
String temperature_display;
String humidity_display;


void Tick_Temp_Humidity() {
  humidity = DHT.humidity;
  temperature = (DHT.temperature * 9)/5 + 32;  //Converts the celcius temperature recorded by the DHT to farenheit
}


enum MODE_STATES {MANUAL, AUTOMATIC_WAIT_RELEASE, AUTOMATIC, MANUAL_WAIT_RELEASE} gStateMode = MANUAL;

void Tick_Mode() {  //Determines whether the fan should react to joystick input or temperature input.
  
  switch(gStateMode) {
    case MANUAL:
      if (buttonPressed) {
        gStateMode = AUTOMATIC_WAIT_RELEASE;
      }
      else {
        gStateMode = MANUAL;
      }
      break;

    case AUTOMATIC_WAIT_RELEASE:
      if (buttonPressed) {
        gStateMode = AUTOMATIC_WAIT_RELEASE;
      }
      else {
        gStateMode = AUTOMATIC;
      } 
      break;

    case AUTOMATIC:
      if (buttonPressed) {
        gStateMode = MANUAL_WAIT_RELEASE;
      }
      else {
        gStateMode = AUTOMATIC;
      }
      break;

    case MANUAL_WAIT_RELEASE:
      if (buttonPressed) {
        gStateMode = MANUAL_WAIT_RELEASE;
      }
      else {
        gStateMode = MANUAL;
      }
      break;
  }

  switch(gStateMode) {
    case MANUAL:
      mode = 0;
      break;
    case AUTOMATIC_WAIT_RELEASE:
      mode = 1;
      break;
    case AUTOMATIC:
      mode = 1;
      break;
    case MANUAL_WAIT_RELEASE:
      mode = 0;
      break;
  }
}


enum MANUAL_STATES {INIT, IDLE, RAISE_FAN_VELOCITY, LOWER_FAN_VELOCITY} gStateManual = INIT;

void Tick_Manual_Update_Fan_Speed() { //Finds what the fan velocity when taking input from the joystick

  switch(gStateManual) {
    case INIT:
      fan_velocity = 0;
      gStateManual = IDLE;
      break;
    case IDLE:
      if (x_joystick_pos > 200) {
        gStateManual = RAISE_FAN_VELOCITY;
      }
      else if (x_joystick_pos < -200) {
        gStateManual = LOWER_FAN_VELOCITY;
      }
      else {
        gStateManual = IDLE;
      }
      break;
    case RAISE_FAN_VELOCITY:
      if (x_joystick_pos > 200) {
        gStateManual = RAISE_FAN_VELOCITY;
      }
      else if (x_joystick_pos < -200) {
        gStateManual = LOWER_FAN_VELOCITY;
      }
      else {
        gStateManual = IDLE;
      }
      break;
    case LOWER_FAN_VELOCITY:
      if (x_joystick_pos > 200) {
        gStateManual = RAISE_FAN_VELOCITY;
      }
      else if (x_joystick_pos < -200) {
        gStateManual = LOWER_FAN_VELOCITY;
      }
      else {
        gStateManual = IDLE;
      }
      break;
  }

  switch(gStateManual) {
    case IDLE:
      break;
    case RAISE_FAN_VELOCITY:
      if (fan_velocity <= 240) {
        fan_velocity += 10;
      }
      break;
    case LOWER_FAN_VELOCITY:
      if (fan_velocity >= -240) {
        fan_velocity -= 10;
      }
      break;
  }
}


void Tick_Automatic_Update_Fan_Speed() {  //Finds what the fan velocity when taking input from the temperature

  if (fan_velocity >= 0) {
    fan_velocity = temperature + 40;  //I add 40 because the fan starts to move when the velocity is 120 and I want the fan to start moving when temperature is 75
  }
  else if (fan_velocity < 0) {
    fan_velocity = -1 * temperature - 40;
  }
}



void setup() {
    pinMode(clockwiseInput,OUTPUT); 
    pinMode(counterclockwiseInput,OUTPUT);

    pinMode(2, INPUT_PULLUP);

     pinMode(LED_BUILTIN, OUTPUT);
     lcd.begin(16, 2);
     lcd.clear();

    Serial.begin(9600);
    TIME_ELAPSED_RECORD_TEMP_HUMIDITY = 5000;
    TimerSet(200);
    TimerOn();
}



void loop() {

    DHT.read11(5);

    x_joystick_pos = analogRead(A0) -540;
    buttonPressed = !digitalRead(2);

    if (TIME_ELAPSED_RECORD_TEMP_HUMIDITY >= 5000) {
      humidity = DHT.humidity;
      temperature = (DHT.temperature * 9)/5 + 32;  //Converts the celcius temperature recorded by the DHT to farenheit
      TIME_ELAPSED_RECORD_TEMP_HUMIDITY = 0;
    }

    TIME_ELAPSED_RECORD_TEMP_HUMIDITY += 200;

    Tick_Mode();


    if (mode == 0) {  //Mode=0=manual     //Mode=1=automatic
      Tick_Manual_Update_Fan_Speed();
    }

    else {
      Tick_Automatic_Update_Fan_Speed();
    }


    if (fan_velocity >= 0) {
      analogWrite(clockwiseInput, fan_velocity);  //set the speed of motor
      analogWrite(counterclockwiseInput, 0);  //stop the counterclockwiseInput  pin of motor
    }

    else {
      analogWrite(clockwiseInput,0);  //stop the clockwiseInput pin of motor
      analogWrite(counterclockwiseInput, -1 * fan_velocity);  //set the speed of motor
    }

    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" ");
    lcd.print(char(223));
    lcd.print("F");

    lcd.setCursor(0, 2);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");

    while(!TimerFlag){}
    TimerFlag = 0;
}