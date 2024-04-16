#include <U8g2lib.h>

#define SERIAL_BAUDIOS 9600

float speed = 5;

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


void setup()
{
    Serial.begin(SERIAL_BAUDIOS);
    u8x8.begin();
}


void loop()
{
    readSpeed();
    draw();
}