#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include "../lib/Filters/Filters.h"
#include "defines.h"

/*=================CURRENT SENSOR VARIABLES===============================*/
const int average_value = 500; // take avg of 500 samples
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
const char* ssid = "Gakibia-Unit3";
const char* password = "password";

/*======================TIMING VARIABLES==================================*/
unsigned long ACS_current_sample_time = 0, ACS_previous_sample_time = 0;
unsigned long ACS_sample_interval;
#define SENSOR_POLL_TIME 2000

/*============================GSM VARIABLES=============================*/
#define RXD2 16
#define TXD2 17

/*===========================TOKEN CONVERSION VARIABLES===================*/
String msg;
String PHONE = "";
int unit_value = 1000; // 1 token is 1 kWh
String meter_id = "";
String token;
String units;
int received_units; // units received from sms
String amount;
unsigned long load_turn_on_time = 0; // exact time the load is turned on
unsigned long current_time = 0;
char relay_on_off_flag = 0; // flag to tell whether the relay(load) is on or off
float consumed_units = 0;
float remaining_units = 0;
float elapsed_time_hrs;
unsigned long elapsed_time = 0;
float power_kw = 0;
float power_kwh = 0;

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
void gsm_action();
void gsm_parse(String);
void alert();
int read_serial(int, char*,int);

/*==========================CURRENT READING VARIABLES=========================*/

//#define ACS_Pin A0                        //Sensor data pin on A0 analog   input

float ACS_Value;                              //Here we keep the raw   data valuess
float testFrequency = 50;                    // test signal frequency   (Hz)
float windowLength = 40.0/testFrequency;     // how long to average the   signal, for statistist

float intercept = 0; // to be adjusted based   on calibration testing
float slope = 0.0752; // to be adjusted based on calibration   testing
                      //Please check the ACS712 Tutorial video by SurtrTech   to see how to get them because it depends on your sensor, or look below

float   Amps_TRMS; // estimated actual current in amps

unsigned long printPeriod   = 1000; // in milliseconds
// Track time in milliseconds since last reading 
unsigned   long previousMillis = 0;

// float read_curr(){
//   RunningStatistics inputStats;                 // create statistics   to look at the raw test signal
//   inputStats.setWindowSecs( windowLength );     //Set   the window length
     
//   while( true ) {   
//     ACS_Value = analogRead(CURRENT_SENSOR_PIN);   // read the analog in value:
//     inputStats.input(ACS_Value);  // log to Stats   function
        
//     if((unsigned long)(millis() - previousMillis) >= printPeriod)   { //every second we do the calculation
//       previousMillis = millis();   //   update time
      
//       Amps_TRMS = intercept + slope * inputStats.sigma();

//        Serial.print( "Amps: " ); 
//       Serial.print( Amps_TRMS );
//       Serial.println();

//      }
//   }
  
// }

/*================END OF CURRENT READING======================================*/

/**
 * Initialize MQTT
 * 
 */
void initialize_mqtt(){
  client.setServer(mqtt_broker, mqtt_port);
  debugln("[]Connecting to broker..");

  while(!client.connected()){
    debug("...");
    client.connect(mqtt_client_name);
    delay(MQTT_RETRY_TIME);
  }

  // at this point the broker is connected
  debugln("[+]Connected to broker!");
  
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
 * Connect to WIFI
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

void gsm_parse(String buff){
  // Serial.println(buff);

  unsigned int index;
  unsigned int index_of_a;

  // remove "AT command" from the response string
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();

  if(buff != "OK"){
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();

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
      index_of_a = msg.indexOf('a'); // last comma is where the string "amt" starts
      units = msg.substring(46, index_of_a);

      index = buff.indexOf(0X22); // looking for position of the first double quotes
      PHONE = buff.substring(index + 1, index + 14); // extract the phone number

      // debug extracted variables
      // Serial.println(PHONE);
      // Serial.println(units);

    }
  }

}


/**
 * Act depending on the GSM message
 * 
 */
void gsm_action(){
  // if (msg == "led on")
  // {
  //   digitalWrite(LED_PIN, HIGH);
  //   Reply("LED is ON");
  // }
  // else if (msg == "led off")
  // {
  //   digitalWrite(LED_PIN, LOW);
  //   Reply("LED is OFF");
  // }

  // update units 
  units = atoi(units.c_str());
  received_units = atoi(units.c_str());

  PHONE = "";//Clears phone string
  msg = "";//Clears message string
  // units = ""; // clears the units string
}

/**
 * @brief Reply sms
 * 
 * @param text 
 */
void gsm_reply(String text)
{
  Serial2.print("AT+CMGF=1\r");
  delay(1000);
  Serial2.print("AT+CMGS=\"" + PHONE + "\"\r");
  delay(1000);
  Serial2.print(text);
  delay(100);
  Serial2.write(0x1A); //ascii code for ctrl+z, DEC->26, HEX->0x1A
  delay(1000);
  Serial.println("SMS Sent Successfully.");
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

  // reset the relay_on_flag
  if(relay_on_off_flag){
    relay_on_off_flag = 0;
  }

  // reset the load turn on time for the next loop
  load_turn_on_time = 0;

}

/**
 * Turn on the relay
 * 
 */
void relay_turn_on(){
  digitalWrite(RELAY, HIGH);

  // set the load on flag to 1
  relay_on_off_flag = 1;

  // log the exact time that this device has been turned on
  load_turn_on_time = millis(); 

}

/**
 * Initialize the OLED
 * 
 */
void oled_init(){
  // configure screen
  display.begin();
}

void oled_show_message(String msg){
  display.setFont(u8g2_font_10x20_mr);
  
  display.firstPage();
  do {
    display.setFont(u8g2_font_helvR14_tr);
    display.drawStr(0,50, msg.c_str());

  } while ( display.nextPage() );
  
}

/**
 * show message on screen
 * 
 */
void oled_hello(){
  oled_show_message("Booting...");
  delay(1000);
}

/**
 * @brief Default screen when running
 * 
 * @param msg - power calculated from measured current
 */
void oled_default_screen(String msg){
  //display.clearDisplay();
  display.setFont(u8g2_font_10x20_mr);
  
  display.firstPage();
  do {
    display.setFont(u8g2_font_helvR14_tr);
    display.drawStr(0,16, "Power");
    display.drawStr(0,50, msg.c_str());

    display.drawStr(30, 50, "kWh");

  } while ( display.nextPage() );
}

/**
 * screen to show when a message is received by GSM
 * 
 */
void oled_message_received(){
  oled_show_message("Message received...");

  delay(1000);
  
  oled_show_message("Updating units...");

  delay(1000);

  oled_show_message("Units updated...");

  delay(700);

}

  
float calc_power_in_kwh(float current){
  // current received is in Amperes
  // calculate the power consumed in KW
  power_kw = (RMS_VOLTAGE * current ) / 1000.0;

  // get the time passed since the relay(load) was turned on
  current_time = millis();
  elapsed_time = current_time - load_turn_on_time;

  // convert the time to hours
  elapsed_time_hrs = elapsed_time / (1000 * 60 * 60);

  // calculate the power in KWh
  power_kwh = power_kw / elapsed_time_hrs;
  consumed_units = power_kwh;

  // power_kwh represents the units 
  // calculate the remaining units
  remaining_units = received_units - consumed_units;

  return power_kwh;

}

/**
 * Publish data to broker
 * 
 */
void mqtt_publish(float current){

  // create MQTT message
  snprintf(mqtt_msg, sizeof(mqtt_msg), "%.2f, %.2f", Amps_TRMS, power_kwh); // todo: check for correct length

  if(client.publish(topic, mqtt_msg)){
    debugln("[+]Data published");
  } else {
    debugln("[-]Failed to publish");
  }

}


/**
 * Setup
 * 
 */
void setup() {

  // connect to wifi
  wifi_connect();

  // init mqtt
  initialize_mqtt();

  // pinmodes
  // pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ALERT_LED, OUTPUT);
  pinMode(CURRENT_SENSOR_PIN, INPUT);

  // int oled
  oled_init();

  // flash oled hello
  oled_hello();

  // alert
  alert();
  
  // init serial 
  debugln("[+]Initializing hardware serial 1");
  Serial.begin(115200);
  delay(1000);

  Serial2.begin(115200);
  Serial.println("[+]Initializing GSM module...");

  Serial2.print("AT+CMGF=1\r"); //SMS text mode
  delay(1000);

}

void loop() {

  RunningStatistics inputStats;                 // create statistics   to look at the raw test signal
  inputStats.setWindowSecs( windowLength );     //Set   the window length
    

  ACS_Value = analogRead(CURRENT_SENSOR_PIN);   // read the analog in value:
  inputStats.input(ACS_Value);  // log to Stats   function
      
  if((unsigned long)(millis() - previousMillis) >= printPeriod)   { //every second we do the calculation
    previousMillis = millis();   //   update time
    
    Amps_TRMS = intercept + slope * inputStats.sigma();

    Serial.print( "Amps: " ); 
    Serial.print( Amps_TRMS );
    Serial.println();

    // update reading on screen
    oled_show_message((String) Amps_TRMS);
    
    // --------------------TRANSMIT TO MQTT------------------
    if(!client.connected()){
      debugln("[-]Client not connected...");
      unsigned long now = millis();

      if(now - last_reconnect_attempt > MQTT_RETRY_TIME){
        last_reconnect_attempt = now;

        if(mqtt_reconnect()){
          debugln("[+]Reconnected...");
          last_reconnect_attempt = 0;
        }
    }
    } else {
      client.loop();
      unsigned long current_millis = millis();

      if(current_millis - previous_millis >= SENSOR_POLL_TIME){
        previous_millis = current_millis;

        //mqtt_publish();

        //----------------publish to mqtt
        // create MQTT message
        snprintf(mqtt_msg, sizeof(mqtt_msg), "%.2f, %.2f", Amps_TRMS, power_kwh); // todo: check for correct length

        if(client.publish(topic, mqtt_msg)){
          debugln("[+]Data published");
        } else {
          debugln("[-]Failed to publish");
        }

      }

    }

    //-------------------------------------------------------

  }

  while (Serial2.available())
  {
    gsm_parse(Serial2.readString());//Calls the parseData function to parse SMS

    // TODO:update screen

    // TODO:alert
  }

  gsm_action();//Does necessary action according to SMS message

  while (Serial.available())
  {
    Serial2.println(Serial.readString());
  }

  // perform units(tokens) conversion
  debugln(received_units);

  // find out whether the remaining units have gone below the threshold
  if(remaining_units <= UNIT_THRESHOLD){
    // TODO: ALERT 
    alert();
    oled_show_message("Low on units. Please recharge.");
    delay(1000);

    // TODO: SEND SMS TO NOTIFY 

    // TODO: CUTOFF LOAD - consider cutting off load after units are all depleted
    relay_turn_off();

  }
}
//-------------------------END OF VOID LOOP--------------------------------

