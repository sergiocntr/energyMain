#ifndef eneMain_h
#define eneMain_h
#include <Arduino.h>
#include <debugutils.h>
//#include <ESP8266Ping.h>
#include <myFunctions.h>
#include <nodeRelay.h>
#include <pin.h>
#include <MedianFilter.h>
#include <PZEM004Tv30.h>
PZEM004Tv30 pzem(13, 15);
MedianFilter mfPower(20, 200);
const uint16_t versione = 10;
struct EneMainData{
  uint8_t v; //volt
  float i;  //current
  float c;  //cospi
  uint16_t e; //current power
};
uint16_t prevPower=0;
EneMainData valori;
nodeRelay daiCorrente(powerPin); //usato x accendere la luce
nodeRelay luceSpia(ledPin); //usato come spia pulsante
void myinit();
void checkConn();
void smartDelay(unsigned long ms);
void reconnect();
void setup_wifi();
void playSound(const uint16_t* melody,const uint8_t* noteDurations);
void callbackcallback(char* topic, byte* payload, unsigned int length);
void sendThing(EneMainData dati);
void sendMySql(EneMainData dati);
#endif