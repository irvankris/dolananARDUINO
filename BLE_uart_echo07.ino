/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "EasyStringStream.h"




SSD1306 display(0x3c, 21, 22);


uint32_t chipId = 0;
  unsigned long previousMillis = 0;
  unsigned long currentMillis ;
long dispinterval = 5000;           // interval at which to blink (milliseconds)


BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

bool messageReceivedComplete = false;

#define stringlenku 1024 

char buf01[stringlenku];
EasyStringStream ss01(buf01, stringlenku);

char buf02[stringlenku];
EasyStringStream ss02(buf02, stringlenku);

char buf03[stringlenku];
EasyStringStream ss03(buf03, stringlenku);


uint8_t txValue = 0;
std::string message;
std::string message02;

  char line01[15];  
  char line02[15];  
  char line03[15];  
  char line04[15];  




void drawdisp() {

  display.clear();
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, line01);
    display.drawString(0, 15, line02);
    display.drawString(0, 30, line03);
    display.drawString(0, 45, line04);

  display.display();
}

void scroll_text(const  char *src, size_t len){

//memcpy(srcArray, destArray, sizeof(destArray));

 memcpy(line04, line03, sizeof(line03));
 memcpy(line03, line02, sizeof(line02));
 memcpy(line02, line01, sizeof(line01));
 memset(line01, 0, sizeof(line01)); 
    
        for (int i = 0; i < sizeof(line01) ; i++) {
          if ( i < len){
               line01[i]=src[i];
          }          
        }
  
}

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Connected!");      
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Disconnected!");      
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();


      if (rxValue.length() > 0) {
        
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);          
        }
        Serial.println();
 
        //add to message (as of now just one packet)
        ss01.reset();    
        ss01 << rxValue.c_str() ;     

        //once you think all packets are received. As of now one packet
        messageReceivedComplete = true;

      }

      
    }
};

void respond(std::string  send_message){
  Serial.println("inside send_message");
  pTxCharacteristic->setValue(send_message);
  pTxCharacteristic->notify();
  Serial.println("sent send_message");

}

void respond_l(const  char *src, size_t len){
  
std::string  send_message;
    for (; len > 0; --len)
    {
        char c = *src++;
        send_message = send_message + c;
    }
      
  Serial.println("inside send_message");
  pTxCharacteristic->setValue(send_message);
  pTxCharacteristic->notify();
  Serial.println("sent send_message");
  
}

boolean chararray_cmp(const char *a, int len_a, const char *b, int len_b){
      int n;

      // if their lengths are different, return false
      if (len_a != len_b) return false;

      // test each element to be the same. if not, return false
      for (n=0;n<len_a;n++) if (a[n]!=b[n]) return false;

      //ok, if we have not returned yet, they are equal :)
      return true;
}


void create_hex_string_implied(const  char *src, size_t len,  char *dest)
{
    static const  char table[] = "0123456789abcdef";

    for (; len > 0; --len)
    {
        char c = *src++;
        *dest++ = table[c >> 4];
        *dest++ = table[c & 0x0f];
    }
}


void printtox(const char *bb, int n) 
{

        for (int i = 0; i < n; i++) {
          if (bb[i] < 16 )
             Serial.print("0");
          Serial.print(bb[i], HEX);       
        }
          Serial.println();  
}



void setup() {
  Serial.begin(115200);

  ss01.reset();
  ss02.reset();

  memset(line01, 0, sizeof(line01));
  memset(line02, 0, sizeof(line02));
  memset(line03, 0, sizeof(line03));
  memset(line04, 0, sizeof(line04));


  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10); 

  // Create the BLE Device
  BLEDevice::init("regSMP01");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_TX,
										BLECharacteristic::PROPERTY_NOTIFY
									);
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  drawdisp() ;
}

void loop() {
currentMillis = millis();
  if(currentMillis - previousMillis > dispinterval) {
      drawdisp() ;
      previousMillis = currentMillis; 
  }
    

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
		// do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }


  // if device is connected and all packets are received, then respond
  if(deviceConnected && messageReceivedComplete){

      
       messageReceivedComplete = false;        

       scroll_text(ss01.get(),ss01.getCursor());
       drawdisp() ;
        
        Serial.println("ss01 >");
        Serial.println(ss01.getLength());
        Serial.println(ss01.get());
        printtox(ss01.get(),ss01.getCursor()) ;
             
       

    

       // pada penerimaan message dari app Android Simple BLE Terminal, selalu ditambah 0x0D 0x0A pada message

       
       //disini protokol device sangat berpengaruh. 
       //kita batasi len>2 karena android selalu mengirim packet ditambah 0x0D,0x0A 
       if(ss01.getCursor()>2) { 
           int ss01_cutlen = ss01.getCursor()-2;
              //masukkan tag protokol

           //Bila yang dikirim adalah "hello"   
              ss02.reset();
              ss02 << "hello";

           if(chararray_cmp( ss02.get() , ss02.getCursor(), ss01.get(), ss01_cutlen  )){
              ss02.reset();
              ss02 << "world";
              ss02 << (char)0x0d;
              ss02 << (char)0x0a;
              respond_l(ss02.get(), ss02.getCursor());

               scroll_text(ss02.get(), ss02.getCursor());
               drawdisp() ;
              

              Serial.println("sent world");

            
              
           }else{

              

             //Bila yang dikirim adalah "siapa kamu"   
                ss02.reset();
                ss02 << "siapa kamu?";
             if(chararray_cmp( ss02.get() , ss02.getCursor(), ss01.get(), ss01_cutlen  )){
                ss02.reset();
                ss02 <<  "ESP32_ID " ;
                ss02 <<  String((uint32_t)chipId, HEX);
                ss02 << (char)0x0d;
                ss02 << (char)0x0a;
                respond_l(ss02.get(), ss02.getCursor());
                
                scroll_text(ss02.get(), ss02.getCursor());
                drawdisp() ;

                Serial.println("sent siapa kamu");
  
              
                
             }else{

             //Bila yang dikirim adalah "siapa kamu"   
                ss02.reset();
                ss02 << "siapa saya?";
             if(chararray_cmp( ss02.get() , ss02.getCursor(), ss01.get(), ss01_cutlen  )){
                ss02.reset();
                ss02 <<  "si pelupa." ;
                ss02 << (char)0x0d;
                ss02 << (char)0x0a;
                respond_l(ss02.get(), ss02.getCursor());
                
                scroll_text(ss02.get(), ss02.getCursor());
                drawdisp() ;

                Serial.println("sent siapa saya");
  
              
                
             }else{

                 respond_l(ss01.get(), ss01.getCursor());
                 
                 scroll_text(ss01.get(), ss01.getCursor());
                drawdisp() ;

                Serial.println("echoed");


             } //else siapa saya 
                
             }//else siapa kamu

           }//else hello

           
       
       }
  }


    
}
