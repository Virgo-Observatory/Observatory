#include "Observatory.h"
#include <LiquidCrystal_I2C.h>
#include <stdio.h>

Observatory::Observatory(int qhy_cam_pin, int ir_pin, int temp_pin, int dht_pin, int r1, int r2) {
  
    /* Define the main object of the observatory:
       - Stepper motor of the focuser
       - The one-wire object for DallasTemperature sensors
       - The Dallas sensors
       - The atmospheric Temperature and Humidity sensors.
    */
    lcd = new LiquidCrystal_I2C(0x27, 16, 2);
    step_motor = new Stepper(stepsPerRevolution, step1, step2, step3, step4);
    wire = new OneWire(temp_pin);
    temp_sensors = new DallasTemperature(wire);
    dht_sensor = new DHT(dht_pin, DHT11);

    // LiquidCrystal initialization
    lcd->init();
    lcd->clear();
    lcd->backlight();
    lcd->print("Weather Station");
    lcd->setCursor(0, 1);
    lcd->print("Virgo Obs. M15");
    
    // QHY camera off
    pinMode(qhy_cam_pin, OUTPUT);
    digitalWrite(qhy_cam_pin, LOW);
  
    // IR lamp off
    pinMode(ir_pin, OUTPUT);
    digitalWrite(ir_pin, LOW);

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
    delete[] step_motor;
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

void Observatory::qhy_camera(){

    Serial.println("Camera Setting");
    Serial.print("Camera power status: ");
    Serial.println(stat_qhy);
    Serial.println("What do you want to do? (use 0/1 to switch off/on, respectively");
    while(Serial.available() == 0) {}

    String cam_status_now = Serial.readStringUntil('\n');
    int rel_stat = cam_status_now.toInt();

    if (rel_stat == 1) stat_qhy == true;
    else stat_qhy = false;

    digitalWrite(qhy_cam_pin, rel_stat);

    Serial.print("Camera power status: ");
    Serial.println(stat_qhy);

}

bool Observatory::IR_lamp(bool stat_ir){

    if(stat_ir) {
        digitalWrite(ir_pin, HIGH);
        stat_ir = !stat_ir;
    } else {
        digitalWrite(ir_pin, LOW);
        stat_ir = !stat_ir;
    }

    return stat_ir;

}

void Observatory::get_status(){

    Serial.println("==================");
    Serial.println("Observatory Status");
    Serial.println("==================");
    Serial.println("===  Devices   ===");
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
   
    Serial.println("=== Atmosphere ===");
    Serial.print("Tatm    : ");
    Serial.print(t);
    Serial.println("C");
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println("%");
    Serial.println("=== Telescope  ===");

    temp_sensors->requestTemperatures();

    for(int i=0; i<deviceCount; i++){
        Serial.print("Sensor ");
        Serial.print(name[i]);
        Serial.print(" ");
        Serial.print(temp_sensors->getTempCByIndex(i));
        Serial.println("C");
    }
    Serial.println("==================");

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
        // Switch on/off the guiding camera
        qhy_camera();
    }

    if ((in == String("Focuser")) || (in == String("focuser"))) 
    {
        // Turn the focuser 
        focuser();
    }
    
    if ((in == String("IrLamp")) || (in == String("irlamp")))
    {
        // Switch-on the infrared lamp
        IR_lamp(true);
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

    // Atmospheric data (Data acquired by the DHT11 sensors)
    h = dht_sensor->readHumidity();
    t = dht_sensor->readTemperature();

    // Control the Humidity
    if( h > 70. ){
        IR_lamp(true);  
    }
    if( h < 50. ) {
        IR_lamp(false);
    }

    lcd->clear();
    lcd->print("Temperature: ");
    lcd->println(t);
    lcd->setCursor(0, 1);
    lcd->print("Humidity: ");
    lcd->print(h);


    
}

void Observatory::setup(){


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
        Serial.println("done\n");

}