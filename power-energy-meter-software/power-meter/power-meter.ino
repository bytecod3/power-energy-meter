#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
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
#define SENSOR_POLL_TIME 2000 // poll sensors every 2 seconds 

#define ESP 1 /* set to 0 when testing with arduino board */
#define DEBUG 1
#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x, y) Serial.printf(x, y)

#else 

#define debug(x) 
#define debugln(x)
#define debugf(x, y)

#endif

/* MQTT variables */
#define MESSAGE_LENGTH 100
#define MQTT_RETRY_TIME 100 // try reconnecting for 100ms
char mqtt_msg[MESSAGE_LENGTH];
unsigned long last_reconnect_attempt = 0;
unsigned long previous_millis = 0;
const char* topic = "power/meter";
String sensor_data = "";
const char* mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_client_name = "POWER_ESP";
WiFiClient esp_client;
PubSubClient client(esp_client);

/* wifi credentials */
#define WIFI_RETRY_TIME 50
const char* ssid = "Boen";
const char* password = "12345678";

double power_watt = 0;
double pp_voltage = 0; /* peak to peak voltage */
double v_rms = 0;
double a_rms = 0;
double voltage = 240.00;

/* Oled screen variables */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* MQTT conection parameters */
void initialize_mqtt(){
  client.setServer(mqtt_broker, mqtt_port);
  debugln("Connecting to broker..");

  while(!client.connected()){
    debug("...");
    client.connect(mqtt_client_name);
    delay(MQTT_RETRY_TIME);
  }

  // at this point the broker is connected
  debugln("Connected to broker!");
  
}

bool reconnect(){
  if(client.connect(mqtt_client_name)){
    debugln("Reconnecting to MQTT broker...");
  } else {
    debug("Failed...");
    debugln(client.state());
  }

  return client.connected();
}


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
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    debugln("[-]ERR: Display allocation failed!");
    for(;;);
  }
  
  // allocation succeeded - buffer initial contents
  debugln("display found");
}

/* draw text on the screen */
void update_screen(double message){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(message);
  display.display();
}

/* connect to WIFI */
void connect_to_wifi(){
  /* try connecting */
  debugln("[+]Scanning for network...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    /* try reconnecting */
    delay(WIFI_RETRY_TIME);
    debugln("[+]Scanning for network...");
  }

  debugln("[+]Network found");debug("[+]IP address: ");
  debugln(WiFi.localIP());
}

void read_and_publish_data(){
  pp_voltage = calculate_peak_voltage();
  v_rms = (pp_voltage / ESP_DIVIDER ) * 0.707;  /* divide by 2.5 for 5V ADC */
  a_rms = (v_rms * 1000) / mv_per_amp;

  debug(a_rms);
  debugln("A RMS");

  power_watt = (a_rms * voltage);
  debug(power_watt);
  debugln(" W");

  /* publish data to MQTT broker */
  // create MQTT message
  if(snprintf(mqtt_msg, sizeof(mqtt_msg), "%.2f, %.2f", a_rms, power_watt) > sizeof(mqtt_msg)){
    debugln("mqtt message size too long");
  }

  // publish the data
  if(client.publish(topic, mqtt_msg)){
    debugln("Data published");
  } else {
    debugln("Failed to publish");
  }
}

void setup() {

  /* initialize serial monitor */
  Serial.begin(115200);

  /* initialise GSM */

  /* initialize OLED screen */
  initialize_oled();
  display.setCursor(0,0);
  display.println("Hello there...");
  display.display();

  /* intitialize WIFI */
  connect_to_wifi();
  initialize_mqtt();

  /* set up GPIO pins */
  pinMode(current_sensor_pin, INPUT); 
}

void loop() {

  if(!client.connected()){
    debugln("Client not connected...");
    unsigned long now = millis();

    if(now - last_reconnect_attempt > MQTT_RETRY_TIME){
      last_reconnect_attempt = now;

      if(reconnect()){
        debugln("Reconnected...");
        last_reconnect_attempt = 0;
      }
      
    }
  } else {
    client.loop();
    unsigned long current_millis = millis();

    if(current_millis - previous_millis >= SENSOR_POLL_TIME){
      previous_millis = current_millis;

      read_and_publish_data();
    }
  }

  
}
