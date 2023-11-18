#ifndef DEFINES_H
#define DEFINES_H

/*========================PINS==========================================*/
#define CURRENT_SENSOR_PIN 32 // current sensor pin
#define BUZZER_PIN 25
#define RELAY 5

#define ALERT_DELAY 150
#define ALERT_LED 15

/*=======================ADC CONVERSION VARIABLES ========================*/
#define REFERENCE_VOLTAGE 3.3
#define RMS_VOLTAGE 230

#define UNIT_THRESHOLD 2

/*======================METER PARAMETERS=============================*/
const char* device_meter_id = "M01-1234-5678";
#define MESSAGE_SIZE 100 // 

/*
 * Debug variables
 */
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



#endif