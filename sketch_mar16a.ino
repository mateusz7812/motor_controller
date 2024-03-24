#include <AccelStepper.h>
#include <MagicPot.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Toggle.h>
#include <U8g2lib.h>


#define DEBUG_BAUDIOS 9600
#define POTENTIOMETER_PIN A0
#define POTENTIOMETER_MIN_READ 0.0
#define POTENTIOMETER_MAX_READ 1000.0

#define MIN_SPEED 0.5
#define MAX_SPEED 11
#define SPEED_CONST 14
#define FAST_SPEED 100

#define i2c_Address 0x3c 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1   

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

constexpr int START_BUTTON_PIN = 12;
constexpr int STOP_BUTTON_PIN = 11;
constexpr int LEFT_SWITCH_PIN = 10;
constexpr int RIGHT_SWITCH_PIN = 9;

const int stepPin = 3;
const int directionPin = 2;
const int enablePin = 4;

float speed = 23;
float direction = 1;

bool stepperRun = false;
bool fastRunForward = false;
bool fastRunBackward = false;

const int ledPin = 13;    // the number of the LED pin

static const unsigned long REFRESH_INTERVAL = 100;

static unsigned long lastRefreshTime = 0;

AccelStepper stepper(AccelStepper::DRIVER, stepPin, directionPin);

MagicPot potentiometer(POTENTIOMETER_PIN, POTENTIOMETER_MIN_READ, POTENTIOMETER_MAX_READ);

/*void eventSwitchLeftCallback(SmartButton *button, SmartButton::Event event, int clickCounter)
{
    if (event == SmartButton::Event::PRESSED) {       
      direction = -1;
    } else if (event == SmartButton::Event::RELEASED) {
      direction = 0; 
    }
}

void eventSwitchRightCallback(SmartButton *button, SmartButton::Event event, int clickCounter)
{
    if (event == SmartButton::Event::PRESSED) {       
      direction = 1;
    } else if (event == SmartButton::Event::RELEASED) {
      direction = 0;
    }
}
*/

//Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Toggle startButton(START_BUTTON_PIN);
Toggle stopButton(STOP_BUTTON_PIN);
Toggle directionSwitch(LEFT_SWITCH_PIN, RIGHT_SWITCH_PIN);

void draw() {
  u8g2.clearBuffer();				
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font

  char resultS[8];
  u8g2.drawStr(0,10,"S:");	// write something to the internal memory
  dtostrf(speed, 6, 2, resultS);
  u8g2.drawStr(50,10,resultS);	// write something to the internal memory

  char resultP[8];
  u8g2.drawStr(0,20,"P:");	// write something to the internal memory
  dtostrf(speed * 0.3, 6, 2, resultP);
  u8g2.drawStr(50,20,resultP);	// write something to the internal memory

  char resultD[8];
  u8g2.drawStr(0,30,"D:");	// write something to the internal memory
  dtostrf(direction, 6, 2, resultD);
  u8g2.drawStr(50,30,resultD);	// write something to the internal memory

  char resultSCP[8];
  u8g2.drawStr(0,40,"SCP:");	// write something to the internal memory
  dtostrf(stepper.currentPosition(), 6, 2, resultSCP);
  u8g2.drawStr(50,40,resultSCP);	// write something to the internal memory

  u8g2.sendBuffer();		

  /*display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.print("S: "); display.println(speed);
  display.print("P: "); display.print(speed * 0.3); display.println("mm");
  display.print("D: "); display.println(direction);
  display.print("SCP: "); display.println(stepper.currentPosition());
  display.display();
  display.clearDisplay();*/
}

void updateStepper(){
  if (fastRunForward){
      stepper.setSpeed(SPEED_CONST * FAST_SPEED * direction);
      stepper.runSpeed();
  } else if (fastRunBackward) {
      stepper.setSpeed(SPEED_CONST * FAST_SPEED * direction * (-1.0));
      stepper.runSpeed();
  } else if (stepperRun){
    stepper.setSpeed(SPEED_CONST * speed * direction);
    stepper.runSpeed();
  } else {
    stepper.stop();
  }
}

void calcSpeed(){
  speed = (((MAX_SPEED - MIN_SPEED) * potentiometer.getValue()) / POTENTIOMETER_MAX_READ) + MIN_SPEED;
}

void updateServices(){
  startButton.poll();
  stopButton.poll();
  directionSwitch.poll();
  
	potentiometer.read();
}

void readButtons(){
  if (startButton.pressedFor(1000)){
    fastRunForward = true; 
  } else if (startButton.isPressed()){
    stepperRun = true;
  } else if (startButton.isReleased()){
    fastRunForward = false;
  }

  if (stopButton.pressedFor(1000)){
    fastRunBackward = true; 
  } else if (stopButton.isPressed()){
    stepperRun = false;
  } else if (stopButton.isReleased()){
    fastRunBackward = false;
  }
  
  if (directionSwitch.isMID()) {
    direction = 0;
  } else if (directionSwitch.isUP()) {
    direction = 1;
  } else if (directionSwitch.isDN()) {
    direction = -1;
  }
}

void sendOutput(){
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL)
	{
    draw();
		lastRefreshTime += REFRESH_INTERVAL;
	  //Serial.println(String("S: ") + speed + String(", D: ") + direction + String(", FRF: ") + fastRunForward + String(", FRB: ") + fastRunBackward + String(", SR: ") + stepperRun + String(", SP: ") + stepper.currentPosition() + String(", SS: ") + stepper.speed() + String(", MS: ") + stepper.maxSpeed());
	}
}

void setup()
{
	Serial.begin(DEBUG_BAUDIOS);
  /*delay(250);
  display.begin(i2c_Address, true);
  display.display();
  delay(2000);
  display.clearDisplay();
*/

  u8g2.begin();

	potentiometer.begin();

  stepper.setPinsInverted(false, false, true);
  stepper.setEnablePin(enablePin);
  stepper.enableOutputs();
  stepper.setMaxSpeed(60000);
  
  //pinMode(ledPin, OUTPUT);

  startButton.begin(START_BUTTON_PIN);
  stopButton.begin(STOP_BUTTON_PIN);
  directionSwitch.begin(LEFT_SWITCH_PIN, RIGHT_SWITCH_PIN);
}

void loop()
{
  updateServices();
  calcSpeed();
  updateStepper();
  sendOutput();
}