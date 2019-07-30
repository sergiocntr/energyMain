#ifndef eneMain_h
#define eneMain_h
#include <Arduino.h>
#include <debugutils.h>
#include <ESP8266Ping.h>
#include <myFunctions.h>
#include <nodeRelay.h>
#include <pin.h>
#include <PZEM004Tv30.h>
PZEM004Tv30 pzem(13, 15);
//PZEM004T pzem(&Serial);
//IPAddress pzemip(192,168,1,1);
//bool pzemrdy = false;
const uint16_t versione =5;
struct EneMainData{
  float v ;
  float i;
  float p;
  float e;
};
EneMainData valori;
nodeRelay daiCorrente(powerPin); //usato x accendere la luce
nodeRelay luceSpia(ledPin); //usato come spia pulsante
void checkConn();
void smartDelay(unsigned long ms);
void reconnect();
void setup_wifi();
void playSound(const uint16_t* melody,const uint8_t* noteDurations);
void callbackcallback(char* topic, byte* payload, unsigned int length);
void sendThing(EneMainData dati);
void sendMySql(EneMainData dati);
#endif