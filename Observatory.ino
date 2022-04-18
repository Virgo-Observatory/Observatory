#include <Stepper.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Observatory.h"

const static int stepsPerRevolution = 200;

const static int temp_pin = 3; //??

const static int r1 = 4;
const static int r2 = 7;

const static int switch_ir = 8;
const static int switch_cam = 12;

int dht_pin = 2;

const static int stepper_speed = 60; // The speed is set in RPM 
const static int serial_speed = 9600;

bool cam_stat = false;
bool IR_stat = true;

Observatory *obs;

void setup() {

  // Init the serial cominication
  Serial.begin(serial_speed);
  Serial.println("**********************************************");
  Serial.println("* Welcome to the ARDUINO Observatory control *");
  Serial.println("*                MPC-CODE: M15               *");
  Serial.println("**********************************************");

  // Create the observatory object and set-up the pins
  obs = new Observatory(switch_cam, switch_ir, temp_pin, dht_pin, r1, r2);
  
}

void loop() {

  obs->control_status();

}
