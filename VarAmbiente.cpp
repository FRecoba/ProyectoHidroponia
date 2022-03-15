#define SHT31_ADDRESS   0x45
#include "VarAmbiente.h"
Adafruit_VEML7700 veml = Adafruit_VEML7700();

SHT31 sht;
int setupHUMTEMP() {
  Serial.println(__FILE__);
  Serial.print("SHT31_LIB_VERSION: \t");
  Serial.println(SHT31_LIB_VERSION);

  Wire.begin();
  sht.begin(SHT31_ADDRESS);
  Wire.setClock(100000);

  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();
  return(1);
}
int setupLUX() {
  Serial.println("Adafruit VEML7700 Setup");

  if (!veml.begin()) {
    Serial.println("Sensor not found");
    return(0);
    
  }
  Serial.println("Sensor found");

  veml.setGain(VEML7700_GAIN_1_8);
  veml.setIntegrationTime(VEML7700_IT_25MS);

  Serial.print(F("Gain: "));
  switch (veml.getGain()) {
    case VEML7700_GAIN_1: Serial.println("1"); break;
    case VEML7700_GAIN_2: Serial.println("2"); break;
    case VEML7700_GAIN_1_4: Serial.println("1/4"); break;
    case VEML7700_GAIN_1_8: Serial.println("1/8"); break;
  }

  Serial.print(F("Integration Time (ms): "));
  switch (veml.getIntegrationTime()) {
    case VEML7700_IT_25MS: Serial.println("25"); break;
    case VEML7700_IT_50MS: Serial.println("50"); break;
    case VEML7700_IT_100MS: Serial.println("100"); break;
    case VEML7700_IT_200MS: Serial.println("200"); break;
    case VEML7700_IT_400MS: Serial.println("400"); break;
    case VEML7700_IT_800MS: Serial.println("800"); break;
  }

  //veml.powerSaveEnable(true);
  //veml.setPowerSaveMode(VEML7700_POWERSAVE_MODE4);

//  veml.setLowThreshold(10000);
//  veml.setHighThreshold(20000);
//  veml.interruptEnable(true);
  return(1);
}

float leerLux(){
   
       float l = veml.readLux()/1000;
       Serial.print(l);
        Serial.println("K Lux");
        if(l<0 || l>25){
          l=-1;
        }
        return l;
}

float leerTempAmb(){
    sht.read();         // default = true/fast       slow = false

        Serial.print("TEMPERATURA:");
        Serial.print(sht.getTemperature(), 1);
        Serial.println("Celcius");
        float tempAmb = sht.getTemperature();
        if(tempAmb<-5 || tempAmb > 48){
          tempAmb = -100;
        }
        return tempAmb;
}
float leerHum(){
    sht.read();         // default = true/fast       slow = false

        Serial.print("HUMEDAD:");
        Serial.print(sht.getHumidity(), 1);
        Serial.println("%");
       float humAmb = sht.getHumidity();
        if(humAmb<0 || humAmb > 100){
          humAmb = -1;
        }
        return humAmb;
}
