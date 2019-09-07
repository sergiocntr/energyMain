#define DEBUGMIO
#include <eneMain.h>
void callback(char* topic, byte* payload, unsigned int length) {
  DEBUG_PRINT("Ricevuto topic.");
  DEBUG_PRINT((String)topic);
  char miosegn=((char)payload[0]);
  DEBUG_PRINT((String)miosegn);
  if(strcmp(topic,updateTopic) == 0){
    delay(10);
    if(miosegn=='2'){
      send(logTopic, "aggiornamento EneMain");
      delay(10);
      uint8_t miocheck = checkForUpdates(versione);
      switch(miocheck) {
        case 1:
          send(logTopic, "HTTP_UPDATE_FAIL"); 
          break;
        case 2:
          send(logTopic, "HTTP_UPDATE_NO_UPDATES");
          break;
        case 0:
          send(logTopic, "Already on latest version" );
          break;
        default:
          send(logTopic, "Firmware version check failed, got HTTP response code " + String(miocheck));
          break;
      }
    }
  }
  else if (strcmp(topic, eneTopic) == 0) 
  {
    if(miosegn=='0'){
      unsigned long mytime = millis();
      // notes in the melody:
      const uint16_t melody[] = {
        NOTE_B4, NOTE_E4, NOTE_FS4, NOTE_G4, NOTE_E4
      };
      const uint8_t noteDurations[] = {
        4, 4, 4, 4, 4
      };
      //while((millis() - mytime) < 30000  ){
      while(Ping.ping("192.168.1.100")){
        playSound(melody,noteDurations);
        delay(3000);
        if((millis() - mytime) > 60000) {
          send(logTopic, "EnePingTimeout");
          return;
        }
      }
      daiCorrente.relay(miosegn); 
    }
  }
  smartDelay(100);
}
void reconnect() {
  client.publish(logTopic, "NodeMCU EneMain connesso");
  client.subscribe(eneTopic); //{DEBUG_PRINT("Subscrive ok.");}
  //else {DEBUG_PRINT("Subscrive non funziona.");}
  client.subscribe(updateTopic);
  smartDelay(100);
  mqtt_reconnect_tries = 0;
}
void setup() {
  daiCorrente.relay('1');
  luceSpia.relay('1');
  #ifdef DEBUGMIO
    Serial.begin(9600);
    delay(3000);
    DEBUG_PRINT("Booting!");
    DEBUG_PRINT("Versione: " + String(versione));
    yield();
  #else
    
    //Serial.begin(115200);
  #endif // DEBUG
  myinit();
}
void myinit(){
  DEBUG_PRINT("entro miinit");
  setIP(ipEneMain,EneMainId);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(20);
  for (int i = 0; i < 10; i++)
  {
    uint16_t nowPower = uint16_t(pzem.power());
    mfPower.in(nowPower);
    nowPower=mfPower.out();
    delay(2000);
  }
  mqtt_reconnect_tries = 1;
  checkConn();
}
void loop() {
  
  if((millis() - wifi_reconnect_time) > wifi_check_time){ 
    if(mqtt_reconnect_tries>3)
    {
      DEBUG_PRINT("caso 1: " + String(mqtt_reconnect_tries));
      delay(20);
      myinit();
    }else checkConn();
  }
  uint16_t nowPower = uint16_t(pzem.power());
  mfPower.in(nowPower);
  nowPower=mfPower.out();
  DEBUG_PRINT("valore prec: " + String(prevPower));
  yield();
  DEBUG_PRINT("valore att: " + String(nowPower));
  yield();
  if((nowPower > (prevPower+8)) || (nowPower < (prevPower-8))){
    DEBUG_PRINT("devo spedire i dati");
    
    valori.v = uint8_t(pzem.voltage());
    valori.i = roundf(pzem.current() * 100) / 100;
    valori.c = roundf(pzem.pf() * 100) / 100;
    valori.e = nowPower;
    prevPower=nowPower;
    
    if(mqtt_reconnect_tries==0)
    {
      DEBUG_PRINT("mqtt retries ok!");
      yield();
      sendMySql(valori);
      smartDelay(100);
      sendThing(valori);
      smartDelay(100);
    }
  } 
  smartDelay(2000);
}
void playSound(const uint16_t* melody,const uint8_t* noteDurations){
  // iterate over the notes of the melody:
  for (uint8_t thisNote = 0; thisNote <= sizeof(melody); thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    if(luceSpia.relayState()) luceSpia.relay('0');
    else luceSpia.relay('1');
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(buzzerPin, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    smartDelay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(buzzerPin);
  }
}
void sendMySql(EneMainData dati){
    smartDelay(100);
    String s =String("GET /meteofeletto/power_logger.php?enepower=" + String(dati.e) +
    +"&&pwd=" + webpass +
    +"&&volt=" + String(dati.v) +
    +"&&cospi=" + String(dati.c) +
    +"&&curr=" + String(dati.i) +
    + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    smartDelay(100);
    DEBUG_PRINT(s);
    //WiFiClient mySqlclient;
    if (c.connect(host, httpPort))
  {
    DEBUG_PRINT("ok mysql connesso")
    c.println(s);
    smartDelay(100);
    
  }else
  {
    DEBUG_PRINT("MYSQL FAIL!!!!!")
  }
  
}
void sendThing(EneMainData dati) {
  StaticJsonBuffer<500> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["topic"] = "EneMain";
  //JSONencoder["acqua"] = dati.acquaTemp;
  JSONencoder["v"] = dati.v;
  JSONencoder["i"] = dati.i;
  JSONencoder["c"] = dati.c;
  JSONencoder["e"] = dati.e;
  char JSONmessageBuffer[500];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  yield();
  client.publish(eneTopic, JSONmessageBuffer,true);
  client.loop();
  //HTTPClient http;
  //http.begin(espClient,post_serverJSON);
	//httpResponseCode = http.PUT(s);
  //JSONencoder.
}
void checkConn(){
  DEBUG_PRINT("Controllo WIFI");
  wifi_reconnect_time=millis();
  for (int i = 0; i < 4; i++)
  {
    smartDelay(100);
    if(connectWiFi()==0) {
      DEBUG_PRINT("wifi ok!");
      delay(100);
      if(connectMQTT()==0){
        DEBUG_PRINT("mqtt ok!");
        wifi_check_time = 300000; //ogni 5 minuti
        //smartDelay(500);
        if (mqtt_reconnect_tries > 0)
        {
          reconnect();
        }
        else{
          break;
        }
      } 
      else {
        DEBUG_PRINT("MQTT FAIL!");
        mqtt_reconnect_tries++;
        //wifi_check_time = 30000; //ogni 30 secondi
      }
    }
    else 
    {
      DEBUG_PRINT("WIFI FAIL!");
      mqtt_reconnect_tries++;
    }
    delay(1000);
  }
  if (mqtt_reconnect_tries > 3){
      wifi_check_time = 1200000;  //venti minuti
      DEBUG_PRINT("MQTT TIMEOUT!");
  } 
}
void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do
  {
    client.loop();
    delay(10);
  } while (millis() - start < ms);
}