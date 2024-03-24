#include <AccelStepper.h>
#include <MagicPot.h>
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
static bool fastRunForward = false;
static bool fastRunBackward = false;

static unsigned long lastRefreshTime = 0;

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIRECTION_PIN);
MagicPot potentiometer(POTENTIOMETER_PIN, POTENTIOMETER_MIN_READ, POTENTIOMETER_MAX_READ);
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

Toggle startButton(START_BUTTON_PIN);
Toggle stopButton(STOP_BUTTON_PIN);
Toggle directionSwitch(LEFT_SWITCH_PIN, RIGHT_SWITCH_PIN);
Toggle limitSwitch(LIMIT_SWITCH_PIN);

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
  speed = (speed + 4 * lastSpeed) / 5;
  speed = speed - fmod(speed, 0.05);
  lastSpeed = speed;
}

void updateServices(){
  startButton.poll();
  stopButton.poll();
  directionSwitch.poll();
  limitSwitch.poll();
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

  if (limitSwitch.isPressed()){
    stepperRun = false;
    fastRunForward = false;
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
  
  startButton.begin(START_BUTTON_PIN);
  stopButton.begin(STOP_BUTTON_PIN);
  directionSwitch.begin(LEFT_SWITCH_PIN, RIGHT_SWITCH_PIN);
  limitSwitch.begin(LIMIT_SWITCH_PIN);
}

void loop()
{
  updateServices();
  readButtons();
  calcSpeed();
  updateStepper();
  sendOutput();
}