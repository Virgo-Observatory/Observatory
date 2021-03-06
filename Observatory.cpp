#include "Observatory.h"
#include <LiquidCrystal_I2C.h>
#include <stdio.h>

Observatory::Observatory(int qhy, int ir, int temp_pin, int dht_pin, Stepper *step){
    qhy_cam_pin = qhy;
    ir_pin = ir;

    stat_qhy = false;
    stat_ir = false;

    /* Define the main object of the observatory:
       - Stepper motor of the focuser
       - The one-wire object for DallasTemperature sensors
       - The Dallas sensors
       - The atmospheric Temperature and Humidity sensors.
    */

    lcd = new LiquidCrystal_I2C(0x27, 16, 2);
    
    // Set-up the motor and put down all the inputs.
    step_motor = step;
    
    // step_motor = new Stepper(stepsPerRevolution, step1, step2, step3, step4);
    step_motor->setSpeed(stepper_speed);
    digitalWrite(step1, LOW);
    digitalWrite(step2, LOW);
    digitalWrite(step3, LOW);
    digitalWrite(step4, LOW);


    // Define the wire of the DallasT temperature sensors.
    wire = new OneWire(temp_pin);
    temp_sensors = new DallasTemperature(wire);

    // Atmospheric temperature and humidity,
    dht_sensor = new DHT(dht_pin, DHT11);

    // LiquidCrystal initialization.
    lcd->init();
    lcd->clear();
    lcd->backlight();
    stat_lcd = true;
    lcd->print("Weather Station");
    lcd->setCursor(0, 1);
    lcd->print("Virgo Obs. M15");
    
    // QHY camera off
    pinMode(qhy_cam_pin, OUTPUT);
    digitalWrite(qhy_cam_pin, LOW);
  
    // IR lamp off
    pinMode(ir_pin, OUTPUT);
    digitalWrite(ir_pin, LOW);

    // Wind pin
    pinMode(2, INPUT);

    // Dopo aver inizializzato l'impedenza dei PIN, posso lavorarci.
    // qhy_camera(false)
    IR_lamp(false);

    delay(500);

    // Start HT and I2C-T sensors
    temp_sensors->begin();
    dht_sensor->begin();

    // Number of I2C devices (Dallas Temp + LCD)
    deviceCount = temp_sensors->getDeviceCount();
    Serial.print("Temperature sensors: ");
    Serial.println(deviceCount); 

    // Distribution of the temperature sensors
    name[0] = String("CCD-QSI  : ");
    name[1] = String("CCD-GUIDE: ");
    name[2] = String("Opt.-Tube: ");
    name[3] = String("Dew-Zap  : ");
    name[4] = String("Opt.-Edg : ");
}

Observatory::~Observatory(){

    // Free the allocated memory
    delete[] wire;
    delete[] temp_sensors;
    delete[] dht_sensor;

}

void Observatory::focuser(){

    Serial.println("Focusing, insert the number of rounds: ");
    while(Serial.available() == 0) {}

    String rounds_c = Serial.readStringUntil('\n');
    int rounds = rounds_c.toInt();

    Serial.print("Move the focuser of ");
    Serial.print(rounds);
    Serial.print(" rounds .... ");

    Serial.print(" steps...");  
    step_motor->step(rounds);
    Serial.println(" DONE");
    digitalWrite(step1, LOW);
    digitalWrite(step2, LOW);
    digitalWrite(step3, LOW);
    digitalWrite(step4, LOW); 

}

void Observatory::qhy_camera(bool qhy){

    Serial.print("Camera power status: ");
    if (qhy){ 
        digitalWrite(qhy_cam_pin, HIGH);
        Serial.println("QHY Switched on");

    } else { 
        digitalWrite(qhy_cam_pin, LOW);
        Serial.println("QHY Switched off");
    }

}

void Observatory::IR_lamp(bool stat_ir){

    if(stat_ir) {
        digitalWrite(ir_pin, HIGH);
        Serial.println("IR-Lamp switched on");
        stat_ir = stat_ir;
        Serial.print("Status: ");
        if(stat_ir) Serial.println("ON");
        else Serial.println("OFF");

    } else {
        digitalWrite(ir_pin, LOW);
        Serial.println("IR-Lamp switched off");
        stat_ir = stat_ir;
    }
}

void Observatory::backlight_switch(bool stat_lcd){
    if(stat_lcd){
        lcd->backlight();
        Serial.println("Backlight ON");

    } else {
        lcd->noBacklight();
        Serial.println("Backlight OFF");
    }
}

void Observatory::get_status(){

    Serial.println("=========================");
    Serial.println("Observatory Status");
    Serial.println("=========================");
    Serial.println("=======  Devices   ======");
    Serial.println("=========================");
    Serial.print("IR-LAMP : ");
    if(stat_ir){
        Serial.println("Power-ON");
    } else {
        Serial.println("Power-OFF");
    }
   
    Serial.print("QHY-CCD : ");
    if(stat_qhy){
        Serial.println("Power-ON");
    } else {
        Serial.println("Power-OFF");
    }

    Serial.print("LCD-Stat: ");
    if(stat_lcd){
        Serial.println("Power-ON");
    } else {
        Serial.println("Power-OFF");
    }
   
    Serial.println("====== Atmosphere =======");
    Serial.print("Tatm    : ");
    Serial.print(t);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    Serial.print("Wind    : ");
    Serial.print(wind_speed);
    Serial.println(" km/h");
    Serial.print("Max     : ");
    Serial.print(max_wind_speed);
    Serial.println(" km/h");
    Serial.println("======= Telescope =======");

    temp_sensors->requestTemperatures();

    for(int i=0; i<deviceCount; i++){
        Serial.print("Sensor ");
        Serial.print(name[i]);
        Serial.print(" ");
        Serial.print(temp_sensors->getTempCByIndex(i));
        Serial.println(" C");
    }

    Serial.println("=========================");

}

void Observatory::get_temperatures(){

    temp_sensors->requestTemperatures();
    Serial.println("==================");
    for(int i=0; i<deviceCount; i++){
        Serial.print("Sensor ");
        Serial.print(name[i]);
        Serial.print(" ");
        Serial.print(temp_sensors->getTempCByIndex(i));
        Serial.println("C");
    }
    
}

void Observatory::control_status(){

    String in = Serial.readStringUntil('\n');

    /* Command selection:
       - Camera
       - Focuser
       - IrLamp
       - Status
       - Temperatures
      */ 
    if((in == String("Camera")) || (in == String("camera")))
    {
        stat_qhy = !stat_qhy;
        qhy_camera(stat_qhy);

    }

    if ((in == String("Focuser")) || (in == String("focuser"))) 
    {
        // Turn the focuser 
        focuser();
    }
    
    if ((in == String("IrLamp")) || (in == String("irlamp")))
    {
        // Switch-on the infrared lamp
        stat_ir = !stat_ir;
        IR_lamp(stat_ir);
    }

    if ((in == String("Status")) || (in == String("status")))
    {
        // A garbage collector of informations about the Observatory
        get_status();
    }

    if ((in == String("Temperatures")) || (in == String("temperatures")))
    {
        // This function returns the temperature of specific parts of the telescope.
        get_temperatures();
    }

    if ((in == String("scani2c")) || (in == String("Scan_I2C")))
    {
        scan_i2c_dev();
    }

    if ((in == String("backlight")) || (in == String("display")))
    {
        stat_lcd = !stat_lcd;
        backlight_switch(stat_lcd);
    }

    // Atmospheric data (Data acquired by the DHT11 sensors)
    h = dht_sensor->readHumidity();
    t = dht_sensor->readTemperature();

    // Control the Humidity
    // if( h > 70. ){
    //     IR_lamp(true);  
    // }
    // if( h < 50. ) {
    //     IR_lamp(false);
    // }

    lcd->clear();
    lcd->print("Temp: ");
    lcd->print(t);
    lcd->print("C");
    lcd->setCursor(0, 1);
    lcd->print("Hum : ");
    lcd->print(h);
    lcd->print("%");
   
}

void Observatory::setup(){


}

void Observatory::set_wind_speed(double w){

    wind_speed = w;
    if(w > max_wind_speed){
        // The Max Speed has to be associated to a RTC using an arduino
        // interrupt. Set speed -> Interrupt --> set_speed() -> get_time, and set_speed -> return (t, s)
        // The couple (t, s) can be encapsulaed in some private varibles.
        // The RT clock needs two pins for working. (!!!)
        max_wind_speed = w;
    }


void Observatory::scan_i2c_dev(){
    
    byte error, address;
    Serial.println("Scanning...");
    deviceCount = 0;
    for (address = 1; address < 127; address++ ) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
                if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");

            deviceCount++;
        }
        else if (error == 4) {
            Serial.print("Unknown error at address 0x");
                if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (deviceCount == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done");

}