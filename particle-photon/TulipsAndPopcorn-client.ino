// Wire library is built in 
#include <math.h>
#include <stdint.h>
#include <spark_wiring_i2c.h>

LSM303D_Compass compass;


double heading;

int16_t twos_comp_combine(uint8_t msb,uint8_t lsb){
  int16_t twos_comp = 256*msb + lsb;
  if(twos_comp >= 32768){
    return twos_comp - 65536;
  }
  else{
    return twos_comp;
  }
}
void setup() {
  int8_t resp;
  uint8_t singleChar;
  
  
  delay(5000);
  
  Serial.begin(115200);
  
  while(!Serial.available()) Particle.process();
  
  Serial.println("TulipsAndPopcorn: Init\r\n-----------------");
  
  
  Wire.setSpeed(CLOCK_SPEED_100KHZ);
  //Wire.stretchClock(true);
  if ( !Wire.isEnabled() ) {
    Wire.begin();   //Join the I2C bus as the I2C Master
  }
  
  Wire.beginTransmission(COMPASS_ADDR); 
  Wire.write(COMPASS_CMD_WHO_AM_I);
  Wire.requestFrom(COMPASS_ADDR, 1);
  while(!Wire.available());
  
  singleChar = Wire.read();
  if(singleChar != COMPASS_CMD_WHO_AM_I_RESULT){
    Serial.printlnf("COMPASS: WHO_AM_I --> ERROR Read %x",singleChar);
  }
  
  //Set CTRL1 Register Value
  Wire.beginTransmission(COMPASS_ADDR);
  Wire.write(COMPASS_CMD_CTRL1_MODE[0]);
  Wire.write(COMPASS_CMD_CTRL1_MODE[1]);
  resp = Wire.endTransmission();
  if(resp){
   Serial.printlnf("COMPASS: CTRL1 Set --> ERROR # %d",resp);
  }
  
  //Set CTRL2 Register Value
  Wire.beginTransmission(COMPASS_ADDR);
  Wire.write(COMPASS_CMD_CTRL2_MODE[0]);
  Wire.write(COMPASS_CMD_CTRL2_MODE[1]);
  resp = Wire.endTransmission();
  if(resp){
    Serial.printlnf("COMPASS: CTRL2 Set --> ERROR # %d",resp);
  }
  
  //Set CTRL5 Register Value
  Wire.beginTransmission(COMPASS_ADDR);
  Wire.write(COMPASS_CMD_CTRL5_MODE[0]);
  Wire.write(COMPASS_CMD_CTRL5_MODE[1]);
  resp = Wire.endTransmission();
  if(resp){
    Serial.printlnf("COMPASS: CTRL5 Set --> ERROR # %d",resp);
  }
  
  //Set CTRL6 Register Value
  Wire.beginTransmission(COMPASS_ADDR);
  Wire.write(COMPASS_CMD_CTRL6_MODE[0]);
  Wire.write(COMPASS_CMD_CTRL6_MODE[1]);
  resp = Wire.endTransmission();
  if(resp){
    Serial.printlnf("COMPASS: CTRL6 Set --> ERROR # %d",resp);
  }
  
  //Set CTRL7 Register Value
  Wire.beginTransmission(COMPASS_ADDR);
  Wire.write(COMPASS_CMD_CTRL7_MODE[0]);
  Wire.write(COMPASS_CMD_CTRL7_MODE[1]);
  resp = Wire.endTransmission();
  if(resp){
    Serial.printlnf("COMPASS: CTRL7 Set --> ERROR # %d",resp);
  }
  
  if (Particle.variable("heading", &heading,DOUBLE)==false){
    Serial.println("Heading Variable not registered");
  }
     
    
}

void loop() {
    int8_t resp;
    int16_t magx,magy,magz,accx,accy,accz;
    uint8_t high,low;
    
    while(true){
        delay(250);
        Wire.beginTransmission(COMPASS_ADDR); // transmit to slave device #4
    
        Wire.write(COMPASS_CMD_MAGN_X_LSB|0x80);
        
        resp = Wire.endTransmission();
        if(resp!=0){
             Serial1.printlnf("\t I2C: Close --> ERROR # %d",resp);
        }
        
        Wire.requestFrom(COMPASS_ADDR,6);
        
        while(!Wire.available());
        
        low = Wire.read();
        high = Wire.read();
        magx = twos_comp_combine(high, low);
        
        low = Wire.read();
        high = Wire.read();
        magy =twos_comp_combine(high, low);
        
        low = Wire.read();
        high = Wire.read();
        magz = twos_comp_combine(high, low);
        
        resp = Wire.endTransmission();
        if(resp!=0){
             Serial1.printlnf("\t I2C: Close --> ERROR # %d",resp);
        }
        Serial.printlnf("Magnetic field (%d, %d, %d):", magx, magy, magz);
        
        heading = atan2 (magy,magx) * 180.0/ M_PI;
        
        if(heading < 0) {
            heading += 360;
        }
        Serial.printlnf("Heading: %f", heading);
        
    }
   

}
