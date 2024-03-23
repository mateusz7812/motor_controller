#include <AccelStepper.h>
#include <MagicPot.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Toggle.h>

#define DEBUG_BAUDIOS 9600
#define POTENTIOMETER_PIN A0
#define POTENTIOMETER_MIN_READ 0.0
#define POTENTIOMETER_MAX_READ 1000.0

#define MIN_SPEED 0.5
#define MAX_SPEED 11
#define SPEED_CONST 14
#define FAST_SPEED 100


#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO


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

// defines pins numbers
const int stepPin = 3;
const int directionPin = 2;
const int enablePin = 4;

float speed = 23;
float direction = 0;

bool stepperRun = false;
bool fastRunForward = false;
bool fastRunBackward = false;

static const unsigned long REFRESH_INTERVAL = 100;

static unsigned long lastRefreshTime = 0;

//using namespace smartbutton;

// Define a stepper and the pins it will use
// 1 or AccelStepper::DRIVER means a stepper driver (with Step and Direction pins)
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
void onPressedForDuration()
{
  fastRunForward = true;  
}
void onPressed()
{
  stepperRun = true;
}

void eventStartCallback(SmartButton *button, SmartButton::Event event, int clickCounter)
{
    if (event == SmartButton::Event::CLICK) {       
      stepperRun = true;
    } else if (event == SmartButton::Event::HOLD) {     
      fastRunForward = true;  
    } else if (event == SmartButton::Event::RELEASED) {
      fastRunForward = false;
    }
}

void eventStopCallback(SmartButton *button, SmartButton::Event event, int clickCounter)
{
    if (event == SmartButton::Event::CLICK) {       
      stepperRun = false;
    } else if (event == SmartButton::Event::HOLD) {    
      fastRunBackward = true;     
    } else if (event == SmartButton::Event::RELEASED) {  
      fastRunBackward = false;  
    }
}
*/
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void draw() {
  
  // text display tests
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("S: "); display.println(speed);
  display.print("P: "); display.print(speed * 0.3); display.println("mm");
  display.print("D: "); display.println(direction);
  display.print("FRF: "); display.println(fastRunForward);
  display.print("SR: "); display.println(stepperRun);
  display.display();
  //delay(2000);
  display.clearDisplay();
}

Toggle startButton(START_BUTTON_PIN);

//EasyButton startButton(START_BUTTON_PIN);
/*SmartButton startButton(START_BUTTON_PIN, SmartButton::InputType::NORMAL_HIGH);
SmartButton stopButton(STOP_BUTTON_PIN, SmartButton::InputType::NORMAL_HIGH);

SmartButton leftSwitch(LEFT_SWITCH_PIN, SmartButton::InputType::NORMAL_HIGH);
SmartButton rightSwitch(RIGHT_SWITCH_PIN, SmartButton::InputType::NORMAL_HIGH);
*/
void setup()
{
	//Serial.begin(DEBUG_BAUDIOS);
  Serial.begin(9600);

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
 //display.setContrast (0); // dim display
 
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  /*pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(RIGHT_SWITCH_PIN, INPUT_PULLUP);

  startButton.begin(eventStartCallback);
  stopButton.begin(eventStopCallback);
  leftSwitch.begin(eventSwitchLeftCallback);
  rightSwitch.begin(eventSwitchRightCallback);
*/
  //startButton.begin();
  // Add the callback function to be called when the button is pressed for at least the given time.
  //startButton.onPressedFor(1000, onPressedForDuration);
  //startButton.onPressed(onPressed);

	potentiometer.begin();

  stepper.setPinsInverted(false, false, true);
  stepper.setEnablePin(enablePin);
  stepper.enableOutputs();
  stepper.setMaxSpeed(60000);
  
  
  startButton.begin(START_BUTTON_PIN);
}

void loop()
{
  startButton.poll();

  if(startButton.pressedFor(1000)){
    fastRunForward = true; 
  }
  
  if(startButton.isPressed()){
    stepperRun = true;
  }

  //startButton.read();
  //SmartButton::service();  
	potentiometer.read();

  speed = (((MAX_SPEED - MIN_SPEED) * potentiometer.getValue()) / POTENTIOMETER_MAX_READ) + MIN_SPEED;

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
  
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL)
	{
    draw();
		lastRefreshTime += REFRESH_INTERVAL;
	  //Serial.println(String("S: ") + speed + String(", D: ") + direction + String(", FRF: ") + fastRunForward + String(", FRB: ") + fastRunBackward + String(", SR: ") + stepperRun + String(", SP: ") + stepper.currentPosition() + String(", SS: ") + stepper.speed() + String(", MS: ") + stepper.maxSpeed());
	}
}