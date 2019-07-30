//#define DEBUGMIO
#include <eneMain.h>
void callback(char* topic, byte* payload, unsigned int length) {
  DEBUG_PRINT("Ricevuto topic.");
  DEBUG_PRINT((String)topic);
  char miosegn=((char)payload[0]);
  DEBUG_PRINT((String)mioPayload);
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
  if(client.subscribe(eneTopic)) {DEBUG_PRINT("Subscrive ok.");}
  else {DEBUG_PRINT("Subscrive non funziona.");}
  client.subscribe(updateTopic);
  client.loop();
}
void sendThing(EneMainData dati) {
  StaticJsonBuffer<500> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["topic"] = "EneMain";
  //JSONencoder["acqua"] = dati.acquaTemp;
  JSONencoder["v"] = dati.v;
  JSONencoder["i"] = dati.i;
  JSONencoder["p"] = dati.p;
  JSONencoder["e"] = dati.e;
  char JSONmessageBuffer[500];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  yield();
  client.publish(eneTopic, JSONmessageBuffer,true);
  client.loop();
}
void setup() {
  daiCorrente.relay('1');
  luceSpia.relay('1');
  #ifdef DEBUGMIO
    Serial.begin(9600);
    delay(3000);
    DEBUG_PRINT("Booting!");
    DEBUG_PRINT("Versione: " + String(versione));
    setIP(ipEneMain,EneMainId);
  #else
    setIP(ipEneMain,EneMainId);
    Serial.begin(115200);
  #endif // DEBUG
  connectWiFi();
  yield();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(10);
  connectMQTT();
  reconnect();
  //pzem.setAddress(pzemip);
  wifi_check_time = 15000;
  wifi_reconnect_time=millis();
  if(client.state()) DEBUG_PRINT("Si si tu mi piaci. Versione " + String(versione));
}
void checkConn(){
  DEBUG_PRINT("Controllo WIFI");
    wifi_reconnect_time=millis();
    if (client.state()!=0) {  // se non connesso a MQTT
      DEBUG_PRINT("MQTT NON VA");
      mqtt_reconnect_tries++;
      connectWiFi();    //verifico connessione WIFI
      delay(100);
      connectMQTT();
      smartDelay(500);
      reconnect();
      wifi_check_time = 15000; //ogni 15 secondi
    }else {
      DEBUG_PRINT("MQTT OK");
      mqtt_reconnect_tries=0;
      wifi_check_time = 300000; //ogni 5 minuti
    }
    if (mqtt_reconnect_tries > 3) wifi_check_time = 1200000;  //venti minuti
}
void loop() {
  smartDelay(5000);
  if((millis() - wifi_reconnect_time) > wifi_check_time){ 
   checkConn();
  }
  valori.v = pzem.voltage();
  valori.i = pzem.current();
  valori.p = pzem.power();
  valori.e = pzem.energy();
  smartDelay(100);
  sendThing(valori);
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
  WiFiClient mySqlclient;
  if (mySqlclient.connect(host, httpPort))
  {
    //String s =String("GET /meteofeletto/EneMain_logger.php?gaspower=" + String(dati.power) +
    //+"&&pwd=" + webpass +
    //+"&&temp=" + String(dati.acquaTemp) +
    //+ " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    //yield();
   // mySqlclient.println(s);
    smartDelay(100);
    //mySqlclient.stop();
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