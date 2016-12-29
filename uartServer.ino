// This code runs a webserver on the ESP8266 chip
// data that comes in on the serial port is buffered
// The root webpage displays the serial data back line-by-line
// The web page will refresh itself
// Not a very efficient serial-wifi bridge 
//  but the html is a good jumping off point for other things.
// Modified from ESP8266 library code
// Mike Holden 11/2016


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <string.h>
#include <mikepw.h> // this should be in the arduino sketch directory\Libraries\mikepw\

const char* ssid = MYSSID; // #defined in mikepw.h for github security
const char* password = MYWIFIPW; // easier to just do ssid = "theapyouwant" but not good for github security

#define MYBUFSIZE 1200
#define HEADFOOTSIZE 100
char serbuf[MYBUFSIZE]="Hi from esp8266!\r\n";
char htmlbuf[MYBUFSIZE+HEADFOOTSIZE];
int bi=0,bo=0,hlen,rbfirst,rblast,ii,jj;

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", serbuf);
  //Serial.print(serbuf);
  //digitalWrite(led, 0);

}

void handleRoot2() {
  // have to insert the html header, then unwrap the ring buffer, then add the footer
  // saved 100 bytes for the header-footer characters so keep track!
  // hlen is how long the header is (header initialized in setup)
  // buffer between hlen and 99 should be blank (footer space, used to copy too)
  jj=hlen; // empty byte after header
  bi=rbfirst;
  while (bi != rblast) {
    htmlbuf[jj]=serbuf[bi];
    jj++;
    if (jj>=MYBUFSIZE+HEADFOOTSIZE) {
      jj=hlen; // should not happen unless header is too big
    }
    bi++;
    if (bi>=MYBUFSIZE) {
      bi=0;
    }
  }
  htmlbuf[jj]='\0'; // end string
  strcat(htmlbuf,"</pre>"); // tack on footer
  server.send(200, "text/html", htmlbuf);
  //String lala="this is the bitter end";
  //server.sendContent(lala);
  Serial.println("sent2");
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

  // initialize web page buffer
  strcpy(htmlbuf,"<head>\r\n<meta http-equiv=\"refresh\" content=\"1\">\r\n</head>");
  strcat(htmlbuf,"<h1>Data</h1><pre>");
  hlen=strlen(htmlbuf);

  // set up ring buffer indices
  rbfirst = 0; // initial ring buffer position. Room for header/footer at beginning
  rblast = strlen(serbuf)-1; // last position with data.

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

  server.on("/", handleRoot2);

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
    serbuf[rblast]=Serial.read();
    bo=rblast; // one place before new rblast
    rblast++;
    if (rblast>=MYBUFSIZE) {
      // wrap buffer
      rblast=0;
    }
    if (rblast==rbfirst) { // buffer is full
      // delete a line to make room
      // start at rbfirst, search ring buffer
      // first '\n' will be new starting point (delete a line)
      ii=rbfirst;
      while ((serbuf[ii]!='\n')&&(ii!=bo)) {  // stop if find \n OR go thru whole buffer
        ii++;
        if (ii>=MYBUFSIZE) { // wrap index
          ii=0; // start of buffer after header
        }
      }
      rbfirst=ii+1; // new start: one past either the next \n or rblast
      if (rbfirst>=MYBUFSIZE) { // wrap
        rbfirst=0;
      }
    }


    
//    bo = strlen(serbuf); // how long
//    if (bo < MYBUFSIZE-6) { // room to add on
//      serbuf[bo]=Serial.read();
//      serbuf[bo+1]=0;
//    } else {
//      strcpy(serbuf,"<head>\r\n<meta http-equiv=\"refresh\" content=\"2\">\r\n</head>");
//      strcat(serbuf,"<h1>Data</h1><pre>");
//      //Serial.println("");
//      //Serial.print(bo);
//      //Serial.println("resetting buffer");
//    }
  }
}
