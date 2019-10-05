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
#include <crash.h>
WiFiClient mywifi;
//PZEM004Tv30 pzem(13, 15);
PZEM004Tv30 pzem(&Serial);
MedianFilter mfPower(10, 200);
const uint16_t versione = 12;
struct EneMainData{
  uint8_t v; //volt
  float i;  //current
  float c;  //cospi
  uint16_t e; //current power
};
uint16_t prevPower=0;
uint8_t offLine=0;
uint8_t alarmPower=0;
EneMainData valori;
nodeRelay daiCorrente(powerPin); //usato x accendere la luce
nodeRelay luceSpia(ledPin); //usato come spia pulsante
void smartDelay(uint32_t ms);
void reconnect();
void setup_wifi();
void checkConn();
void prepareData();
void playSound(const uint16_t* melody,const uint8_t* noteDurations);
void callback(char* topic, byte* payload, unsigned int length);
void sendThing(EneMainData dati);
void sendMySql(EneMainData dati);
#endif