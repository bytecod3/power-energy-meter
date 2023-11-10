#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include "defines.h"

/*=================CURRENT SENSOR VARIABLES===============================*/
const int average_value = 500; // take avg of 500 samples
long int sensor_value = 0;
float sensitivity = 1000.0/200.0; // 1000mA per 200mV
float v_ref = 1508;
float power_watt = 0;
float v_rms = 230.0; // default RMS voltage 

/*===================MQTT VARIABLES========================================*/
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

/*==========================wifi credentials========================*/
#define WIFI_RETRY_TIME 50
const char* ssid = "Boen";
const char* password = "12345678";

/*======================TIMING VARiABLES==================================*/
unsigned long ACS_current_sample_time = 0, ACS_previous_sample_time = 0;
unsigned long ACS_sample_interval;
#define SENSOR_POLL_TIME 2000

/*============================GSM VARIABLES=============================*/
#define RXD2 16
#define TXD2 17

/*===========================TOKEN CONVERSION VARIABLES===================*/
String msg;
String PHONE = "";
int unit_value = 1000; // 1 token is 1kWhg
String meter_id = "";
String token;
String units;
String amount;
/*=============================BUZZER VARIABLES===========================*/
#define BUZZ_TIME 150

/*===============================OLED SCREEN VARIABLES ==================*/
/**
 * Configure the screen
 */
#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 128
#define YELLOW_OFFSET 16
#define OLED_SDA 21
#define OLED_SCL 22
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(/*R2: rotation 180*/U8G2_R0, /*reset*/U8X8_PIN_NONE, /* clock */ OLED_SCL, /* data */ OLED_SDA);

// function prototypes
float readCurrent(); 
void mqtt_init();
bool mqtt_reconnect();
void mqtt_publish();
void wifi_connect();
void gsm_receive();
void gsm_action();
void gsm_parse(String);
void alert();

// function definitions
float readCurrent() {

  ACS_current_sample_time = millis(); // get current time
  float current;

  if(ACS_current_sample_time - ACS_previous_sample_time > ACS_sample_interval){
    // take average of 500 readings every 1 second

    for(int i = 0; i < average_value; i++){
      sensor_value += analogRead(CURRENT_SENSOR_PIN);

      // wait for 2 ms before the next loop
      delay(2);
    }

    // find the average of the values 
    sensor_value = sensor_value / average_value;

    // the on-board ADC is 12 bits
    // for esp32, 2^12 = 4096, so 3.3V / 4095 ~= 0.8mV
    // unit value = 3.3 / 4096 * 1000 = 0.8 

    float unit_value = (REFERENCE_VOLTAGE / 4096) * 1000; // resolution
    float voltage = unit_value * sensor_value;

    // when no load, v_ref = initial_value
    debug("initial value: ");
    debug(voltage);
    debug(" mV");

    // calculate the corresponding current
    current = (voltage - v_ref) * sensitivity;

    // print display voltage(mV) voltage corresponding to the current
    voltage = unit_value * sensor_value - v_ref;
    debug(voltage);
    debugln("mV");

    // print current (mA)
    debug(current);
    debugln(" mA");

    // reset the sensor_value for the next reading 
    sensor_value = 0;


    // update the timing for the next loop
    ACS_previous_sample_time = ACS_current_sample_time;

    
  }

  return current;

}

/**
 * Initialize MQTT
 * 
 */
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

/**
 * Try reconnecting to MQTT
 * 
 */
bool mqtt_reconnect(){
  if(client.connect(mqtt_client_name)){
    debugln("Reconnecting to MQTT broker...");
  } else {
    debug("Failed...");
    debugln(client.state());
  }

  return client.connected();
}

/**
 * Publish data to broker
 * 
 */
void mqtt_publish(){
  float a_rms = readCurrent(); // read the current from the current sensor

  // create MQTT message
  snprintf(mqtt_msg, sizeof(mqtt_msg), "%.2f, %.2f", a_rms); // todo: check for correct length

  if(client.publish(topic, mqtt_msg)){
    debugln("[+]Data published");
  } else {
    debugln("[-]Failed to publish");
  }

}

/**
 * Connect to WIFI
 * 
 */
void wifi_connect(){
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

/**
 * Send whatever is in Serial2 to Serial0(programming serial) for debugging and vice versa
 * 
 */
void gsm_update_serial(){
  delay(500);
  while(Serial.available()){
    Serial2.write(Serial.read()); // forward what programming serial receives to Serial2
  }

  while(Serial2.available()){
    Serial.write(Serial2.read()); // forward what serial2 recieves to programming serial for debug
  }

}

/**
 * Init GSM module
 * 
 */
void gsm_init(){
  
  // perform handshake
  Serial2.println("AT"); // will return OK if handshake sucessful
  // gsm_update_serial();
  gsm_receive();

  // configure text mode
  Serial2.println("AT+CMGF=1"); 
  // gsm_update_serial();
  gsm_receive();

  // define how newly received SMS should be handled
  Serial2.println("AT+CNMI=1,2,0,0,0");
  gsm_receive();

}


/**
 * Set susbstring
 * 
 */
void get_substring(char string[], char substring[], int position, int length)
{
  int c = 0;

  while (c < length)
  {
    substring[c] = string[position + c - 1];
    c++;
  }

  substring[c] = '\0';  

}

/**
 * Receive and decode GSM message
 * 
 */
void gsm_receive(){
  while(Serial2.available()){
    //Serial.write(Serial2.read()); // forward what serial2 receives to programming serial for debug
    gsm_parse(Serial2.readString()); // parse the sms
  }

  // gsm_action();

  while(Serial.available()){
    Serial2.println(Serial.readString()); // forward what programming serial receives to Serial2
  }

}

void gsm_parse(String buff){
  // Serial.println(buff);
  unsigned int index;
  unsigned int index_of_a;

  // remove "AT command" from the response string
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();

  if(buff !="OK"){
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);

    buff.remove(0, index + 2);

    // parse necessary message from SIM800L Serial buffer string
    if(cmd == "+CMT"){
      // get newly arrived memory location and store it in temp
      index = buff.lastIndexOf(0x0D); // looking for position of the carriage return

      // extract the message
      msg = buff.substring(index + 2, buff.length()); // writes message variable to msg
      msg.toLowerCase(); // whole message gets converted to lower case

      // extract the amount paid
      // amount = msg.substring(54, msg.length());
      // amount = msg.substring(0, 10);

      // extract the number of units
      // index_of_a = msg.indexOf('a'); // last comma is where the string "amt" starts
      // units = msg.substring(47, index_of_a);

      //index = buff.indexOf(0X22); // looking for position of the first double quotes

      // extract the phone number
      //PHONE = buff.substring(index + 1, index + 14);

      // debug extracted variables
      // Serial.println(PHONE);
      Serial.println(msg);
      // Serial.println(amount);
      // Serial.println(units);
    }
  }

}

/**
 * Act depending on the GSM message
 * 
 */
void gsm_action(){
  if(msg == "buzz"){
    alert();
  }

  // reset variables
  PHONE = ""; // clear the pone string
  msg = ""; // clear the message string
  amount = "";
}

/**
 * calculate units
 * 
 */
float calculate_units(){
  float units = 0; // received units from SERVER
  return units;
}

/**
 * @brief 
 * 
 */
void receive_from_http(){

}


/**
 * Buzz to alert of depleted tokens
 * 
 */
void alert(){
  digitalWrite(BUZZER_PIN, HIGH);
  delay(BUZZ_TIME);

  digitalWrite(BUZZER_PIN, LOW);
  delay(BUZZ_TIME);
}

/**
 * Turn off the relay
 * 
 */
void relay_turn_off(){
  digitalWrite(RELAY, LOW);
}

/**
 * Turn on the relay
 * 
 */
void relay_turn_on(){
  digitalWrite(RELAY, HIGH);
}

/**
 * Initialize the OLED
 * 
 */
void oled_init(){
  // configure screen
    display.setColorIndex(1);  // set the color to white
    display.drawStr(0,0, "Power Meter");
    display.begin();
}

/**
 * show oled default oled screen
 * show the instantaneous current, voltage and power 
 * 
 */
void oled_default_screen(){
  display.setFont(u8g2_font_10x20_mr);
  display.drawStr(0,0, "Power Meter");
  
}

void setup() {

  // pinmodes
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ALERT_LED, OUTPUT);
  
  // init serial 
  Serial.begin(115200);
  // init hardware serial format: Serial.begin(baud_rate, protocol, RX pin, TX pin)
  debugln("Initializing hardware serial 2...");
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(1000);
  debugln("Serial 2 ready!");
  // init GSM
  gsm_init();

  // int oled
  oled_init();

  // alert
  // alert();

}

void loop() {

  // read the current
  //float current_rms = readCurrent();

  //  if(!client.connected()){
  //   debugln("[-]Client not connected...");
  //   unsigned long now = millis();

  //   if(now - last_reconnect_attempt > MQTT_RETRY_TIME){
  //     last_reconnect_attempt = now;

  //     if(mqtt_reconnect()){
  //       debugln("[+]Reconnected...");
  //       last_reconnect_attempt = 0;
  //     }
      
  //   }
  //  } else {
  //   client.loop();
  //   unsigned long current_millis = millis();

  //   if(current_millis - previous_millis >= SENSOR_POLL_TIME){
  //     previous_millis = current_millis;

  //     mqtt_publish();
  //   }

  //  }

   // poll for SMS
  //  gsm_update_serial(); // todo: change this method

  gsm_receive();
  // gsm_decode_sms(received_message_buffer);

  // Serial.println(meter_id);

  // update screen
  oled_default_screen();

}