//#define DEBUGMIO
//NON VA SE COLLEGATO......
#include <eneMain.h>
void setup() {
  WiFi.mode(WIFI_OFF);
  delay(10);
  handleCrash();
  daiCorrente.relay('1');
  luceSpia.relay('1');
  Serial.swap();
  //Serial.begin(9600);
  #ifdef DEBUGMIO
    delay(3000);
    DEBUG_PRINT("Booting!");
    DEBUG_PRINT("Versione: " + String(versione));
    delay(10);
  #endif 
  setIP(ipEneMain,EneMainId);
  delay(10);
  int8_t checkmio=0;
  checkmio = connectWiFi();
  if(checkmio==0) 
  {
    sendCrash();
    DEBUG_PRINT("WIFI OK!");
  }//else DEBUG_PRINT("WIFI KAPUTT!");
  delay(10);
  randomSeed(micros());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  String clientId = String(mqttID);
  clientId += String(random(0xffff), HEX);
  delay(100);
  if(!client.connected()){
    //checkmio =connectMQTT();
    if(connectMQTT()) 
    {
      DEBUG_PRINT("MQTT OK!");
      smartDelay(500);
      reconnect();
    }
  }
  wifi_reconnect_time=millis();
}
void callback(char* topic, byte* payload, unsigned int length) {
  //DEBUG_PRINT("Ricevuto topic.");
  //DEBUG_PRINT((String)topic);
  char miosegn=((char)payload[0]);
  //DEBUG_PRINT((String)miosegn);
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
      uint32_t mytime = millis();
      // notes in the melody:
      const uint16_t melody[] = {
        NOTE_B4, NOTE_E4, NOTE_FS4, NOTE_G4, NOTE_E4
      };
      const uint8_t noteDurations[] = {
        4, 4, 4, 4, 4
      };
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
    else if(miosegn=='1'){
    }
  }
  smartDelay(100);
}
void reconnect() {
  smartDelay(50);
  client.publish(logTopic, "EneMain connesso");
  client.subscribe(eneTopic); //{DEBUG_PRINT("Subscrive ok.");}
  client.subscribe(updateTopic);
  smartDelay(50);
}
void checkConn(){
  DEBUG_PRINT("Controllo WIFI");
  wifi_reconnect_time=millis();
  delay(100);
  if(!client.connected()) {  // se non connesso a MQTT
    DEBUG_PRINT("MQTT NON VA");
    mqtt_reconnect_tries++;
    connectWiFi();    //verifico connessione WIFI
    delay(100);
    if(connectMQTT()) 
    {
      DEBUG_PRINT("MQTT OK!");
      smartDelay(500);
      reconnect();
    }
    wifi_check_time = 5000; //cinque secondi
  }else {
    DEBUG_PRINT("MQTT OK");
    mqtt_reconnect_tries=0;
    wifi_check_time = 60000; //ogni 5 minuti
  }
}
void prepareData(){
  uint16_t nowPower = uint16_t(pzem.power());
  if(nowPower>3400){
    if(alarmPower==0)
    {
      alarmPower=1;
      String s="MP: " + String(nowPower);
      client.publish(teleTopic,s.c_str());
    }
  }else alarmPower=0;
  mfPower.in(nowPower);
  nowPower=mfPower.out();
  DEBUG_PRINT("valore prec: " + String(prevPower));
  delay(10);
  DEBUG_PRINT("valore att: " + String(nowPower));
  if((nowPower > (prevPower+8)) || (nowPower < (prevPower-8)))
  {
    DEBUG_PRINT("devo spedire i dati");
    valori.v = uint8_t(pzem.voltage());
    delay(10);
    valori.i = roundf(pzem.current() * 100) / 100;
    valori.c = roundf(pzem.pf() * 100) / 100;
    delay(10);
    valori.e = nowPower;
    prevPower=nowPower;
    delay(10);
    sendMySql(valori);
    smartDelay(100);
    sendThing(valori);
    smartDelay(100);
  }
}
void loop(){
  smartDelay(2000);
  if((millis() - wifi_reconnect_time) > wifi_check_time){ 
    checkConn();
  }
  if (mqtt_reconnect_tries > 2){
    offLine=1;
    wifi_check_time = 300000;  //cinque minuti
  }
  if(mqtt_reconnect_tries==0)
  {
    prepareData();
  } 
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
  if (mywifi.connect(host, httpPort))
  {
    String s =String("GET /meteofeletto/power_logger.php?enepower=" + String(dati.e) +
    +"&&pwd=" + webpass +
    +"&&volt=" + String(dati.v) +
    +"&&cospi=" + String(dati.c) +
    +"&&curr=" + String(dati.i) +
    + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    smartDelay(100);
    mywifi.println(s);
    smartDelay(100);
    mywifi.stop();
    smartDelay(100);
  }
}
void sendThing(EneMainData dati) {
  StaticJsonBuffer<500> JSONbuffer;
  smartDelay(100);
  JsonObject& JSONencoder = JSONbuffer.createObject();
  smartDelay(100);
  JSONencoder["topic"] = "EneMain";
  smartDelay(100);
  JSONencoder["v"] = dati.v;
  JSONencoder["i"] = dati.i;
  smartDelay(100);
  JSONencoder["c"] = dati.c;
  JSONencoder["e"] = dati.e;
  smartDelay(100);
  char JSONmessageBuffer[500];
  smartDelay(100);
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  smartDelay(100);
  client.publish(eneTopic, JSONmessageBuffer);
  smartDelay(100);
  //HTTPClient http;
  //http.begin(espClient,post_serverJSON);
	//httpResponseCode = http.PUT(s);
  //JSONencoder.
}
void smartDelay(uint32_t ms){
  uint32_t start = millis();
  do
  {
    client.loop();
    delay(10);
  } while (millis() - start < ms);
}