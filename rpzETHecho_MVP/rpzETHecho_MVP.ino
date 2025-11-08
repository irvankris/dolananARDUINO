/*
 Web client Raspberry Pi Pico (or other rp2040 boards)
 and Arduino Ethernet library
 
 This sketch connects to a test website (httpbin.org)
 and try to do a GET request, the output is printed
 on Serial
 
 by Renzo Mischianti <www.mischianti.org>
 
 https://www.mischianti.org
 
GUNAKAN BOARD DARI
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

di arduino Macbook
Arduino IDO > Settings > Additional Board Manager URLS 


 */
 
#include <SPI.h>
#include <Ethernet.h>
//#include <EthernetLarge.h>
#include <SSLClient.h>
#include "trust_anchors.h"


// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
 
// Set the static IP address to use if the DHCP fails to assign
#define MYIPADDR 192,168,1,28
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,1,1
#define MYGW 192,168,1,1

 
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)
const int rand_pin = (15u);

uint16_t PORTKU = 9999;

EthernetServer *serverPtr = NULL;
/*

# Example using OpenSSL to get the PEM file first
echo | openssl s_client -showcerts -connect www.example.com:443 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' > certificate.pem

# Then convert it using the script
python pycert_bearssl.py convert --no-search certificate.pem --output certificates.h

python pycert_bearssl.py download --output certificates.h www.example.com


*/


void setup() {
    Serial.begin(115200);
 
    //this a stopper tu make sure serial is ready
    //while (!Serial) {delay(100);};
 
    Serial.println("Begin Ethernet");
 
    // You can use Ethernet.init(pin) to configure the CS pin
    //Ethernet.init(10);  // Most Arduino shields
    // Ethernet.init(5);   // MKR ETH Shield
    //Ethernet.init(0);   // Teensy 2.0
    //Ethernet.init(20);  // Teensy++ 2.0
    //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
    //Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet
    // Ethernet.init(PA4); // STM32 with w5500
    //Ethernet.init(17); // Raspberry Pi Pico with w5500
    Ethernet.init(5); // RP2040 Zero with w5500


          IPAddress ip(MYIPADDR);
          IPAddress dns(MYDNS);
          IPAddress gw(MYGW);
          IPAddress sn(MYIPMASK);
          Ethernet.begin(mac, ip, dns, gw, sn);

        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
          while (true) {
            delay(1); // do nothing, no point running without Ethernet hardware
          }
        }
        if (Ethernet.linkStatus() == LinkOFF) {
          Serial.println("Ethernet cable is not connected.");
        }

    delay(5000);
  serverPtr = new EthernetServer(PORTKU);
  serverPtr->begin();

    Serial.print("Local IP : ");
    Serial.println(Ethernet.localIP());
    Serial.print("Subnet Mask : ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway IP : ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS Server : ");
    Serial.println(Ethernet.dnsServerIP());
 
   Serial.println("Server at port " + String(PORTKU) + "Successfully Initialized");



}
 
void loop() {


  // Listen for incoming clients
  EthernetClient client = serverPtr->available();

  if (client) {
    Serial.println("New client connected.");

    // Check for incoming data from the client
    while (client.connected()) {
      if (client.available()) {
        char c = client.read(); // Read a byte from the client
        client.write(c+1);        // Echo the byte back to the client
        Serial.write(c+1);        // Print to serial monitor for debugging
        //client.write(c);        // Echo the byte back to the client
        //Serial.write(c);        // Print to serial monitor for debugging

      }
    }
    // Client disconnected
    Serial.println("Client disconnected.");
    client.stop(); // Close the connection
  }



}