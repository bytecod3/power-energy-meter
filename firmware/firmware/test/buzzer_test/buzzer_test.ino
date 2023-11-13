//#include <U8g2lib.h>
//
//#define SCREEN_HEIGHT 32
//#define SCREEN_WIDTH 128
//#define YELLOW_OFFSET 16
//#define OLED_SDA 21
//#define OLED_SCL 22
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(/*R2: rotation 180*/U8G2_R0, /*reset*/U8X8_PIN_NONE, /* clock */ OLED_SCL, /* data */ OLED_SDA);

void setup() {
  // put your setup code here, to run once:
  pinMode(A0, OUTPUT);
  //pinMode(5, OUTPUT);

  //display.begin();

  alert();
}

void loop() {
  // put your main code here, to run repeatedly:
//  display.firstPage();
//  do {
//    display.setFont(u8g2_font_helvR14_tr);
//    display.drawStr(0,16,"Power: ");
//    display.drawStr(0,50, "2 kWh");
//
//  } while ( display.nextPage() );
  
  alert();
  //relay_off();

}

void alert(){
  digitalWrite(A0, HIGH);
  delay(400);
  digitalWrite(A0, LOW);
  delay(200);
}

void relay_off(){
  digitalWrite(5, HIGH);
  delay(1000);
  digitalWrite(5, LOW);
  delay(1000);
}
