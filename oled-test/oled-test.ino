#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    // address 0x3c for oled display
    Serial.println(F("SSD1306 allocation failed..."));
    for(;;);
  }

  // show initial buffer contents
  display.display();
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
