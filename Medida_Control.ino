#include "PH.h"
#include "EC.h"
#include "Temperature.h"
#include "VarAmbiente.h"
#include "WiFi.h"

//############
//Variables publicacion
//############
// MQTT TOPIC AND MESSAGE
char cMqtt_topic[50];
char cMqtt_message[200];
boolean publicar = true;
int publicarsatate = 2;
unsigned long tProximaPublicacion = 300000;
unsigned long tUltimaPublicacion = 0;


//############
//Variables PH
//############
boolean statusPH = true;
int PHsatate = 1;
float ArrayPH[10];
int punteroPH = 0;
float PHpromedio = 0;
float PHpromedioAux = 0;

float PHUmbral = 5.6;
float PHUmbralMedio = 5.8;
float PHUmbralAlto = 6;
const int segDispensadoPH = 2000; //90ml cada 10 sg 

unsigned long inicioActuadorPH = 0;
unsigned long tiempoActuadorPH = 2000; //90ml cada 10 sg
unsigned long ProximaMedidiaPH = 72000000;
const byte pinBombaAcido = 1;


//############
//Variables EC
//############
boolean statusEC = true;
int ECsatate = 0;
float ArrayEC[10];
int punteroEC = 0;
float ECpromedio = 0;

float ECUmbral = 2500;
float ECUmbralMedio = 2400;
float ECUmbralMedioBajo = 2300;
float ECUmbralBajo = 2200;
const int segDispensadoCE = 60000; //100ml por min por bomba 

unsigned long inicioActuadorEC = 0;
unsigned long tiempoActuadorEC = 60000; //100ml por min por bomba 
unsigned long ProximaMedidiaEC = 300000;

const byte pinBombaEcA = 2;
const byte pinBombaEcB = 3;
const byte pinBombaEcMedir = 5;
const byte pinBombaEcLimpiar = 4;

unsigned long inicioTomaEC = 0;
unsigned long inicioLimpiezaEC = 0;
unsigned long inicioVaciadoEC = 0;
unsigned long tiempoTomaEC = 100000;
unsigned long tiempoLimpiezaEC = 50000;
unsigned long tiempoVaciadoEC = 180000;

//########################
//Variables Temp & Hum & lux
//########################
boolean statusHumTempLux = true;
int ambsatate = 1;
float Klux = 0;
float KluxAux = 0;
float humAmb = 0;
float humAmbAux = 0;
float tempAmb = 0;
float tempAmbAux = 0;
unsigned long tUltimaAmbiente = 0;
unsigned long tProximaAmbiente = 60000;

//########################
//Variables Distancia
//########################
boolean statusDistancia = true;
int distanciasatate = 1;
unsigned long tUltimaDistancia = 0;
unsigned long tProximaDistancia = 60000;

unsigned long durationDistancia;
unsigned long inicioDistancia = 0;
unsigned long finalDistancia = 0;
boolean midiendoDistancia = false;
float ArrayDistancia[11];
int posicion = 0;

const byte pinInteruptDistance = A2;//tx
const byte pinRXDistance = 6;
float distancia = 0;
unsigned int nivel = 0;

//########################
//Variables bombeo de potencia
//########################
const byte pinBombaPower = 0;


void setup() {
   //Prendo bombeo principal
  digitalWrite(pinBombaPower, LOW);
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Configuracion de ADC
  analogReadResolution(12);
  delay(1000);
  setupHUMTEMP();
  setupLUX();
  setupDistancia();
  for (int i = 0; i < 10 ; i++) {
    leerDistancia();
    delay(500);
    filtro(nivel);
    
  }
  
 

  //Poner todos los pines en output-low
  setupReles();

  //Armo el topic de MQTT
  char charBuff[50];
  String sTopic = "/b4k4r/hidrosmart/attrs";
  sTopic.toCharArray(charBuff, 50);
  strcpy(cMqtt_topic, charBuff);

  // Variables temporales de arranque
  inicioTomaEC = millis();
  
 
}

void loop() {
  if (statusPH) {
    switch (PHsatate) {
      case 1:   //MedirPH:
        if (punteroPH < 10) {
          ArrayPH[punteroPH] = medirPH();
          punteroPH++;
        } else {
          PHpromedio = promediarPH();
          punteroPH = 0;
          PHsatate = 2;
          Serial.println("PH promedio:");
          Serial.println(PHpromedio);
        }
        break;
      case 2:  //MedirTemp:
       PHpromedioAux = corregirPH(PHpromedio, medirTemp());
       if(PHpromedioAux<=7 && PHpromedioAux>= 3.5){
         PHpromedio = PHpromedioAux;
        PHpromedio = PHpromedio + 0.2; // sumo error 17-1-22
       }
        Serial.print("PH corregido:");
        Serial.println(PHpromedio);
        if (PHpromedio > PHUmbral) { //Evaluo si estoy fuera de rango
          ProximaMedidiaPH = 900000
          //Calculo cuantos ml dispensar
          inicioActuadorPH = millis();
          if(PHpromedio<PHUmbralMedio){
            ProximaMedidiaPH = 1800000
            tiempoActuadorPH = segDispensadoPH;
          }else if(PHpromedio<PHUmbralAlto){
            tiempoActuadorPH = segDispensadoPH*2;
          }else tiempoActuadorPH = segDispensadoPH*3;
          
          PHsatate = 3;
          Serial.println("BOMBA ON");
        } else {
          PHsatate = 4;
          inicioActuadorPH = millis();
          ProximaMedidiaPH = 72000000;
        }
        break;
      case 3: //ActuarPH:
        if (millis() - inicioActuadorPH < tiempoActuadorPH) { //Evaluo el tiempo actuado
          //Bomba PH ON
          //Enciendo bomba suministro de acido
          digitalWrite(pinBombaAcido, LOW);
        } else {
          //Bomba de acido OFF
          digitalWrite(pinBombaAcido, HIGH);
          PHsatate = 4;
          Serial.println("BOMBA OFF");
        }
        break;
      case 4:
        if (millis() - (inicioActuadorPH) > ProximaMedidiaPH) {//Evaluo el tiempo de espera
          PHsatate = 1;
        }
        break;
      default:
        // statements
        break;
    }

  }
  if (statusEC) {
    switch (ECsatate) {
      case 0:
        if (millis() - inicioTomaEC < tiempoTomaEC) {
          //Enciendo bomba de toma de muestra
          digitalWrite(pinBombaEcMedir, LOW);
        } else {
          ECsatate = 1;
          //Apagociendo bomba de toma de muestra
          digitalWrite(pinBombaEcMedir, HIGH);
        }

        break;
      case 1:   //MedirEC:
        if (punteroEC < 10) {
          ArrayEC[punteroEC] = medirEC();
          punteroEC++;
        } else {
          float ecAux = promediarEC() + 200;
          if((ecAux <4500) && (ecAux > 1500) &&  (nivel >= 50)){
            ECpromedio = ecAux;
          }
         
          punteroEC = 0;
          ECsatate = 2;
          Serial.println("EC promedio:");
          Serial.println(ECpromedio);
          inicioVaciadoEC = millis();
        }
        break;
      case 2:  //MedirTemp:
       if(millis() - inicioVaciadoEC > tiempoVaciadoEC){
        if (millis() - inicioLimpiezaEC < tiempoLimpiezaEC) {
          //Enciendo bomba de limpieza
          digitalWrite(pinBombaEcLimpiar, LOW);
        } else {
          //Apago bomba de limpieza
          digitalWrite(pinBombaEcLimpiar, HIGH);

          ECpromedio = corregirEC(ECpromedio, medirTemp() ); //medirTemp());
          Serial.println("EC corregido:");
          Serial.println(ECpromedio);
          if ((ECpromedio < ECUmbral) &&( ECpromedio > 0)) {
            
            ECsatate = 3;
            if(ECpromedio > ECUmbralMedio){
              tiempoActuadorEC = segDispensadoEC;
            }else if(ECpromedio > ECUmbralMedioBajo){
              tiempoActuadorEC = segDispensadoEC*2;
            }else if(ECpromedio > ECUmbralBajo){
              tiempoActuadorEC = segDispensadoEC*3;
            }else tiempoActuadorEC = segDispensadoEC*4;
              
            //Inicio de la actuacion EC
            inicioActuadorEC = millis();
            Serial.println("BOMBAS A & B ON");

            ProximaMedidiaEC = 1800000;
          } else {
            //Inicio de la actuacion EC
            inicioActuadorEC = millis(); // solo para la espera
            ECsatate = 4;
            ProximaMedidiaEC = 21.6E6;
          }
        }
       }else inicioLimpiezaEC = millis();
        break;
      case 3: //ActuarEC:
        if (millis() - inicioActuadorEC < tiempoActuadorEC) {

          //Bomba EC ON
          digitalWrite(pinBombaEcA, LOW);
          digitalWrite(pinBombaEcB, LOW);
        } else {
          //Bomba de EC OFF
          digitalWrite(pinBombaEcA, HIGH);
          digitalWrite(pinBombaEcB, HIGH);
          
          Serial.println("BOMBAS A & B OFF");
          ECsatate = 4;
        }

        break;
      case 4:
        if (millis() - (inicioActuadorEC) > ProximaMedidiaEC) {
          ECsatate = 0;
          inicioTomaEC = millis();
        }
        break;

      default:
        // statements
        break;
    }

  }
  if (statusHumTempLux) {
    switch (ambsatate) {
      case 1:
        //Lectura de Temp & Hum
        tempAmbAux = leerTempAmb();
        if(tempAmbAux != -100){
          tempAmb = tempAmbAux;
        }
        humAmbAux = leerHum();
        if(humAmbAux != -1){
          humAmb = humAmbAux;
        }
        medirTemp();
        ambsatate = 2;
        break;
      case 2:  //MedirLux:
        //Lectura de luxes
         KluxAux = leerLux();
        if(KluxAux != -1){
           Klux = KluxAux;
        }
        ambsatate = 3;
        tUltimaAmbiente = millis();
        break;
      case 3: //Estdo de espera
        if (millis() - tUltimaAmbiente > tProximaAmbiente) {
          ambsatate = 1;
        }
        break;
      default:
        // statements
        break;
    }

  }
  if (statusDistancia) {
    switch (distanciasatate) {
      case 1:
        //Lectura de Temp & Hum
        leerDistancia();
        nivel = filtro(nivel); 
        distanciasatate = 2;
        tUltimaDistancia = millis();
        break;
      case 2:  //Estado espera:
        if (millis() - tUltimaDistancia > tProximaDistancia) {
          distanciasatate = 1;
        }
        break;
      default:
        // statements
        break;
    }
  }
  if (publicar) {
    switch (publicarsatate) {
      case 1:
        //Lectura de Temp & Hum
        buildMQTTMessage();
         if(connectToServer() == 0){
          setup();
         }
        if (!pub(cMqtt_topic, cMqtt_message)) { //Evaluo estado del envio
          setup();//Reinicio sistema
        }
        disconnectFromServer();
        
        publicarsatate = 2;
        tUltimaPublicacion = millis();
        break;
      case 2:  //Estado espera:
        if (millis() - tUltimaPublicacion > tProximaPublicacion) {
          publicarsatate = 1;
        }
        break;
      default:
        // statements
        break;
    }
  }
}
float promediarPH() {
  float aux = 0;
  if (punteroPH == 10) {
    for (int i = 0; i < 10; i++) {
      aux += ArrayPH[i];
    }
    aux = aux / 10;
  }
  return aux;
}
float promediarEC() {
  float aux = 0;
  if (punteroEC == 10) {
    for (int i = 0; i < 10; i++) {
      aux += ArrayEC[i];
    }
    aux = aux / 10;
  }
  return aux;
}
void leerDistancia() {
  attachInterrupt(digitalPinToInterrupt(pinInteruptDistance), isrI, CHANGE);
  digitalWrite(pinRXDistance, LOW);
  delay(80);
  digitalWrite(pinRXDistance, HIGH);
  delay(500);
  //Serial.println("Time microS");
  //Serial.println(durationDistancia);
  Serial.println("distance>");
  Serial.print(durationDistancia / 57.5);
  Serial.println("cm");
  distancia = durationDistancia / 57.5;
  if ((5 < distancia) && (distancia < 55)){
    nivel = ((55 - distancia) /(50))* 100;
      Serial.println("Nivel>");
  Serial.print(nivel);
  Serial.println("%");
  }else setupDistancia();
  detachInterrupt(digitalPinToInterrupt(pinInteruptDistance));
}
void isrI() {
  if (!midiendoDistancia) {
    inicioDistancia = micros();
    midiendoDistancia = true;
    //Serial.println('I');
  } else {
    durationDistancia = micros() - inicioDistancia;
    midiendoDistancia = false;
  }
}
int setupDistancia() {
  pinMode(pinRXDistance, INPUT_PULLUP);
  pinMode(pinRXDistance, OUTPUT);
  digitalWrite(pinRXDistance, HIGH);
  midiendoDistancia = false;
  pinMode(pinInteruptDistance, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(pinInteruptDistance), isrI, CHANGE);
  return 1;
}

int setupReles() {

  pinMode(pinBombaAcido, OUTPUT);
  digitalWrite(pinBombaAcido, HIGH);
 

  pinMode(pinBombaEcA, OUTPUT);
  digitalWrite(pinBombaEcA, HIGH);
  pinMode(pinBombaEcB, OUTPUT);
  digitalWrite(pinBombaEcB, HIGH);
  pinMode(pinBombaEcMedir, OUTPUT);
  digitalWrite(pinBombaEcMedir, HIGH);
  pinMode(pinBombaEcLimpiar, OUTPUT);
  digitalWrite(pinBombaEcLimpiar, HIGH);
  pinMode(pinBombaPower, OUTPUT);
  digitalWrite(pinBombaPower, LOW);
  return 1;
  

}

/*****************************************
   Genero el mensaje JSON a enviar
 *****************************************/
void buildMQTTMessage() {



  // Variables para el armado de los datos
  char charBuf[200];
  String sMessage = "{\"tempamb\":" + String(tempAmb) + ", \"hum\":" + String(humAmb) + ", \"ph\":" + String(PHpromedio) + ", \"ec\":" + String(ECpromedio) +  ", \"nivel\":" + String(nivel) + ", \"klux\":" + String(Klux) + "}";

  Serial.println(sMessage);
  // Cargo el mensaje en el cMqtt_message
  sMessage.toCharArray(charBuf, 200);
  strcpy(cMqtt_message, charBuf);
  Serial.println(cMqtt_message);

}
int filtro(int medida) {
  int arrayAux[20];
  int p;
  ArrayDistancia[posicion] = medida;
  if (++posicion >= 10) {
    posicion = 0;
  }
  for (p = 0; (p <= 10) ; p++) {
    arrayAux[p] = ArrayDistancia[p];
  }

  int i, j, flag = 1;
  int temp;
  for (i = 1; (i <= 10) && flag; i++)
  {
    flag = 0;
    for (j = 0; j < (10 - 1); j++)
    {
      if (arrayAux[j + 1] < arrayAux[j])
      {
        temp = arrayAux[j];
        arrayAux[j] = arrayAux[j + 1];
        arrayAux[j + 1] = temp;
        flag = 1;
      }
    }
  }
  int retorno = 0;
  for (i = 3; i <= 8 ; i++) {
    retorno += arrayAux[i];
  }
  Serial.print("Filtro: ");
  Serial.println(String(medida) + "---->" + String(retorno / 6));


  return retorno / 6;
}
