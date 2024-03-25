#include <AccelStepper.h>
#include <MagicPot.h>
#include <Toggle.h>
#include <U8g2lib.h>
#include <SmartButton.h>

#define DEBUG_BAUDIOS 9600
#define POTENTIOMETER_PIN A0
#define POTENTIOMETER_MIN_READ 0.0
#define POTENTIOMETER_MAX_READ 1000.0

#define MIN_SPEED 0.5
#define MAX_SPEED 11
#define SPEED_CONST 14
#define FAST_SPEED 100

#define START_BUTTON_PIN 12
#define STOP_BUTTON_PIN 11
#define LEFT_SWITCH_PIN 10
#define RIGHT_SWITCH_PIN 9
#define LIMIT_SWITCH_PIN 8

#define STEP_PIN 3
#define DIRECTION_PIN 2
#define ENABLE_PIN 4

#define REFRESH_INTERVAL 100

static float lastSpeed = 5;
static float speed = 5;
static float direction = 0;

static bool stepperRun = false;
static bool fastRun = false;

static unsigned long lastRefreshTime = 0;

using namespace smartbutton;

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIRECTION_PIN);
MagicPot potentiometer(POTENTIOMETER_PIN, POTENTIOMETER_MIN_READ, POTENTIOMETER_MAX_READ);
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

Toggle limitSwitch(LIMIT_SWITCH_PIN);
SmartButton startButton(START_BUTTON_PIN, SmartButton::InputType::NORMAL_HIGH);
SmartButton stopButton(STOP_BUTTON_PIN, SmartButton::InputType::NORMAL_HIGH);

void draw() {
  u8x8.setFont(u8x8_font_inr21_2x4_r);

  char resultS[8];
  dtostrf(speed, 5, 2, resultS);
  u8x8.drawString(0, 0, resultS);	
  u8x8.drawString(10, 0, "obr");	

  char resultP[8];
  dtostrf(speed * 0.3, 5, 2, resultP);
  u8x8.drawString(0, 12, resultP);	
  u8x8.drawString(12, 12, "mm");	
}

void updateStepper(){
  if (fastRun){
    stepper.setSpeed(SPEED_CONST * FAST_SPEED * direction);
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
  speed = (speed + 4 * lastSpeed) / 5;
  speed = speed - fmod(speed, 0.05);
  lastSpeed = speed;
}

void updateServices(){
  limitSwitch.poll();
	potentiometer.read();
  SmartButton::service();  
}

void startButtonCallback(SmartButton *button, SmartButton::Event event, int clickCounter)
{
  if (event == SmartButton::Event::CLICK) {  
    stepperRun = !stepperRun;
    direction = 1;
  } else if (event == SmartButton::Event::RELEASED) {   
    fastRun = false;
  } else if (event == SmartButton::Event::HOLD) {   
    direction = 1;
    fastRun = true; 
  }
}

void stopButtonCallback(SmartButton *button, SmartButton::Event event, int clickCounter)
{
  if (event == SmartButton::Event::CLICK) {  
    stepperRun = !stepperRun;
    direction = -1;
  } else if (event == SmartButton::Event::RELEASED) {   
    fastRun = false;
  } else if (event == SmartButton::Event::HOLD) {   
    direction = -1;
    fastRun = true; 
  }
}

void readButtons(){
  if (limitSwitch.isPressed()){
    stepperRun = false;
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

  u8x8.begin();
	potentiometer.begin();

  stepper.setPinsInverted(false, false, true);
  stepper.setEnablePin(ENABLE_PIN);
  stepper.enableOutputs();
  stepper.setMaxSpeed(60000);

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  limitSwitch.begin(LIMIT_SWITCH_PIN);

  startButton.begin(startButtonCallback);
  stopButton.begin(stopButtonCallback);
}

void loop()
{
  updateServices();
  readButtons();
  calcSpeed();
  updateStepper();
  sendOutput();
}