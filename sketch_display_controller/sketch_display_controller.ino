#include <U8g2lib.h>

#define SERIAL_BAUDIOS 9600

#define BLINK_INTERVAL 1000

#define GROUND_1_PIN A1
#define GROUND_2_PIN A2
#define GROUND_3_PIN A3

#define VOUT_1_PIN 12
#define VOUT_2_PIN 11
#define VOUT_3_PIN 10

float speed = 5;

static bool blinkState = false;

static unsigned long lastBlinkTime = 0;

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

void readSpeed() {
    char resultS[32];
    Serial.readBytes(resultS, 32);
    float temp = atof(resultS); 
    if(temp != 0){
        speed = temp; 
    }
}

void draw() {
    u8x8.setFont(u8x8_font_inr21_2x4_r);

    char resultS[32];
    dtostrf(speed, 5, 2, resultS);
    u8x8.drawString(0, 0, resultS);	
    u8x8.drawString(10, 0, "obr");	

    char resultP[32];
    dtostrf(speed * 1.67, 5, 2, resultP);
    u8x8.drawString(0, 12, resultP);	
    u8x8.drawString(12, 12, "mm");
}

void blink() {
    if(millis() - lastBlinkTime >= BLINK_INTERVAL){
        if(blinkState){
            digitalWrite(LED_BUILTIN, HIGH);
        } else {
            digitalWrite(LED_BUILTIN, LOW);
        }
        blinkState = !blinkState;
        lastBlinkTime = millis();
    }
}

void setup()
{
    Serial.begin(SERIAL_BAUDIOS);

    pinMode(GROUND_1_PIN, OUTPUT);
    pinMode(GROUND_2_PIN, OUTPUT);
    pinMode(GROUND_3_PIN, OUTPUT);

    pinMode(VOUT_1_PIN, OUTPUT);
    pinMode(VOUT_2_PIN, OUTPUT);
    pinMode(VOUT_3_PIN, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);
    
    digitalWrite(GROUND_1_PIN, LOW);
    digitalWrite(GROUND_2_PIN, LOW);
    digitalWrite(GROUND_3_PIN, LOW);

    digitalWrite(VOUT_1_PIN, HIGH);
    digitalWrite(VOUT_2_PIN, HIGH);
    digitalWrite(VOUT_3_PIN, HIGH);

    u8x8.begin();
}


void loop()
{
    readSpeed();
    draw();
    blink();
}