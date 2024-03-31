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
//#define MAX_SPEED 11
#define MAX_SPEED 200
#define SPEED_CONST 14
#define FAST_SPEED 100

#define GROUND_1_PIN 13
//#define GROUND_2_PIN 12
//#define GROUND_3_PIN 11
#define GROUND_4_PIN 10
#define GROUND_5_PIN 9
#define GROUND_6_PIN 8
#define GROUND_7_PIN A1
#define GROUND_8_PIN A2
#define GROUND_9_PIN A3
#define GROUND_10_PIN A6
#define GROUND_11_PIN A7

#define VOUT_1_PIN 12
#define VOUT_2_PIN 11

#define START_BUTTON_PIN 7
#define STOP_BUTTON_PIN 6
#define LIMIT_SWITCH_PIN 5
#define STEP_PIN 3
#define DIRECTION_PIN 2
#define ENABLE_PIN 4

#define OUTPUT_REFRESH_INTERVAL 100
#define SPEED_REFRESH_INTERVAL 100

static float lastSpeed = 5;
static float speed = 5;
static float direction = 0;

static bool stepperRun = false;
static bool fastRun = false;

static unsigned long lastOutputRefreshTime = 0;
static unsigned long lastSpeedRefreshTime = 0;
static unsigned long lastLoopRefreshTime = 0;

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

void calcSpeed(){
  if(millis() - lastSpeedRefreshTime >= SPEED_REFRESH_INTERVAL)
	{
    if (fastRun){
      speed = FAST_SPEED;
    } else if (stepperRun){
      potentiometer.read();
      speed = (((MAX_SPEED - MIN_SPEED) * potentiometer.getValue()) / POTENTIOMETER_MAX_READ) + MIN_SPEED;
      //speed = (speed + 4 * lastSpeed) / 5;
      speed = (speed + lastSpeed) / 2;
      speed = speed - fmod(speed, 0.05);
      lastSpeed = speed;
    } else {
      speed = 0;
    }
    stepper.setSpeed(SPEED_CONST * speed * direction);
		lastSpeedRefreshTime += SPEED_REFRESH_INTERVAL;
	}
}

void updateServices(){
  if (speed > 0){
    stepper.runSpeed();
  } else {
    stepper.stop();
  }
  limitSwitch.poll();
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
  if(millis() - lastOutputRefreshTime >= OUTPUT_REFRESH_INTERVAL)
	{
    draw();
		lastOutputRefreshTime += OUTPUT_REFRESH_INTERVAL;
	  //Serial.println(String("S: ") + speed + String(", D: ") + direction + String(", FRF: ") + fastRunForward + String(", FRB: ") + fastRunBackward + String(", SR: ") + stepperRun + String(", SP: ") + stepper.currentPosition() + String(", SS: ") + stepper.speed() + String(", MS: ") + stepper.maxSpeed());
	}
}

void setup()
{
	Serial.begin(DEBUG_BAUDIOS);

  pinMode(GROUND_1_PIN, OUTPUT);
 // pinMode(GROUND_2_PIN, OUTPUT);
 // pinMode(GROUND_3_PIN, OUTPUT);
  pinMode(GROUND_4_PIN, OUTPUT);
  pinMode(GROUND_5_PIN, OUTPUT);
  pinMode(GROUND_6_PIN, OUTPUT);
  pinMode(GROUND_7_PIN, OUTPUT);
  pinMode(GROUND_8_PIN, OUTPUT);
  pinMode(GROUND_9_PIN, OUTPUT);
  pinMode(GROUND_10_PIN, OUTPUT);
  pinMode(GROUND_11_PIN, OUTPUT);

  pinMode(VOUT_1_PIN, OUTPUT);
  pinMode(VOUT_2_PIN, OUTPUT);
  
  digitalWrite(GROUND_1_PIN, LOW);
 // digitalWrite(GROUND_2_PIN, LOW);
  //digitalWrite(GROUND_3_PIN, LOW);
  digitalWrite(GROUND_4_PIN, LOW);
  digitalWrite(GROUND_5_PIN, LOW);
  digitalWrite(GROUND_6_PIN, LOW);
  digitalWrite(GROUND_7_PIN, LOW);
  digitalWrite(GROUND_8_PIN, LOW);
  digitalWrite(GROUND_9_PIN, LOW);
  digitalWrite(GROUND_10_PIN, LOW);
  digitalWrite(GROUND_11_PIN, LOW);

  digitalWrite(VOUT_1_PIN, HIGH);
  digitalWrite(VOUT_2_PIN, HIGH);
  
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
  sendOutput();
}