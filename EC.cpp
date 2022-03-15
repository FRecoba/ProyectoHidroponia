#include "Arduino.h"
#include "EC.h"

float ECVoltageA = 289; //buffer solution 1.413mS /199.5
float ECVoltageB = 2152; //buffer solution 12.883mS / 1755.8

float  ecValue=0;


float voltageEC;

float leerEC(){
    voltageEC = (analogRead(EC_PIN)*3300)/4096;
    float slope = (12880-1413)/((ECVoltageB) -(ECVoltageA));  // two point
    float intercept = 1413 - slope*(ECVoltageA);
//    Serial.print("slope:");
//    Serial.print(slope);
//    Serial.print(",intercept:");
//    Serial.println(intercept);
    ecValue = slope*(voltageEC) + intercept;  //y = k*x + b
     
    return ecValue; //en microS
}
float medirEC(){		
	float auxVoltageEC =0;
	float auxEC = 0;
    for(int i=0; i<100 ; i++){
      leerEC();
      auxEC+=ecValue;
      auxVoltageEC+=voltageEC;
      delay(20);
    } 
    ecValue =auxEC/100;
    auxVoltageEC = auxVoltageEC/100;
    Serial.println("Voltage EC:");
    Serial.print(auxVoltageEC);
    Serial.println("mV");
    Serial.print("Value EC:");
    Serial.print(ecValue);
    Serial.println("miroS");
    if(ecValue<1500 || ecValue > 4500){
          ecValue = -1;
          Serial.println("Medida de EC fuera de rango");
        }
   return ecValue;	
}
float corregirEC(float ec, float temp){
	float ecCorregido = -1;
	ecCorregido = ec / (1.0-0.0185*(temp-25.0));
	return ecCorregido;
}
