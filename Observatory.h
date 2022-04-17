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
                int r1,
                int r2);
    ~Observatory();

    void focuser();

    void qhy_camera();

    bool IR_lamp(bool stat_ir);

    void get_status();

    void get_temperatures();

    void control_status();

    void setup();

    void scan_i2c_dev();

  private:
    Stepper *step_motor;
    OneWire *wire;
    DallasTemperature *temp_sensors;
    DHT *dht_sensor;

    bool stat_ir;
    bool stat_qhy;
    bool stat_obs;
    int deviceCount;

    const static int stepsPerRevolution = 200;
    const static int stepper_speed = 60;
    

    const static int step1 = 6; 
    const static int step2 = 9;
    const static int step3 = 10; 
    const static int step4 = 11;

    const static int relay_1 = 4;
    const static int relay_2 = 7;

    int qhy_cam_pin;
    int ir_pin;
    int temp_pin;
    int dht_pin;
    int r1;
    int r2;

    // DHT measures
    float h;
    float t;

};
