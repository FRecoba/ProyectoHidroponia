#include  "Wifi.h"

char ssid[] = "Felipe";     //  your network SSID (name)
char pass[] = "Felipe";  // your network

char server[] = "iot.arnaldocastro.com.uy";
int port = 1883;
char username[] = "user";
char pswd[] = "pass";


void callback(char* topic, byte* payload, unsigned int length){
  return;
}

WiFiClient net;
PubSubClient client(server, port, callback, net);


boolean pub(char* cMqtt_topic, char*cMqtt_message) {
  Serial.print("Sending ");
  Serial.print(cMqtt_message);
  Serial.print(" to ");
  Serial.println(cMqtt_topic);
  return client.publish(cMqtt_topic, cMqtt_message);
}
boolean connectToServer() {

  Serial.print("Connecting to ruter");
  int status;
  int intentos = 1;
  while (status != WL_CONNECTED) {
    Serial.print('.');
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid);
    delay(500);
  }
  Serial.println("Connected");
  
  //WiFi.lowPowerMode();
  delay(500);
  Serial.print("Connecting to server");

    while (!client.connect("", username, pswd) && intentos <= 2) {
    Serial.print(".");
    intentos ++;
  }
//  while (!client.connect("", username, pswd, 0, 1, 0, 0) && intentos <= 2) {
//    Serial.print(".");
//    intentos ++;
//  }
  Serial.println("Connected");
  Serial.println(intentos);
  return (intentos <= 2);
}
void disconnectFromServer() {
  Serial.println("Disconnecting from server\n\n");
  client.disconnect();
  net.stop();
  WiFi.disconnect();
  WiFi.end();
}
