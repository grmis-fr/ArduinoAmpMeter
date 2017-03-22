// ================================================
// Author: GRMIS on RCGROUPS
// GrMis@free.fr
// November 2017
// version 2: two buttons, 3 different display modes, energy (Joules) & mean power
// ================================================
// I2C OLED DISPLAY: SCL = A5  ,  SDA = A4
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
// ================================================
//Timing variables:
int now,previous_screen,previous_measure,prev_scr_change;

//Analog input connected to the current sensor
const int ACS755_pin=A0;
//Analog input connected to the voltage divider
const int volt_pin=A1;

//Buttons pins
const int Button1=2;
const int Button2=3;

float Offset;
float current;
float voltage;
float current_min=999;
float current_max=-999;
float voltage_min=999;
float voltage_max=-999;

float power;
float power_min=999;
float power_max=-999;

const float ACS=40;// mV/A
const float current_factor=5.0/(1024.0*ACS*0.001);
const float voltage_factor=20.0/1024.0;
const float Ah_factor=0.001/3600.0*1000.0;

float Ah=0;
float Energy=0;
long int time=0;

volatile int  screen_number=0;

// ================================================
void RefreshScreen() {

  display.clearDisplay();
  display.setCursor(0,0);

const int sn=screen_number;
if (sn==0) {
  display.setTextSize(1);
  display.print(current);display.println(" A");
  display.print("   min:");  display.print(current_min);
  display.println(" A");
  display.print("   max:");  display.print(current_max);
  display.println(" A");
  display.print("    ");
  if (Ah<10) {display.print(Ah,2);} else {display.print(Ah,0);}
  display.println(" mA.h");
  display.print(voltage);display.println(" V");
  display.print("   min:");display.print(voltage_min);display.println(" V");
  display.print("   max:");display.print(voltage_max);display.println(" V");
  display.print(power,1);display.print(" W  (");display.print(power_max,1);display.println(")");
}
if (sn==1) {
  display.setTextSize(1);
  display.print(current);display.println(" Amper");
  if (Ah<10) {display.print(Ah,2);} else {display.print(Ah,0);}
  display.println(" mA.h");
  display.print(voltage);display.println(" Volts");
  display.print(0.001*Energy);display.println(" Joules");
  long int dt=(millis()-time);
  display.print("Mean P:");display.print(Energy/dt);display.println(" W");
  display.print("R:");display.print(1000*(voltage_max-voltage_min)/current_max);display.println(" mOhm");
  display.print(dt/1000);display.println(" seconds");
}
  if (sn==2) {
display.setTextSize(2);
  display.print(current);display.println(" A");
display.print(voltage);display.println(" V");
display.print(power,1);display.print(" W");
  }
  display.display();
}
//===================================================
void reset() {
  current_min=999;
  current_max=-999;
      voltage_min=999;
      voltage_max=-999;
      power_min=999;
      power_max=-999;
      Ah=0;Energy=0;time=millis();
      delay(100);//de-bounce
}
//===================================================
void change_screen() {
  now=millis();
  if (now-prev_scr_change>250) {//de-bounce
  screen_number++;
    if (screen_number>2) screen_number=0;
  prev_scr_change=now;
  }
}
//===================================================
void setup() {

  //OLED Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("Hall");
  display.println("Effect");
  display.println("Power");
  display.println("Analyzer");
  display.display();
  display.setTextSize(1);

  now=millis();
  previous_measure=now;
  previous_screen=now;
  prev_scr_change=now;
  //Calibrate the Current sensor offset  
  int ofs=0;
  int n=0;
  for (int i=0;i<99;i++) {
    ofs+=analogRead(ACS755_pin);
    delay(50);
    n++;
  }
  Offset=float(ofs)*current_factor/n;

  display.setTextSize(1);  

  pinMode(Button1, INPUT_PULLUP);           // set pin to input
  pinMode(Button2, INPUT_PULLUP);           // set pin to input
  //digitalWrite(Button1, HIGH);       // turn on pullup resistors
  //digitalWrite(Button2, HIGH);       // turn on pullup resistors
  time=millis();
  
  attachInterrupt(0, reset, FALLING );// Interrupt attached to pin 2 (Button 1)
  attachInterrupt(1, change_screen, FALLING );// Interrupt attached to pin 3 (Button 2)
}
//===================================================
void loop() {
  now=millis();

  if(now-previous_measure>50) {
    const int i=analogRead(ACS755_pin);
    current=current_factor*i-Offset;
    Ah+=Ah_factor*current*(now-previous_measure);  
    
    int v=analogRead(volt_pin);
    voltage=voltage_factor*v; 
    power=current*voltage;
    Energy+=power*(now-previous_measure);
  previous_measure=now;


    if (voltage>voltage_max) voltage_max=voltage;  
    if (voltage<voltage_min) voltage_min=voltage;  
    if (current>current_max) current_max=current;  
    if (current<current_min) current_min=current; 
    if (power>power_max) power_max=power;  
    if (power<power_min) power_min=power; 

  }

  if(now-previous_screen>400) {
    previous_screen=now;
    RefreshScreen();
  }
}
