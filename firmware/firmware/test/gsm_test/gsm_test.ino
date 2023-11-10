
/*============================GSM VARIABLES=============================*/
#define RXD2 16
#define TXD2 17

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  gsm_init();

}

void loop() {
  // poll for SMS
  gsm_update_serial(); // todo: change this method

}

/**
 * Init GSM module
 * 
 */
void gsm_init(){
  // init hardware serial format: Serial.begin(baud_rate, protocol, RX pin, TX pin)
  Serial.println("[...]Initializing hardware serial 2");
  delay(1000);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 initialized!");

  // perform handshake
  Serial.println("Attempting handshake...");
  Serial2.println("AT"); // will return OK if handshake sucessful
  gsm_update_serial();

  // configure text mode
  Serial.println("Configuring text mode...");
  Serial2.println("AT+CMGF=1"); 
  gsm_update_serial();

  // define how newly received SMS should be handled
  Serial2.println("AT+CNMI=1,2,0,0,0");

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
