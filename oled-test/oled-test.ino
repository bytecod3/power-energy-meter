#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
char* msg = "scanning..."; // 25 chars is the maximum size msg can 

void showText(char* message){
  // set default font for screen
  display.setCursor(0,10);
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.println(message);
  display.display();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    // address 0x3c for oled display
    Serial.println(F("SSD1306 allocation failed..."));
    for(;;);
  }

  // set default font for screen
  display.setFont(&FreeMono9pt7b);
}

void loop() {
  
  for(int i=0; i<10; i++){
    display.setCursor(0,10);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.println(i);

    display.fillRect(10,10,50,30, WHITE);
    
    delay(500);
    display.display();

  }

  
  
//  showText(msg);

}
