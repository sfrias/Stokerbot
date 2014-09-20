/*
  WiFi Web Server

 A simple web server that shows the value of the analog input pins.
 using a WiFi shield.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Circuit:
 * WiFi shield attached
 * Analog inputs attached to pins A0 through A5 (optional)

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe

 */

#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <JsonGenerator.h>
#include <JsonParser.h>
#include <Wire.h>
#include <BMA222.h>
#include <Adafruit_TMP006.h>

#include "Queue.h"
#include "WebClient.h"

using namespace ArduinoJson;

Queue tasks;
BMA222 mySensor;
Adafruit_TMP006 tmp006(0x41);


// your network name also called SSID
char ssid[] = "---";
// your network password
char password[] = "---";
// your network key Index number (needed only for WEP)
int keyIndex = 0;
int clientid = 0;

WiFiServer server(80);

void setup() {

  Serial.begin(115200);      // initialize serial communication
  pinMode(RED_LED, OUTPUT);      // set the LED pin mode

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  // you're connected now, so print out the status  
  printWifiStatus();
  
  Serial.println("Starting webserver on port 80");
  server.begin();                           // start the web server on port 80
  Serial.println("Webserver started!");

  //randomSeed(analogRead(14));

  mySensor.begin();
  WSInit();

// tasks.scheduleFunction(exports, "exports", 0, 5000);
// tasks.scheduleFunction(test, "test", 0, 100);
//   tasks.scheduleFunction(accel, "accel", 0, 1000);
   tasks.scheduleFunction(WSRun, "WSrun", 0, 1000);
//   tasks.scheduleFunction(temp, "temp", 0, 1000);
    
  if (! tmp006.begin()) {
    Serial.println("No sensor found");
  }  

}

int temp(unsigned long now)
{
  //tmp006.wake();
  float objt = tmp006.readObjTempC();
  Serial.print("Object Temperature: "); Serial.print(objt); Serial.println("*C");
  float diet = tmp006.readDieTempC();
  Serial.print("Die Temperature: "); Serial.print(diet); Serial.println("*C");    
}

void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    clientid++;
    Serial.print("new client ");
    Serial.println(clientid);
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          delay(25);
          client.println("Content-Type: text/html");
          delay(25);
          client.println("Connection: close");  // the connection will be closed after completion of the response
          delay(25);
//          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          delay(25);
          client.println();
          delay(25);
          client.println("<!DOCTYPE HTML>");
          delay(25);
          client.println("<html>");

          for (uint16_t i=0; i<100; i++)
          {
            client.println("Dummy");
            delay(2);
          }
          /*
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogChannel; //analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          */
          client.println("</html>");
          
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }

    // close the connection:
    client.stop();
    Serial.print("client ");
    Serial.print(clientid);
    Serial.println(" disonnected");
  }
  
  tasks.Run(millis());
 
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("Network Name: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

int exports(long unsigned int now)
{
  sendReport();  
}

int test(long unsigned int now)
{
  PubNubRun();
}

int accel(long unsigned int now)
{
  Generator::JsonObject<4> root; 
  root["id"] = "CC3200";
  root["x"] = mySensor.readXData();
  root["y"] = mySensor.readYData();
  root["z"] = mySensor.readZData();
  char buffer[100];
  memset(buffer, NULL, sizeof(buffer));
  root.printTo(buffer, sizeof(buffer));  
//  PubNubPub(buffer); 
  sendMessage(buffer);
}
