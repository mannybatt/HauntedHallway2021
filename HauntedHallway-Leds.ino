



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

//RGB
#include <FastLED.h>
#define LED_PIN     D2
#define NUM_LEDS    120
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100

//States
int workLights = 0;
int manualRGB = 0;
int ledR = 100;
int ledG = 100;
int ledB = 20;
int initialButtonState1 = -1;
int initialButtonState2 = -1;
#define workLightsButton2 D6
#define workLightsButton1 D5

int handsDirection = 1;
int handsCount = 0;
int handsR = 0;




// ***************************************
// *************** Setup *****************
// ***************************************


void setup() {

  //Initialize Serial
  Serial.begin(115200);
  Serial.println("Booting");

  //Initialize RGB
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  delay(2000);
  for (int i = 0; i < 120; i++) {
    setPixel(i, 255, 255, 255);
    delay(10);
    FastLED.show();
  }
  FastLED.show();

  //Mercy Time!
  for (int i = 0; i < 60; i++) {
    Serial.println(i);
    //delay(1000);
  }

  //Initialize button
  pinMode(workLightsButton1, INPUT_PULLUP);
  pinMode(workLightsButton2, INPUT_PULLUP);
  delay(10);
  initialButtonState1 = digitalRead(workLightsButton1);
  initialButtonState2 = digitalRead(workLightsButton2);
}




// ***************************************
// ************* Da Loop *****************
// ***************************************


void loop() {

  //Network Housekeeping
  ArduinoOTA.handle();

  //Check for work lights mode
  checkWorkLights();

  //Work Lights are on, nothing changes until we leave work lights mode
  if (workLights == 1) {
    for (int i = 0; i < NUM_LEDS; i++) {
      setPixel(i, 255, 255, 255);
      delay(10);
      FastLED.show();
    }
    Serial.println("Work Lights");
    delay(100);
  }

  //Work Lights are off, allow Manual RGB mode
  else {
    //Breathing Red for Hands
    for (int i = 0; i < 34; i++) {
      if (handsDirection == 1) {
        if (handsCount % 4 == 0) {
          handsR++;
        }
        handsCount++;
      }
      else if (handsDirection == 0) {
        if (handsCount % 4 == 0) {
          handsR--;
        }
        handsCount--;
      }
      if (handsCount == 1020) {
        handsDirection = 0;
      }
      else if (handsCount == 0) {
        handsDirection = 1;
      }
      Serial.print("handsR - ");
      Serial.println(handsR);
      setPixel(i, handsR, 0, 0);
    }

    //Lightning Flash for FunnyBones
    for (int i = 34; i < 52; i++) {
      setPixel(i, 150, 0, 255);
    }

    for (int i = 52; i < 150; i++) {
      setPixel(i, 255, 100, 0);
    }//FireFly Effect

    delay(10);
    FastLED.show();
    //ALSO make wind louder and Scream louder too
  }
}




// ***************************************
// ********** Backbone Methods ***********
// ***************************************


//If a btn press occurs, change workLights
void checkWorkLights() {

  //Read Button States
  int z = digitalRead(workLightsButton1);
  delay(10);
  int y = digitalRead(workLightsButton2);
  delay(10);

  //Button 1
  if (z != initialButtonState1) {
    //Turn on
    if (workLights == 0) {
      workLights = 1;
      //whiteHall();
    }
    //Turn off
    else if (workLights == 1) {
      workLights = 0;
      //blackHall();
    }
    initialButtonState1 = z;
  }

  //Button 2
  else if (y != initialButtonState2) {
    //Turn on
    if (workLights == 0) {
      workLights = 1;
      //whiteHall();
    }
    //Turn off
    else if (workLights == 1) {
      workLights = 0;
      //blackHall();
    }
    initialButtonState2 = y;
  }
}

void whiteHall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    setPixel(i, 255, 255, 255);
    delay(10);
    FastLED.show();
  }
}

void blackHall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    setPixel(i, 0, 0, 0);
    delay(10);
    FastLED.show();
  }
}

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
  ArduinoOTA.setHostname("HauntedHallway-LEDs");
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
