/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Example works with either Wired or WiFi Ethernet, define one of these values to 1, other to 0
#define USE_WIFI 0
#define USE_WIRED 1

#if USE_WIFI
#include <WiFi.h>
#elif USE_WIRED

//untuk debug , menggunakan port serial2, karena ketika upload program, Serial Monitor harus dimatikan. 
//dengan Serial2, maka untuk serial monitor bisa menggunakan USB2UART terpisah, dan serial monitor bisa tetap berjalan ketika upload program
//Serial2 menggunakan pinTX = GP8 dan pinRX = GP9


//The RP2040 provides two hardware-based UARTS with configurable pin selection.
//Serial1 is UART0, and Serial2 is UART1.


//PERHATIKAN CHIP SELECT INI adalah pin RP2040 yang dihubungkan ke CS pada modul eth
//#include <W5500lwIP.h> // Or W5100lwIP.h or ENC28J60lwIP.h
//Wiznet5500lwIP eth(1 /* chip select */); // or Wiznet5100lwIP or ENC28J60lwIP

#include <ENC28J60lwIP.h> // Or W5100lwIP.h or ENC28J60lwIP.h
ENC28J60lwIP eth(17 /* chip select */); // or Wiznet5100lwIP or ENC28J60lwIP

#endif

#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <StreamString.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK "your-password"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

WebServer server(80);

const int led = LED_BUILTIN;

void handleRoot() {
  static int cnt = 0;
  digitalWrite(led, 1);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  StreamString temp;
  temp.reserve(500); // Preallocate a large chunk to avoid memory fragmentation
  temp.printf("<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>" BOARD_NAME " Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from the " BOARD_NAME "!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Free Memory: %d</p>\
    <p>Page Count: %d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>", hr, min % 60, sec % 60, rp2040.getFreeHeap(), ++cnt);
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void drawGraph() {
  String out;
  out.reserve(2600);
  char temp[70];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial2.begin(115200);

  delay(500);
  Serial2.print("Set Network: ");

#if USE_WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial2.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial2.print(".");
  }

  Serial2.println("");
  Serial2.print("Connected to ");
  Serial2.println(ssid);

  Serial2.print("IP address: ");
  Serial2.println(WiFi.localIP());
#elif USE_WIRED

//------------------------------------ETHERNET----------------------------------------
  // Set up SPI pinout to match your HW

  /*
  SPI.setRX(0);
  SPI.setCS(1);
  SPI.setSCK(2);
  SPI.setTX(3);
  */

  
  SPI.setRX(16); //GP16
  SPI.setCS(17); //GP17 
  SPI.setSCK(18); //GP18
  SPI.setTX(19); //GP19
  
  
  //agar tidak menindih UART0, maka pindah pin SPI nya
  /*
  SPI.setRX(4); //GP4  -----  ENC_SO
  SPI.setCS(5); //GP5  -----  ENC_CS 
  SPI.setSCK(2); //GP2  ----- ENC_SCK
  SPI.setTX(3); //GP3  -----  ENC_SI     
  */

  Serial2.println("Wired: ");
  
  // Start the Ethernet port
  if (!eth.begin()) {
    Serial2.println("No wired Ethernet hardware detected. Check pinouts, wiring.");
    while (1) {
      delay(1000);
    }
  }

  Serial2.println("eth: ");
 


  // Wait for connection
  while (eth.status() != WL_CONNECTED) {
    delay(500);
    Serial2.print(".");
  }
  Serial2.print("IP address: ");
  Serial2.println(eth.localIP());
#endif

//------------------------------------ETHERNET----------------------------------------



  if (MDNS.begin("picow")) {
    Serial2.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial2.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
