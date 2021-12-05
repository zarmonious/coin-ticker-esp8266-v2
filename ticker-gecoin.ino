#include <stdio.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>


#define COIN1 "mobox"
#define COIN1_NAME "MBOX"
#define COIN2 "cosmos"
#define COIN2_NAME "ATOM"
 
/* Set these to your desired credentials. */
const char *ssid = "XXXXXXXXX";
const char *password = "XXXXXXXXX";

const char *host = "api.coingecko.com";
const int httpsPort = 443; //HTTPS= 443 and HTTP = 80
const String path = String("/api/v3/simple/price?ids=") + COIN1 + "," + COIN2 + "&vs_currencies=usd";
 
//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "33 C5 7B 69 E6 3B 76 5C 39 3D F1 19 3B 17 68 B8 1B 0A 1F D9";

char line1[17] = {};
LiquidCrystal_I2C lcd(0x27,16,2);

DynamicJsonDocument doc(255);
String line;
String jsonResponse;

float rawPrice = 0;

//=======================================================================
//                    Power on setup
//=======================================================================
void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
 
  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  lcd.init();
  lcd.backlight();
}
 
//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient
 
  Serial.println(host);
 
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTPS Connecting");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }
 
  Serial.print("requesting URL: ");
  Serial.println(host + path);
  
  httpsClient.print(
    "GET " + path + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Accept: application/json\r\n" +
    "Connection: close\r\n\r\n"
  );
 
  Serial.println("request sent");

  Serial.println("Response headers:");
  Serial.println("==========");
  while (httpsClient.connected()) {
    line = httpsClient.readStringUntil('\n');
    Serial.println(line); //Print response
    if (line == "\r") {
      break;
    }
  }
  Serial.println("==========\n");
 
  Serial.println("Body:");
  Serial.println("==========");
  int counter = 0;
  while(httpsClient.available()) {
    ++counter;
    line = httpsClient.readStringUntil('\n');
    Serial.println(line); //Print response
    if (counter == 2) {
      jsonResponse = line;
    }
  }
  Serial.println("==========\n");
  Serial.println("closing connection\n");

  Serial.println("JSON: " + jsonResponse);

  DeserializationError error = deserializeJson(doc, jsonResponse.c_str());
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }

  lcd.clear();
  
  rawPrice = doc[COIN1]["usd"];
  sprintf(line1, "%s: %.2f USD", COIN1_NAME, rawPrice);
  lcd.setCursor(0,0);
  lcd.print(line1);

  rawPrice = doc[COIN2]["usd"];
  sprintf(line1, "%s: %.2f USD", COIN2_NAME, rawPrice);
  lcd.setCursor(0,1);
  lcd.print(line1);
    
  delay(60000);  //GET Data at every 2 seconds
}
