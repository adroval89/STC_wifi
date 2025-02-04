#define BLYNK_TEMPLATE_ID "TMPL2KLUHeagS"
#define BLYNK_TEMPLATE_NAME "STC WIFI"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_NODE_MCU_BOARD
#define pincooling1 4
#define pinheating1 5
#define pincooling2 14
#define pinheating2 12
#define pinled_on 15

#define ONE_WIRE_BUS 2

#include <Blynk.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include "BlynkEdgent.h"




// constants


WiFiClient client;

// variables
struct switches {
  bool on_cooling;
  bool on_heating;
  bool cooling;
  bool heating;
  float delta;
  float set;
  unsigned long compressor;
  unsigned long cool_timer_start;
  long compressor_lag;
  float probe;
  bool compressor_rest = false;
};
bool button_setup;
bool button_up;
bool button_down;
unsigned long previousMillis = 0;  // Stores the last time the LED was updated
int ledState = HIGH;     // Initial LED state
const long interval = 1100;
long max_cooling = 10000;
long COMPRESSOR_LAG = 10000;
long MAX_COOL_TIME = 20000;
long REST_LAG = 30000;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

struct switches sw1;
struct switches sw2;



void setup() {
  // put your setup code here, to run once:
  
  digitalWrite(pincooling1, HIGH);
  digitalWrite(pinheating1, HIGH);
  pinMode(pincooling1, OUTPUT);
  pinMode(pinheating1, OUTPUT);
  pinMode(pincooling2, OUTPUT);
  pinMode(pinheating2, OUTPUT);
  pinMode(pinled_on, OUTPUT);
  sw1.compressor_lag = COMPRESSOR_LAG;
  sw2.compressor_lag = COMPRESSOR_LAG;

  sensors.begin();

  Serial.begin(115200);
  delay(1000);
  BlynkEdgent.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  BlynkEdgent.run();
  Blynk.syncAll();
  if (Serial.available() > 0) {
    float value = Serial.parseFloat();
    if (value != 0) {
      sw1.set = value;
    }
  }
  sensors.requestTemperatures();
  sensors.getTempCByIndex(0);
  //sw1.probe = sensors.getTempCByIndex(0);
  //sw2.probe = sensors.getTempCByIndex(1);
  temperature_control(sw1);
  //temperature_control(sw2);
  //  compressorTimeout(sw1, max_cooling);
  relays(sw1, pincooling1, pinheating1);
  blinkNonBlocking(pinled_on, interval);
}

void temperature_control(switches &sw) {
  
  long lag_time;
  Serial.print("Probe: "); Serial.print(sw.probe);
  Serial.print(", Set: "); Serial.print(sw.set);
  Serial.print(", Delta: "); Serial.print(sw.delta);
  Serial.print(", Heating: "); Serial.print(sw.heating);
  Serial.print(", Cooling: "); Serial.print(sw.cooling);
  Serial.print(", millis: "); Serial.print(millis());
  Serial.print(", compressor: "); Serial.println(millis() - sw.compressor);
<<<<<<< HEAD
    Serial.print(", compressor_lag: "); Serial.println(sw.compressor_lag);

=======
 
  Serial.print(", cooling_timer: "); Serial.println(sw.cool_timer_start);

// if the compressor is resting lag time extended to REST_LAG
  if(sw.compressor_rest){
    lag_time = REST_LAG;
  }
  else{
    lag_time = sw.compressor_lag;
    }
>>>>>>> c058152 (first full implemented version)
  if (!(sw.heating && sw.cooling)) {
    if (sw.probe < (sw.set - sw.delta)) {
      sw.heating = true;
    }
    if ((sw.probe > (sw.set + sw.delta)) && (millis() - sw.compressor > lag_time)) {
      sw.compressor_rest = false;
      if (sw.cooling == false) {
        sw.cooling = true;
        sw.cool_timer_start = millis();
      }
    }
  }
  if (sw.heating) {
    if (sw.probe >= sw.set) {
      sw.heating = false;
    }
  }
  if (sw.cooling) {
    if (millis() - sw.cool_timer_start > MAX_COOL_TIME){
      sw.cooling = false;
      sw.compressor_rest = true;
      sw.compressor = millis();
      }
    if (sw.probe <= sw.set) {
      sw.cooling = false;
      sw.compressor = millis();
      sw.cool_timer_start = 0;

    }
  }
Serial.print(", compressor_lag: "); Serial.println(lag_time);
}
void relays(switches &sw, int pincooling, int pinheating) {
  if (sw.cooling) {
    digitalWrite(pincooling, LOW);
  }
  else {
    digitalWrite(pincooling, HIGH);
  }
  if (sw.heating) {
    digitalWrite(pinheating, LOW);
  }
  else {
    digitalWrite(pinheating, HIGH);
  }
  delay(1000);
}

void blinkNonBlocking(int pin, long interval) {
  unsigned long currentMillis = millis();
  // Check if the interval has passed
  if (currentMillis - previousMillis >= interval) {
    // Save the last time the LED was updated
    previousMillis = currentMillis;
    // Toggle the LED state
    ledState = (ledState == LOW) ? HIGH : LOW;
    // Set the LED to the new state
    digitalWrite(pin, ledState);
  }
}

BLYNK_CONNECTED() 
{
  // sync data when the device connects with the cloud.
    Blynk.syncAll();
}
BLYNK_WRITE(V0)
{
  // any code you place here will execute when the virtual pin value changes
  sw1.on_heating = param.asInt();
}

<<<<<<< HEAD
=======
BLYNK_WRITE(V1)
{
  // any code you place here will execute when the virtual pin value changes
  Serial.print("Blynk.Cloud is writing something to V0");
  sw1.on_cooling = param.asInt();
}


>>>>>>> c058152 (first full implemented version)
BLYNK_WRITE(V2)
{
  // any code you place here will execute when the virtual pin value changes
  sw1.set = param.asFloat();
}
BLYNK_WRITE(V3)
{
  // any code you place here will execute when the virtual pin value changes
  sw1.delta = param.asFloat();
}
BLYNK_WRITE(V4)
{
  // any code you place here will execute when the virtual pin value changes
  sw1.compressor_lag = param.asFloat() * 60000;

}
