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
const char* ssid = "Boen";
const char* password = "12345678";

/*======================TIMING VARIABLES==================================*/
unsigned long ACS_current_sample_time = 0, ACS_previous_sample_time = 0;
unsigned long ACS_sample_interval;
#define SENSOR_POLL_TIME 2000

/*============================GSM VARIABLES=============================*/
#define RXD2 16
#define TXD2 17

/*===========================TOKEN CONVERSION VARIABLES===================*/
String msg;
String PHONE = "+254700750148"; // TODO: change to owner's number 
int sms_reply_flag = 0;
int unit_value = 1000; // 1 token is 1 kWh
String meter_id = "";
String token;
String units;
String amount;
unsigned long load_turn_on_time = 0; // exact time the load is turned on
unsigned long current_time = 0;
int relay_on_off_flag = 0; // flag to tell whether the relay(load) is on or off
double received_units = 0; // units received from sms set to 5 initially
double consumed_units = 0;
double remaining_units;
double elapsed_time_hrs;
unsigned long elapsed_time = 0;
double power_kw = 0;
double power_kwh = 0;

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
void relay_turn_on();
void relay_turn_off();

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
  Serial.println(buff);

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

      // update units 
      units = atoi(units.c_str());
      received_units = atoi(units.c_str());
      remaining_units = received_units;
      // debug("Init Unts:"); debugln(received_units);


      index = buff.indexOf(0X22); // looking for position of the first double quotes
      PHONE = buff.substring(index + 1, index + 14); // extract the phone number

      // debug extracted variables
      Serial.println(PHONE);
      // Serial.println(units);

    }
  }

}


/**
 * Act depending on the GSM message
 * 
 */
void gsm_action(){
  
  if (remaining_units <= 0) {
    remaining_units = received_units;

    // turn off load
    // relay_turn_off();
  } else {
    
  }
  

  PHONE = "";//Clears phone string
  msg = "";//Clears message string
  // units = ""; // clears the units string

}

/**
 * @brief Reply sms
 * 
 * @param text 
 */
void gsm_reply(String text){
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial2.print("AT+CMGF=1\r");
  delay(1000);
  // updateSerial();
  Serial2.print("AT+CMGS=\"+254700750148\"\r");
  delay(1000);
  Serial2.print(text); // send SMS
  delay(100);
  Serial2.write(0x1A); //ascii code for ctrl+z, DEC->26, HEX->0x1A
  delay(1000);
  debugln("SMS Sent Successfully.");
}

void set_gsm_to_receive(){
  // Serial2.begin(115200);
  Serial.println("[+]Initializing GSM module...");
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  Serial2.print("AT+CMGF=1\r"); //SMS text mode

  // set up the GSM to send SMS to ESP32, not the notification only
  Serial2.print("AT+CNMI=2,2,0,0,0\r");
  delay(2000);
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    Serial2.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(Serial2.available()) 
  {
    Serial.write(Serial2.read());//Forward what Software Serial received to Serial Port
  }
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
  if(!relay_on_off_flag){
    relay_on_off_flag = 1;
  }
  
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

void oled_show_message(String msg, int flag){ // flag shows what we are printing, can be current, power, raw msg etc...
  display.setFont(u8g2_font_10x20_mr);

  if(flag == 0)  {
    // raw message
    display.firstPage();
    do {
      display.setFont(u8g2_font_helvR14_tr);
      display.drawStr(0,50, msg.c_str());

    } while ( display.nextPage() );
  } else if(flag == 1) {
    // current 
    display.firstPage();
    do {
      display.setFont(u8g2_font_helvR14_tr);
      display.drawStr(0,50, msg.c_str());
      display.drawStr(40, 50, "Amperes"); // TODO: calculate x position 70 is hardcoded : 

    } while ( display.nextPage() );
  } else if (flag == 2){
    // power
    display.firstPage();
    do {
      display.setFont(u8g2_font_helvR14_tr);
      display.drawStr(0,50, msg.c_str());
      display.drawStr(70, 50, "KWh"); // TODO: calculate x position 70 is hardcoded : 

    } while ( display.nextPage() );
  }
  
  
}

/**
 * show message on screen
 * 
 */
void oled_hello(){
  oled_show_message("Booting...", 0);
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
  oled_show_message("Message received...", 0);

  delay(1000);
  
  oled_show_message("Updating units...", 0);

  delay(1000);

  oled_show_message("Units updated...", 0);

  delay(700);

}

float cumulative_pow = 0;
float calc_cumulative_power(float power_kwh){
  cumulative_pow += power_kwh;

  return cumulative_pow;
}

  
float calc_power_in_kwh(float current){
  // current received is in Amperes
  // calculate the power consumed in KW
  power_kw = (RMS_VOLTAGE * current ) / 1000.0; // TODO: change this conversion to consume units faster - for demo

  // get the time passed since the relay(load) was turned on
  // current_time = millis();
  // elapsed_time = current_time - load_turn_on_time;

  // convert the time to hours
  // elapsed_time_hrs = (double) elapsed_time / (1000.0 * 60.0 * 60.0);

  // calculate the power in KWh
  // power_kwh = power_kw / elapsed_time_hrs;
  power_kwh = power_kw / 36000.0; // arbitrary for testing 

  // consumed_units = power_kwh; // TODO: here we assume 1kWh = 1 Unit 
  consumed_units = power_kw; // TODO: for demo

  // power_kwh represents the units 
  // calculate the remaining units
  remaining_units = remaining_units - consumed_units;

  // debug("ElpsdTm:"); debugln(elapsed_time_hrs);
  // debug("KWH:"); debugln(power_kwh);
  
  if(remaining_units > 0){
    oled_show_message((String)remaining_units + "     Units", 0); // show remaining units on screen 
    delay(1000);
  }
  // Serial.print("Units:"); Serial.println(remaining_units);Serial.print("\t");
  // Serial.print(elapsed_time);

  return power_kwh;
  // return power_kw;

}

/**
 * Publish data to broker
 * This function is not used
 * 
 */
// void mqtt_publish(float current){

//   // create MQTT message
//   Serial.println(cumulative_power_kwh);
//   snprintf(mqtt_msg, sizeof(mqtt_msg), "%.2f, %.2f, %.2f, %.2f, %d", Amps_TRMS, power_kwh, remaining_units, cumulative_power_kwh, relay_on_off_flag); // TODO: check for correct length

//   if(client.publish(topic, mqtt_msg)){
//     debugln("[+]Data published");
//   } else {
//     debugln("[-]Failed to publish");
//   }

// }

/**
 * Setup
 * 
 */
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(1000);

  // connect to wifi
  wifi_connect();

  // init mqtt
  initialize_mqtt();

  // turn on load - load is Normally ON
  relay_turn_on();

  // pinmodes
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ALERT_LED, OUTPUT);
  pinMode(CURRENT_SENSOR_PIN, INPUT);
  pinMode(RELAY, OUTPUT);
  Serial.println("Hello!!");

  // int oled
  oled_init();

  // flash oled hello
  oled_hello();

  // alert
  alert();
  
  // init serial 
  debugln("[+]Initializing hardware serial 1");
  delay(1000);

  // Serial2.begin(115200);
  Serial.println("[+]Initializing GSM module...");

  Serial2.print("AT+CMGF=1\r"); //SMS text mode

  // set up the GSM to send SMS to ESP32, not the notification only
  Serial2.print("AT+CNMI=2,2,0,0,0\r");
  delay(2000);

}

void loop() {

  while (Serial2.available()){
    gsm_parse(Serial2.readString());//Calls the parseData function to parse SMS

    // update screen
    oled_message_received();

    // alert
    alert();

  }

  //gsm_action(); // Does necessary action according to SMS message

  while (Serial.available()){
    Serial2.println(Serial.readString());
  }

  //--------------------measure current------------------------------
  RunningStatistics inputStats;                 // create statistics   to look at the raw test signal
  inputStats.setWindowSecs( windowLength );     //Set   the window length
    
  ACS_Value = analogRead(CURRENT_SENSOR_PIN);   // read the analog in value:
  inputStats.input(ACS_Value);  // log to Stats   function
      
  if((unsigned long)(millis() - previousMillis) >= printPeriod)   { //every second we do the calculation
    previousMillis = millis();   //   update time
    
    Amps_TRMS = intercept + slope * inputStats.sigma();

  }

  // update reading on screen
  oled_show_message((String) Amps_TRMS, 1);
  delay(1000);

  //-------------------------calc power in kwh-----------------------------
  calc_power_in_kwh(Amps_TRMS);
  float pwr = calc_power_in_kwh(Amps_TRMS);
  debug("Pwer: "); debugln(pwr);
  //-----------------------------------------

  // find out whether the remaining units have gone below the threshold
  if(remaining_units <= UNIT_THRESHOLD){
    // ALERT 
    alert();
    oled_show_message("Low on units", 0);
    delay(1000);

  }

  // turn off load if units are depleted
  if(remaining_units <= 0){
    relay_turn_off();
    
    // sms_reply_flag = 1;

    // if(sms_reply_flag == 1){
    //   Serial2.print("AT+CMGF=1\r");
    //   updateSerial();
    //   delay(1000);
    //   Serial2.print("AT+CMGS=\"" + PHONE + "\"\r");
    //   delay(1000);
    //   Serial2.print("Low units\r");
    //   updateSerial();
    //   delay(100);
    //   Serial2.write(0x1A); //ascii code for ctrl+z, DEC->26, HEX->0x1A
    //   updateSerial();
    //   delay(1000);
    //   Serial.println("SMS Sent Successfully.");
    //   sms_reply_flag = 0; 
    // } else {
    //   Serial2.print("AT+CNMI=2,2,0,0,0\r");
    //   delay(1000);
    // }

  } else {
    relay_turn_on();
  }
  
  // --------------------TRANSMIT TO MQTT------------------
  if(!client.connected()){
    debugln("[-]Client not connected...");
    unsigned long now = millis();

    if(now - last_reconnect_attempt > MQTT_RETRY_TIME){
      last_reconnect_attempt = now;

      if(mqtt_reconnect()){ // mqtt is connected at this point
        debugln("[+]Reconnected...");
        last_reconnect_attempt = 0;
      }
    }
  } else { // if mqtt is connected
    client.loop();
    unsigned long current_millis = millis();
             
    if(current_millis - previous_millis >= SENSOR_POLL_TIME){
      previous_millis = current_millis;

      //mqtt_publish();

      //----------------publish to mqtt------------------------------
      // create MQTT message
      // debug("p:"); debug(power_kw); debug("remun: "); debug(remaining_units); debug("ld_stat:"); debug(relay_on_off_flag);
      //snprintf(mqtt_msg, sizeof(mqtt_msg), "%.2f, %.2f, %.2f", Amps_TRMS, power_kw, remaining_units); // todo: check for correct length
      
      // calculated power connsumption
      float cumulative_power_kwh = calc_cumulative_power(power_kwh);
      // float cumulative_power_kwh = calc_cumulative_power(power_kwh);
      // debug("PowerKWh: "); debugln(power_kwh);

      debug("power: ");
      float p = calc_power_in_kwh(Amps_TRMS);
      debugln(p);

      // debug("total power: ");
      // debugln(cumulative_power_kwh);
      debug("REL_STAT:"); debugln(relay_on_off_flag);

      snprintf(mqtt_msg, sizeof(mqtt_msg), "%.2f, %.6f, %.2f, %.2f, %d", Amps_TRMS, p, remaining_units, cumulative_power_kwh, relay_on_off_flag); // TODO: check for correct length

      if(client.publish(topic, mqtt_msg)){
        // debugln("[+]Data published");
      } else {
        debugln("[-]Failed to publish");
      }

    }

  }

    //-------------------------------------------------------

  
}
//-------------------------END OF VOID LOOP--------------------------------

