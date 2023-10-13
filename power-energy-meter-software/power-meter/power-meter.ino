#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

#define current_sensor_pin 36
#define mv_per_amp 100  /* sensitivity for the 20A current sensor version */
#define sample_time 1000 /* sample the current for 1 second */
#define MAX_ADC 1023 /* max ADC value for Arduino - 8 bit ADC value */
#define MAX_ADC_VOLTAGE 5
#define MAX_ADC_ESP 4095  /* max ADC value for ESP32 - 12 bit ADC */
#define MAX_ADC_VOLTAGE_ESP 3.3
#define ESP_DIVIDER 1.65

#define ESP 1 /* set to 0 when testing with arduino board */

/* wifi credentials */
#define WIFI_RETRY_TIME 50
const char* ssid = "Gakibia-Unit3";
const char* password = "password";

uint8_t power_watt = 0;
double pp_voltage = 0; /* peak to peak voltage */
double v_rms = 0;
double a_rms = 0;
double voltage = 240.00;

/* Oled screen variables */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET (-1)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float calculate_peak_voltage(){
  float converted_voltage;
  uint8_t raw_voltage_value;
  uint8_t max_voltage_value = 0;
  uint8_t min_voltage_value = 4095; /* use 1024 for 8-bit ADCs */

  uint32_t start_time = millis();

  /* sample for 1 second */
  while( (millis() - start_time) < sample_time ){
    raw_voltage_value = analogRead(current_sensor_pin);

    /* handle noise. if we read below a given value, we consider it noise */
    if(raw_voltage_value < 0.16){
      raw_voltage_value = 0;
    }

    /* keep track of max and min values */
    if(raw_voltage_value > max_voltage_value){
      max_voltage_value = raw_voltage_value;
    }

    if(raw_voltage_value < min_voltage_value){
      min_voltage_value = raw_voltage_value;
    }
  }

  /* calculate peak to peak voltage */
  #if ESP
    converted_voltage = (max_voltage_value - min_voltage_value) * (MAX_ADC_VOLTAGE_ESP / MAX_ADC_ESP);
  #elif
    converted_voltage = (max_voltage_value - min_voltage_value) * (MAX_ADC_VOLTAGE / MAX_ADC);
  #endif

  return converted_voltage;
}

/* initialize oled screen */
void initialize_oled(){
  if(!display.begin(SSD1306_SWITCb HCAPVCC, SCREEN_ADDRESS)){
    Serial.println("[-]ERR: Display allocation failed!");
    for(;;);
  }
  
  // allocation succeeded
  display.clearDisplay();
}

/* draw text on the screen */
void update_screen(double message){
  display.clearDisplay();
  display.setFont(&FreeMono9pt7b);
  display.setCursor(0,0);
  display.println(message);
}

/* connect to WIFI */
void connect_to_wifi(){
  /* try connecting */
  Serial.println("[+]Scanning for network...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    /* try reconnecting */
    delay(WIFI_RETRY_TIME);
    Serial.println("[+]Scanning for network...");
  }

  Serial.println("[+]Network found");Serial.print("[+]IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {

  /* initialize serial monitor */
  Serial.begin(9600);

  /* initialise GSM */

  /* initialize OLED screen */
  initialize_oled();

  /* intitialize WIFI */
  connect_to_wifi();

  /* set up GPIO pins */
  pinMode(current_sensor_pin, INPUT);
  
}

void loop() {
  Serial.println("");
  pp_voltage = calculate_peak_voltage();
  v_rms = (pp_voltage / ESP_DIVIDER ) * 0.707;  /* divide by 2.5 for 5V ADC */
  a_rms = (v_rms * 1000) / mv_per_amp;

  Serial.print(a_rms);
  Serial.println("A RMS");

  power_watt = (a_rms * voltage);
  Serial.print(power_watt);
  Serial.println(" W");

  /* update readings on screen */
  display.println(a_rms);
  
}
