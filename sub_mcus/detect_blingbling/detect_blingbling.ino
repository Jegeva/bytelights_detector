#include <ESP8266WiFi.h>

#include <WiFiUdp.h>
IPAddress local_IP;
IPAddress local_subnet;
IPAddress local_broadcast;

const int ledPin =  LED_BUILTIN;// the number of the LED pin
const int interpin = 15;

// Variables will change:
volatile int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
volatile unsigned long previousMillis = 0;        // will store last time LED was updated
volatile unsigned long currentMillis;
// constants won't change:
const long interval = 100; 

char ssid[] = "telenet-1EF5AC8";
char pass[] = "fGawcczenan6"; //Password of your Wi-Fi router

volatile uint8_t m = 1;
WiFiUDP Udp;
char  replyPacket[] = "b";
char  brightPacket[] = "B ";

volatile int ISR_Fired=0;


volatile unsigned long ISRpreviousMillis = 0;        // will store last time LED was updated
volatile unsigned long ISRcurrentMillis;
#line 47 "/home/jg/Arduino/Bytelights/detect_blingbling/detect_blingbling.ino"
void connectAP();
#line 81 "/home/jg/Arduino/Bytelights/detect_blingbling/detect_blingbling.ino"
void setup();
#line 94 "/home/jg/Arduino/Bytelights/detect_blingbling/detect_blingbling.ino"
void loop();
#line 34 "/home/jg/Arduino/Bytelights/detect_blingbling/detect_blingbling.ino"
void ICACHE_RAM_ATTR interpinSR(){
  
  if(ISR_Fired) return;
  
  ISRcurrentMillis = millis();
  if (ISRcurrentMillis - ISRpreviousMillis >= 200) {   
      ISR_Fired = 1;
      ISRpreviousMillis = currentMillis;
  detachInterrupt(digitalPinToInterrupt(interpin));
      
  }
}

void connectAP(){
  Serial.println();
  Serial.println();
  Serial.print("Connecting to...");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  local_IP=WiFi.localIP();
  local_subnet=WiFi.subnetMask();
  
  local_broadcast = local_IP;
  local_broadcast[0] = local_broadcast[0]&local_subnet[0];
  local_broadcast[1] = local_broadcast[1]&local_subnet[1];
  local_broadcast[2] = local_broadcast[2]&local_subnet[2];
  local_broadcast[3] = ~(local_broadcast[3]&local_subnet[3]);
  
  
  Serial.println("");
  Serial.println("Wi-Fi connected successfully");
  Serial.println("IP address: ");
  Serial.println(local_IP); 
  Serial.println(local_subnet);
  Serial.println(local_broadcast);


  
}

const int analogInPin = A0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  pinMode(LED_BUILTIN, OUTPUT); pinMode(interpin, INPUT);
  connectAP();
  attachInterrupt(digitalPinToInterrupt(interpin),interpinSR,RISING);

}


char old_analog_val = 0;


void loop() {
  // put your main code here, to run repeatedly:
  //
  delay(100);
  char packetBuffer[2];
  //Serial.print('t');
  currentMillis = millis();
  memset(packetBuffer, 0, 1);
  char sensorValue = 0;

  if(ISR_Fired){
     if (ledState == LOW) {
        ledState = HIGH;
     } else {
        ledState = LOW;
     }
     digitalWrite(ledPin, ledState);
     Udp.beginPacket(local_broadcast, 1234);  
     Udp.write(replyPacket); 
     Udp.endPacket();
     ISR_Fired=0;
     attachInterrupt(digitalPinToInterrupt(interpin),interpinSR,RISING);
  }

  if (currentMillis - previousMillis >= interval) {
    
    // save the last time you blinked the LED
    previousMillis = currentMillis;
// Udp.beginPacket(broadcast, 1234); //NTP requests are to port 123
  //Serial.println("4");
 // EthernetUdp.write(packetBuffer, 1);
  //Serial.println("5");
 // EthernetUdp.endPacket();

    sensorValue = analogRead(analogInPin)>>2;

    if(old_analog_val!=sensorValue){
      Serial.println("B pack");
      packetBuffer[0] = 'B';
      packetBuffer[1] = sensorValue;
      Udp.beginPacket(local_broadcast, 1234);    
      Udp.write(packetBuffer); 
      Udp.endPacket();
      old_analog_val=sensorValue;
     }
    
    /*Serial.println(sensorValue);
    Serial.println(ISR_Fired);
    */
    // if the LED is off turn it on and vice-versa:
    // set the LED with the ledState of the variable:
    int s = WiFi.status();
    if( (s == WL_DISCONNECTED) || (s == WL_CONNECTION_LOST) ){
     // WiFi.stop();
     
      connectAP();
     }
  }
}

