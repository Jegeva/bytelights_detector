#include "FastLED.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const int ledPin =  LED_BUILTIN;

char ssid[] = "telenet-1EF5AC8";
char pass[] = "fGawcczenan6"; //Password of your Wi-Fi router

byte packetBuffer[16];
const long interval = 1000;
volatile int ledState = LOW;
WiFiUDP Udp;
volatile unsigned long previousMillis = 0;        // will store last time LED was updated
volatile unsigned long currentMillis;
char incomingPacket[128];

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
  Serial.println("");
  Serial.println("Wi-Fi connected successfully");
  Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); 
    Serial.println(WiFi.subnetMask());
  }



FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif


#define DATA_PIN    5   // wemos = D1
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    32
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

volatile int curr_bright = BRIGHTNESS;

void setup() {
  delay(500); // 3 second delay for recovery500
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  Serial.begin(115200); 
  Serial.println("Hey\r\n"); 
  connectAP();
  Udp.begin(1234);  
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
volatile uint8_t cycle_go=1;

void loop()
{
  if (cycle_go)
  gPatterns[gCurrentPatternNumber]();
  currentMillis = millis();

  
  int packetSize = Udp.parsePacket();
 if (packetSize)
  {

    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
     incomingPacket[len] = 0;
    if(incomingPacket[0] =='b'){

if((curr_bright * 2) > 255){
    FastLED.setBrightness(255);}else{
      FastLED.setBrightness((curr_bright * 2));
      }
    for(int i = 0;i<NUM_LEDS;i++){
        leds[ i ] += CRGB::White;
    }
        
     }
      if(incomingPacket[0] =='B'){ 
        // to manually change brightness
        // ' ' = off, '?' = 30 % , '_' = 60%
        
        int i = incomingPacket[1];
         FastLED.setBrightness(i);
         curr_bright = i;
        }
    if(incomingPacket[0] =='S'){ // to sync when plenty of them
      gCurrentPatternNumber=0;
      gHue=0;
      cycle_go = 1;
      }
    if(incomingPacket[0] =='f'){ // to sync when plenty of them
      cycle_go = !cycle_go;    
         Serial.println("f ");
         Serial.println(cycle_go) ;
       fixed();         
    }
    if(incomingPacket[0] =='H'){ // to sync when plenty of them
       cycle_go = 0;
       gHue=incomingPacket[1];
       fixed();         
    }
    if(incomingPacket[0] =='W'){ // to sync when plenty of them
       cycle_go = 0;
       gHue=incomingPacket[1];
        for(int i = 0;i<NUM_LEDS;i++){
        leds[ i ] += CRGB::White;
    }
    }
  }
  }

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    int s = WiFi.status();
    if( (s == WL_DISCONNECTED) || (s == WL_CONNECTION_LOST) ){
     // WiFi.stop();  
      connectAP();
    }
  }

  
  // Call the current pattern function once, updating the 'leds' array
 

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  FastLED.setBrightness(curr_bright);
  // insert a delay to keep the framerate modest
  //FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { inc_ghue(); } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void inc_ghue(){
  
    if (cycle_go) gHue++; 
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  if (cycle_go)
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void fixed(){
  for(int i = 0;i<NUM_LEDS;i++){
        leds[ i ] = CHSV( gHue, 255, 192);
    }
  }

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
