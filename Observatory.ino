#include <Stepper.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Observatory.h"

// Temperature and Humidity sensor
int dht_pin = 13;
double radius = 40; // mm

// Interrupts dedicated to the wind step_count
int wind_pin = 2;
double wind_speed;
volatile unsigned long time=0;
volatile unsigned long time_new;
volatile unsigned long dt=0; // ms

// DallasTemperature sensors (read with OneWire)
const static int temp_pin = 3;

// Relay connected to the IR-Lamp and to the QHY camera
const static int switch_ir = 8;
const static int switch_cam = 12;

// Speed and step of the motor
const static int stepsPerRevolution = 200;
const static int stepper_speed = 60;

// Serial port baud-rate
const static int serial_speed = 9600;

// Inizialize the object "Observatory" as a pointer.
Observatory *obs;

Stepper step = Stepper(stepsPerRevolution, 6, 9, 10, 11);

void setup() {

  // Init the serial cominication
  Serial.begin(serial_speed);
  Serial.println("**********************************************");
  Serial.println("* Welcome to the ARDUINO Observatory control *");
  Serial.println("*                MPC-CODE: M15               *");
  Serial.println("**********************************************");

  // Create the observatory object and set-up the pins
  obs = new Observatory(switch_cam, switch_ir, temp_pin, dht_pin, &step);
  
  attachInterrupt(digitalPinToInterrupt(wind_pin), count_wind_step, RISING);
  
}

void loop() {

  /*I would like to implement a sort of interactive shell with two main environment:
    - The first env is dedicated to the control of the observatory durin the daylight
      and during the non-observative nights
    - The second env will be dedicated to directly interact with the devices. For example,
      swith on/off the irlamp for brief inspection of the telescope connections, focuser,
      and other ancillary options that will be implemented in time.
  */
  
  obs->control_status();

}

void count_wind_step(){

  time_new = millis();
  dt = time_new-time;
  time = time_new;

  wind_speed = 3.6 * ( (2 * 3.141592 * radius) / dt );

  obs->set_wind_speed(wind_speed);

}
