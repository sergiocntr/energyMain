#ifndef eneMain_h
#define eneMain_h
#pragma once
//#define DEBUGMIO
#include <Arduino.h>
#include <pitches.h>
#include <ESP8266WiFi.h>
//#include <ESP8266Ping.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoJson.h>
#include <topic.h>
#include <myIP.h>
#include "password.h"
#include <nodeRelay.h>
#include <pin.h>
#include <MedianFilter.h>
#include <PZEM004Tv30.h>
//#include <crash.h>
WiFiClient mywifi;
WiFiClient c;
PubSubClient client(c);
const uint8_t versione =13;
uint32_t wifi_check_time=600000L;
uint32_t wifi_initiate =0;
uint8_t wifi_reconnect_tries = 0;
//PZEM004Tv30 pzem(13, 15);
PZEM004Tv30 pzem(&Serial);
MedianFilter mfPower(10, 200);
struct EneMainData{
  uint8_t v; //volt
  float i;  //current
  float c;  //cospi
  uint16_t e; //current power
};
uint16_t prevPower=0;
uint8_t mqttOK=0;
uint8_t alarmPower=0;
EneMainData valori;
nodeRelay daiCorrente(powerPin); //usato x accendere la luce
nodeRelay luceSpia(ledPin); //usato come spia pulsante
void smartDelay(uint32_t ms);
void reconnect();
void setupWifi();
void checkConn();
void prepareData();
void playSound(const uint16_t* melody,const uint8_t* noteDurations);
void callback(char* topic, byte* payload, unsigned int length);
void sendThing(EneMainData dati);
void sendMySql(EneMainData dati);
uint8_t checkForUpdates(uint8_t versione);
#endif