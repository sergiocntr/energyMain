//#define DEBUGMIO
//NON VA SE COLLEGATO......
#include <eneMain.h>
void blinkLed(uint8_t volte){
  for (uint8_t i = 0; i < volte; i++)
  {
    digitalWrite(LED_BUILTIN,LOW);
    delay(250);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(250);
  }
}
void adessoDormo(){
  client.disconnect();
  delay(10);
  WiFi.disconnect(true);
  delay(10);
  WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  WiFi.forceSleepBegin();
  delay(10);
  daiCorrente.relay('1');
  luceSpia.relay('1');
  //system_deep_sleep_set_option(2);
  //system_deep_sleep_instant(180000*1000);
  delay(180000);
  setupWifi();
}
void setupWifi(){
  WiFi.begin(ssid, password);
  delay(10);
  //handleCrash();
  wifi_initiate = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - wifi_initiate) > 5000L) {
      adessoDormo();
      //dopo c'e' il restart
    }
    delay(500);
  }
  blinkLed(2);
  String clientId = String("enemon");
  clientId += String(random(0xffff), HEX);
  delay(10);
  //randomSeed(micros());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(100);
  client.connect(clientId.c_str(),mqttUser,mqttPass);
  delay(10);
  wifi_initiate = millis();
  while (!client.connected()) {
    if ((millis() - wifi_initiate) > 5000L) {
      adessoDormo();
      //dopo c'e' il restart
    }
    delay(500);
  } 
  reconnect();
  if(mqttOK){blinkLed(3);}
  //DS18B20.begin();
  delay(500);
  prepareData();
  delay(10);
}
void setup() {
  WiFi.mode(WIFI_OFF);
  delay(10);
  //handleCrash();
  daiCorrente.relay('1');
  luceSpia.relay('1');
  Serial.swap();
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  WiFi.disconnect(true);    // Delete SDK wifi config
  delay(200);
  WiFi.setOutputPower(17);        // 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW
  delay(10);
  WiFi.hostname("enemon");      // DHCP Hostname (useful for finding device for static lease)
  WiFi.mode(WIFI_STA);
  WiFi.forceSleepWake();
  delay(10);
  WiFi.config(ipEneMain, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  delay(10);
  setupWifi();
  
}
void callback(char* topic, byte* payload, unsigned int length) {
  //DEBUG_PRINT("Ricevuto topic.");
  //DEBUG_PRINT((String)topic);
  char miosegn=((char)payload[0]);
  //DEBUG_PRINT((String)miosegn);
  if(strcmp(topic,updateTopic) == 0){
    delay(10);
    if(miosegn=='2'){
      mqttOK=client.publish(logTopic, "aggiornamento EneMain");
      delay(10);
      uint8_t miocheck = 0;
      miocheck = checkForUpdates(versione);
      switch(miocheck) {
        case 1:
          mqttOK=client.publish(logTopic, "HTTP_UPDATE_FAIL"); 
          break;
        case 2:
          mqttOK=client.publish(logTopic, "HTTP_UPDATE_NO_UPDATES");
          break;
        case 0:
          mqttOK=client.publish(logTopic, "Already on latest version" );
          break;
        default:
          //mqttOK=client.publish(logTopic, "Firmware version check failed, got HTTP response code " + String(miocheck));
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
      
      daiCorrente.relay(miosegn); 
    }
    else if(miosegn=='1'){
    }
  }
  else if (strcmp(topic, systemTopic) == 0) 
  {
    String s="SYS" + String(payload[0]);
    client.publish(teleTopic,s.c_str());
    delay(10);
    if(payload[0] == 48) {
      blinkLed(3);
      delay(10);
      adessoDormo();
    }
  }
  smartDelay(100);
}
void reconnect() {
  smartDelay(50);
  client.publish(logTopic, "EneMain connesso");
  delay(10);
  client.subscribe(eneTopic); //{DEBUG_PRINT("Subscrive ok.");}
  delay(10);
  client.subscribe(systemTopic);
  delay(10);
  mqttOK = client.subscribe(updateTopic);
  smartDelay(50);
}

void prepareData(){
  #ifdef DEBUGMIO 
    DEBUG_PRINT("DATI FITTIZI!!!!");
    valori.v = 220;
    valori.i = 1.2;
    valori.c = 1.54;
    valori.e = 800;
    delay(10);
    sendMySql(valori);
    smartDelay(100);
    sendThing(valori);
    smartDelay(100);
  #else
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
    if((nowPower > (prevPower+8)) || (nowPower < (prevPower-8)))
    {
      valori.v = uint8_t(pzem.voltage());
      valori.i = roundf(pzem.current() * 100) / 100;
      valori.c = roundf(pzem.pf() * 100) / 100;
      valori.e = nowPower;
      delay(10);
      prevPower=nowPower;
      delay(10);
      sendMySql(valori);
      smartDelay(100);
      sendThing(valori);
      smartDelay(100);
    }
  #endif
}
void loop(){
  if((millis() - wifi_initiate) > wifi_check_time){ 
    wifi_initiate=millis();
    if (mqttOK == 0) {
      prepareData();
      adessoDormo();
      //dopo c'e' il restart
    }
  }
  prepareData();
  smartDelay(3000);
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
    #ifdef DEBUGMIO 
    DEBUG_PRINT("SPEDISCO SQL!");
    #endif
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
    //mywifi.stop();
    //smartDelay(100);
  }
}
void sendThing(EneMainData dati) {
  DynamicJsonDocument doc(500);
  doc["topic"] = "EneMain";
  doc["v"] = dati.v;
  doc["i"] = dati.i;
  doc["c"] = dati.c;
  doc["e"] = dati.e;
  char buffer[500];
  size_t n = serializeJson(doc, buffer);
  mqttOK = client.publish(eneTopic, buffer,n);
  smartDelay(100);
  
}
void smartDelay(uint32_t ms){
  uint32_t start = millis();
  do
  {
    client.loop();
    delay(10);
  } while (millis() - start < ms);
}
uint8_t checkForUpdates(uint8_t FW_VERSION) {
  //Serial.println( "Checking for firmware updates." );
  //Serial.print( "Current firmware version: " );
  //Serial.println( FW_VERSION );
  uint8_t check=0;
  String fwURL = String( fwUrlBase );
  fwURL.concat( "cald" ); //QUI DA VEDERE CON PERCORSO SERVER
  String fwVersionURL = fwURL;
  fwVersionURL.concat( "/version.php" );
  //Serial.print( "Firmware version URL: " );
  //Serial.println( fwVersionURL );

  String fwImageURL = fwURL;
  fwImageURL.concat( "/firmware.bin" );
  //Serial.print( "Firmware  URL: " );
  //Serial.println( fwImageURL );
  //#ifdef HTTP_ON
  WiFiClient myUpdateConn;
  HTTPClient http;
  http.begin( myUpdateConn, fwVersionURL );
  int httpCode = http.GET();
  if( httpCode == 200 ) {
    String newFWVersion = http.getString();
    int newVersion = newFWVersion.toInt();
    if( newVersion > FW_VERSION ) {
    //  //Serial.println( "Preparing to update" );
      t_httpUpdate_return ret = ESPhttpUpdate.update(myUpdateConn , fwImageURL );
      switch(ret) {
        case HTTP_UPDATE_FAILED:
          check=1;
          //Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          check=2;
          //Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          //Serial.println("[update] Update ok."); // may not called we reboot the ESP
          break;
      }
    }
    else {
      check=0;
      //Serial.println( "Already on latest version" );
    }
  }
  else {
    //Serial.print( "Firmware version check failed, got HTTP response code " );
    //Serial.println( httpCode );
    check= httpCode;
  }
  http.end();
  myUpdateConn.stop();
  return check;
}