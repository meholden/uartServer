#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <string.h>
#include <mikepw.h> // this should be in the arduino sketch directory\Libraries\mikepw\

const char* ssid = MYSSID; // #defined in mikepw.h for github security
const char* password = MYWIFIPW; // easier to just do ssid = "theapyouwant" but not good for github security

#define MYBUFSIZE 600
char serbuf[MYBUFSIZE]="Hi from esp8266!\r\n";
int bi=0,bo=0;

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", serbuf);
  //Serial.print(serbuf);
  //digitalWrite(led, 0);

}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Hello");
  strcpy(serbuf,"<head>\r\n<meta http-equiv=\"refresh\" content=\"5\">\r\n</head>");
  strcat(serbuf,"<h1>Go Data</h1><pre>");
  bo=strlen(serbuf);
  Serial.print("\r\nBuf init to:");
  Serial.println(bo);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  if (Serial.available() > 0) {
    // get incoming byte:
    //strcat(serbuf,(char)Serial.read());
    bo = strlen(serbuf); // how long
    if (bo < MYBUFSIZE-6) { // room to add on
      serbuf[bo]=Serial.read();
      serbuf[bo+1]=0;
    } else {
      strcpy(serbuf,"<head>\r\n<meta http-equiv=\"refresh\" content=\"2\">\r\n</head>");
      strcat(serbuf,"<h1>Data</h1><pre>");
      //Serial.println("");
      //Serial.print(bo);
      //Serial.println("resetting buffer");
    }
  }
}
