/*============================GSM VARIABLES=============================*/
#define RXD2 16
#define TXD2 17


void setup()
{
  Serial.begin(115200);
  Serial.println("[...]Initializing hardware serial 2");
  delay(1000);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 initialized!");
  Serial.println("Initializing...");
  delay(1000);

  Serial2.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  Serial2.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  Serial2.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  Serial2.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
}

void loop() {
  updateSerial(); // todo: change this method


}

void updateSerial(){
  delay(500);
  while(Serial.available()){
    Serial2.write(Serial.read()); // forward what programming serial receives to Serial2
  }

  while(Serial2.available()){
    Serial.write(Serial2.read()); // forward what serial2 recieves to programming serial for debug
  }

}
