#include <Stepper.h>

#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include <LiquidCrystal_I2C.h>

#include<stdlib.h>

class Observatory {
  public:
    Observatory(int qhy_cam_pin, 
                int ir_pin, 
                int temp_pin, 
                int dht_pin,
                Stepper *step);
    ~Observatory();

    void focuser();

    void qhy_camera(bool stat_qhy);

    void backlight_switch(bool stat_lcd);

    void IR_lamp(bool stat_ir);

    void get_status();

    void get_temperatures();

    void control_status();

    void setup();

    void scan_i2c_dev();

    void set_wind_speed(double w);

  private:
    Stepper *step_motor;
    OneWire *wire;
    DallasTemperature *temp_sensors;
    DHT *dht_sensor;
    LiquidCrystal_I2C *lcd;

    bool stat_ir;
    bool stat_qhy;
    bool stat_obs;
    bool stat_lcd;
    int deviceCount;
    String name[5];

    const static int stepsPerRevolution = 200;
    const static int stepper_speed = 60;
    
    // Step motor pins
    const static int step1 = 6; 
    const static int step2 = 9;
    const static int step3 = 10; 
    const static int step4 = 11;

    int qhy_cam_pin;
    int ir_pin;
    int temp_pin;
    int dht_pin;
    int r1;
    int r2;

    // DHT measures
    float h;
    float t;
    float wind_speed=0;

};
