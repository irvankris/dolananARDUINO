/*
    This sketch shows the Ethernet event usage

Board : W32-ETH01

Source Code ini adalah PoC TCP client menggunakan ethernet
adopsi dari 
https://github.com/espressif/arduino-esp32/blob/master/libraries/Ethernet/examples/ETH_LAN8720/ETH_LAN8720.ino
dan 
https://github.com/khoih-prog/WebServer_WT32_ETH01/blob/main/src/WebServer_WT32_ETH01_Impl.h

ini membuktikan bahwa pada board ESP32 , library Ethernet dan LAN8720, 
berfungsi/kompatibel dengan baik.

selanjutnya mencoba kompatibilitas dengan client MQTT existing dan client menggunakan SSL Certificate

*/

#include <ETH.h>


// Important to be defined BEFORE including ETH.h for ETH.begin() to work.
// Example RMII LAN8720 (Olimex, etc.)
//#define ETH_PHY_TYPE        ETH_PHY_LAN8720
//#define ETH_PHY_ADDR         0
//#define ETH_PHY_MDC         23
//#define ETH_PHY_MDIO        18
//#define ETH_PHY_POWER       -1
//#define ETH_CLK_MODE        ETH_CLOCK_GPIO0_IN

//definisi untuk WT32_eth01
#define ETH_PHY_TYPE        ETH_PHY_LAN8720
#define ETH_PHY_ADDR         1
#define ETH_PHY_MDC         23
#define ETH_PHY_MDIO        18
#define ETH_PHY_POWER       16
#define ETH_CLK_MODE        ETH_CLOCK_GPIO0_IN


static bool eth_connected = false;

// WARNING: WiFiEvent is called from a separate FreeRTOS task (thread)!
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      // The hostname must be set after the interface is started, but needs
      // to be set before DHCP, so set it from the event handler thread.
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.println("ETH Got IP");
      Serial.println(ETH.localIP());
      //ETH.printInfo(Serial);
      eth_connected = true;
      break;
    //case ARDUINO_EVENT_ETH_LOST_IP:
    //  Serial.println("ETH Lost IP");
    //  eth_connected = false;
    //  break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void testClient(const char * host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available());
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
}

/*
void setup()
{
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);  // Will call WiFiEvent() from another thread.
  ETH.begin();
}
*/
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("ETHku plain\n");

  
  WiFi.onEvent(WiFiEvent);

  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  
  Serial.println("ETHku setelah begin\n");

}


/*
void loop()
{
  if (eth_connected) {
    testClient("google.com", 80);
  }
  delay(10000);
}
*/


void loop() {
  if (eth_connected) {
    Serial.println("ETHku Tes klien\n");    
    testClient("www.google.com", 80);
  }
  delay(10000);
  Serial.println("ETHku LOOOP !\n");
}
