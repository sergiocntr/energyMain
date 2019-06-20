#ifndef eneMain_h
#define eneMain_h
#include <Arduino.h>
#include <debugutils.h>
#include <ESP8266Ping.h>
#include <myFunctions.h>
#include <nodeRelay.h>
#include <MedianFilter.h>
#include <pin.h>
MedianFilter mfPower(31, 0);
const uint16_t versione =2;
struct EneMainData{
  uint16_t power;
};
nodeRelay daiCorrente(powerPin); //usato x accendere la luce
nodeRelay luceSpia(ledPin); //usato come spia pulsante
typedef struct EneMainData datiEneMain;
datiEneMain valori;
void smartDelay(unsigned long ms);
void reconnect();
void setup_wifi();
void playSound(const uint16_t* melody,const uint8_t* noteDurations);
void callbackcallback(char* topic, byte* payload, unsigned int length);
void sendThing(datiEneMain dati);
void sendMySql(datiEneMain dati);
#endif