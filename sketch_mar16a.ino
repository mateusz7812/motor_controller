#include <AccelStepper.h>

// defines pins numbers
const int stepPin = 3;
const int directionPin = 2;
const int enablePin = 4;

// Define a stepper and the pins it will use
// 1 or AccelStepper::DRIVER means a stepper driver (with Step and Direction pins)
AccelStepper stepper(AccelStepper::DRIVER, stepPin, directionPin);

void setup()
{
  //Serial.begin(9600);
  stepper.setPinsInverted(false, false, true);
  stepper.setEnablePin(enablePin);
  stepper.enableOutputs();
  stepper.setMaxSpeed(60000);
  stepper.setSpeed(14 * 20 * 16);
}

void loop()
{
  stepper.runSpeed();
}