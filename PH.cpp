#include "Arduino.h"
#include "PH.h"
float temperature    = 25.0;
float acidVoltage    = 2027;    //buffer solution 4.0 at 25C
float neutralVoltage = 1511;     //buffer solution 7.0 at 25C //1511




float  phValue=0;


float  voltagePH;


float leerPH(){
    voltagePH = (analogRead(PH_PIN)*3300)/4096;
    float slope = (7.0-4.0)/((neutralVoltage-1500.0)/3.0 - (acidVoltage-1500.0)/3.0);  // two point: (_neutralVoltage,7.0),(_acidVoltage,4.0)
    float intercept =  7.0 - slope*(neutralVoltage-1500.0)/3.0;
//    Serial.print("slope:");
//    Serial.print(slope);
//    Serial.print(",intercept:");
//    Serial.println(intercept);
    phValue = slope*(voltagePH-1500.0)/3.0+intercept;  //y = k*x + b
    return phValue;
}
float medirPH(){		
	float auxVoltagePH =0;
	float auxPH = 0;
    for(int i=0; i<100 ; i++){
      leerPH();
      auxPH+=phValue;
      auxVoltagePH+=voltagePH;
      delay(20);
    } 
    phValue =auxPH/100;
    auxVoltagePH = auxVoltagePH/100;
    Serial.println("Voltage pH:");
    Serial.print(auxVoltagePH);
    Serial.println("mV:");
    Serial.print("Value pH:");
    Serial.println(phValue);
   return phValue;	
}
float corregirPH(float ph, float temp){
	float phCorregido = -1;
	phCorregido = ph + ((ph -7)*(25-temp)*0.24/7/10);
	return phCorregido;
}
