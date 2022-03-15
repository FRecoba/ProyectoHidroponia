#include <SPI.h>
#include <Time.h>
#include <WiFi101.h>
#include "PubSubClientMKR-WiFi.h"

boolean pub(char* cMqtt_topic,char*cMqtt_message);
boolean connectToServer();
void setupWiFi();
void disconnectFromServer();
