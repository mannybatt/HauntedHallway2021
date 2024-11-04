



// ***************************************
// ********** Global Variables ***********
// ***************************************


//Globals for Wifi Setup and OTA
#include <credentials.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//WiFi Credentials
#ifndef STASSID
#define STASSID "your_ssid"
#endif
#ifndef STAPSK
#define STAPSK  "your_password"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;

//MQTT
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#ifndef AIO_SERVER
#define AIO_SERVER      "your_MQTT_server_address"
#endif
#ifndef AIO_SERVERPORT
#define AIO_SERVERPORT  0000 //Your MQTT port
#endif
#ifndef AIO_USERNAME
#define AIO_USERNAME    "your_MQTT_username"
#endif
#ifndef AIO_KEY
#define AIO_KEY         "your_MQTT_key"
#endif
#define MQTT_KEEP_ALIVE 150
unsigned long previousTime;

//Initialize and Subscribe to MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe hauntedHallway = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/HauntedHallway");

//MP3 Player
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>
DFRobotDFPlayerMini myDFPlayer;
SoftwareSerial mySoftwareSerial(D4, D2);  //Pins for MP3 Player Serial (RX, TX)

//Sensor
#define sensor D7
#define fogOn D5
#define fogOff D6

//Globals for System
int trigger = 0;




// ***************************************
// *************** Setup *****************
// ***************************************


void setup() {

  //Relays
  pinMode(fogOn, OUTPUT);
  pinMode(fogOff, OUTPUT);
  digitalWrite(fogOn, LOW);
  digitalWrite(fogOff, LOW);

  //Wifi
  wifiSetup();
  mqtt.subscribe(&hauntedHallway);
  MQTT_connect();
  delay(2000);  

  //Sensor
  pinMode(sensor, INPUT);

  //MP3
  mySoftwareSerial.begin(9600);
  if (!myDFPlayer.begin(mySoftwareSerial)) //Is DfPlayer ready?
  {
    Serial.println(F("Not initialized:"));
    Serial.println(F("1. Check the DFPlayer Mini connections"));
    Serial.println(F("2. Insert an SD card"));
    while(true);
  }
  Serial.println();
  Serial.println("DFPlayer Mini module initialized!");
  myDFPlayer.setTimeOut(500); //Timeout serial 500ms
  myDFPlayer.volume(0); //Volume 0-30
  myDFPlayer.EQ(0); //Equalization normal
  char q = 1;
  delay(100);
  
  digitalWrite(fogOn, HIGH);
  delay(2000);
  digitalWrite(fogOn, LOW);
  delay(3000);
  digitalWrite(fogOff, HIGH);
  delay(2000);
  digitalWrite(fogOff, LOW);
  
  myDFPlayer.play(q);
  delay(3000);
  myDFPlayer.volume(30);
}




// ***************************************
// ************* Da Loop *****************
// ***************************************


void loop() {

  //Network Housekeeping
  ArduinoOTA.handle();
  //MQTT_connect();

  //State Manager
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10))) {
    Serial.println("Subscription Recieved");
    uint16_t value = atoi((char *)hauntedHallway.lastread);
    if(value == 1){
      trigger = 1;
    }
    else if(value == 0){
      //lol
    }
    delay(1);
  }

   if(digitalRead(sensor) == HIGH){
    trigger = 1;
    //hauntedHallway.publish(1);
    Serial.println("SUCKERS DETECTED");
  }
  else{
    Serial.println("...");
  }

  //Play Audio Triggers
  if (trigger == 1) {
    delay(3000);
    digitalWrite(fogOn, HIGH);
    delay(1500);
    digitalWrite(fogOn, LOW);
    delay(15000);
    digitalWrite(fogOff, HIGH);
    delay(1500);
    digitalWrite(fogOff, LOW);
    delay(2000);
    char q = 1;
    myDFPlayer.play(q);
    trigger = 0;
    Serial.println("Deezy");
    trigger = 0;
  }
  delay(100);
}




// ***************************************
// ********** Backbone Methods ***********
// ***************************************


void wifiSetup() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("HauntedHallway-Secondary");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    //Serial.println("Connected");
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      //while (1);
      Serial.println("Wait 10 min to reconnect");
      delay(600000);
    }
  }
  Serial.println("MQTT Connected!");
}
