



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
Adafruit_MQTT_Publish hauntedHallway = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/HauntedHallway");

//RGB
#include <FastLED.h>
#define LED_PIN     D8
#define NUM_LEDS    10
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
CRGB leds[10];

//Sensor
#define sensor D7

//Relays
#define bigHead D6
#define funnyBones D5
#define speaker D0

//MP3 Player
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>
DFRobotDFPlayerMini myDFPlayer;
SoftwareSerial mySoftwareSerial(D4, D2);  //Pins for MP3 Player Serial (RX, TX)

//Globals for System
int trigger = 0;




// ***************************************
// *************** Setup *****************
// ***************************************


void setup() {
  
  //Wifi
  wifiSetup();
  Serial.println("Wifi Setup Complete");

  //Sensor
  pinMode(sensor, INPUT);

  //Relays
  pinMode(bigHead, OUTPUT);
  pinMode(funnyBones, OUTPUT);
  pinMode(speaker, OUTPUT);
  digitalWrite(bigHead, LOW);
  digitalWrite(funnyBones, LOW);
  digitalWrite(speaker, LOW);
  delay(200);
  digitalWrite(bigHead, HIGH);
  delay(500);
  digitalWrite(bigHead, LOW);
  digitalWrite(funnyBones, HIGH);
  delay(200);
  digitalWrite(funnyBones, LOW);
  Serial.println("Relay Setup Complete");
  
  //Initialize RGB
  FastLED.addLeds<WS2811, D2, RGB>(leds, 10).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(255);
  //delay(2000);
  for (int i = 0; i < 120; i++) {
    setPixel(i, 0, 255, 0);
  }
  delay(100);
  FastLED.show();
  delay(1000);
  FastLED.clear();
  delay(10);
  FastLED.show();
  Serial.println("RGB Setup Complete");

  //MP3  
  mySoftwareSerial.begin(9600);
  Serial.println("mp3");
  delay(1500);
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
  myDFPlayer.volume(25); //Volume 0-30
  myDFPlayer.EQ(0); //Equalization normal  
  delay(100);
  myDFPlayer.play(1);
  delay(20000);
  myDFPlayer.volume(20);
  char q = 1;
}




// ***************************************
// ************* Da Loop *****************
// ***************************************


void loop() {

  Serial.println("Loop");

  //Network Housekeeping
  ArduinoOTA.handle();
  Serial.println("OTA Done");
  hauntedHallway.publish(1);

  if(digitalRead(sensor) == HIGH){
    trigger = 1;
    //hauntedHallway.publish(1);
    Serial.println("SUCKERS DETECTED");
  }
  else{
    Serial.println("...");
  }

  //SHOWTIME BABY
  if (trigger == 1) {
    Serial.println("Trigger");
    delay(50);
    
    //Turn on Big Head
    delay(1500);
    digitalWrite(bigHead, HIGH);
    delay(5500);

    //Trigger Funny Bones and his Lights
    for (int i = 0; i < 10; i++) {
      setPixel(i, 0, 255, 0);
    }
    delay(100);
    FastLED.show();
    digitalWrite(funnyBones, HIGH);
    delay(200);
    digitalWrite(funnyBones, LOW);
    delay(500);

    //Start playing creepy sounds
    //hauntedHallway.publish(1);
    //char q = 1;
    //myDFPlayer.play(1);
    delay(3000);
    digitalWrite(speaker, HIGH);
    delay(250);
    digitalWrite(speaker, LOW);
    delay(15000);

    //Turn off everything
    digitalWrite(bigHead, LOW);
    for (int i = 0; i < 10; i++) {
      setPixel(i, 0, 0, 0);
    }
    delay(500);
    FastLED.show();
    myDFPlayer.stop();
    trigger = 0;
  }
  delay(100);
}




// ***************************************
// ********** Backbone Methods ***********
// ***************************************


void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}

void wifiSetup() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("HauntedHallway-Main");
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
