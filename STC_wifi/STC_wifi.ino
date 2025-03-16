#define BLYNK_TEMPLATE_ID "TMPL2E63cE2pB"
#define BLYNK_TEMPLATE_NAME "STC WIFI"

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
float tempsensor;
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
  String stat = "Idle";
};
bool button_setup;
bool button_up;
bool button_down;
bool program = false;
unsigned long previousMillis = 0;  // Stores the last time the LED was updated
int ledState = HIGH;     // Initial LED state
const long interval = 1100;
long MAX_COOL_TIME = 7200000;
long REST_LAG = 1800000;
String sw1_change_status = "Idle";
String sw2_change_status = "Idle";


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
BlynkTimer timer;
// init
;
//call programming order.

struct switches sw1;
struct switches sw2;


void setup() {
  // put your setup code here, to run once:

  digitalWrite(pincooling1, HIGH);
  digitalWrite(pinheating1, HIGH);
  digitalWrite(pincooling2, HIGH);
  digitalWrite(pinheating2, HIGH);
  pinMode(pincooling1, OUTPUT);
  pinMode(pinheating1, OUTPUT);
  pinMode(pincooling2, OUTPUT);
  pinMode(pinheating2, OUTPUT);
  pinMode(pinled_on, OUTPUT);
  sensors.begin();
  Serial.begin(115200);
  delay(1000);
  BlynkEdgent.begin();
  //Set timer interval to send temmperature data to cloud 1 time each 10 minutes
  timer.setInterval(600000L, tempSensorTimer);
}


void loop() {
  // put your main code here, to run repeatedly:
  timer.run();
  BlynkEdgent.run();
  if (Serial.available() > 0) {
    float value = Serial.parseFloat();
    if (value != 0) {
      sw1.set = value;
    }
  }
  sensors.requestTemperatures();
  tempsensor = sensors.getTempCByIndex(0);
  // there is an unnknown error that sommetimes it doesnt read correcty. if this happens, do not actualize temperature.
  if (tempsensor > -127.0) {
    sw1.probe = tempsensor;
  }
  tempsensor = sensors.getTempCByIndex(1);
  if (tempsensor > -127.0) {
    sw2.probe = tempsensor;
  }
  temperature_control(sw1);
  temperature_control(sw2);
  relays(sw1, pincooling1, pinheating1);
  relays(sw2, pincooling2, pinheating2);

  blinkNonBlocking(pinled_on, interval);

  // Send status data each time the status changes.
  if (sw1.stat != sw1_change_status) {
    Blynk.virtualWrite(V12, sw1.stat);
    sw1_change_status = sw1.stat;
    Serial.println("Sw1_stat_sent");
  }
  if (sw2.stat != sw2_change_status) {
    Blynk.virtualWrite(V13, sw2.stat);
    sw2_change_status = sw2.stat;
    Serial.println("Sw2_stat_sent");
  }
}

void temperature_control(switches &sw) {

  long lag_time;
  Serial.print("  Probe: "); Serial.print(sw.probe);
  Serial.print(", Set: "); Serial.print(sw.set);
  Serial.print(", Delta: "); Serial.print(sw.delta);
  Serial.print(", Heating: "); Serial.print(sw.heating);
  Serial.print(", Cooling: "); Serial.print(sw.cooling);
  Serial.print(", millis: "); Serial.print(millis());
  Serial.print(", compressor: "); Serial.println(millis() - sw.compressor);
  Serial.print(", cooling_timer: "); Serial.println(sw.cool_timer_start);
  Serial.print(", status: "); Serial.println(sw.stat);

  // if the compressor is resting lag time extended to REST_LAG
  if (sw.compressor_rest) {
    lag_time = REST_LAG;
  }
  else {
    lag_time = sw.compressor_lag;
  }
  if (!(sw.heating && sw.cooling)) {
    if (sw.probe < (sw.set - sw.delta)) {
      sw.heating = true;
      //set stat to heating
    }
  }
  if (millis() - sw.compressor > lag_time) {
    if (sw.probe > (sw.set + sw.delta)) {
      sw.compressor_rest = false;
      if (sw.cooling == false) {
        sw.cooling = true;
        //set stat to cooling
        sw.cool_timer_start = millis();
      }
    }
  }
  else if (!sw.compressor_rest) {
    sw.stat = "Lag";
  }
  if (sw.heating) {
    if (sw.probe >= sw.set) {
      sw.heating = false;
      //Set stat to Idle
      sw.stat = "Idle";
    }
  }
  if (sw.cooling) {
    //consider if the compressor has been working for more than MAX_COOL_TIME
    if (millis() - sw.cool_timer_start > MAX_COOL_TIME) {
      sw.cooling = false;
      sw.compressor_rest = true;
      //set status to Rest
      sw.stat = "Rest";
      sw.compressor = millis();
    }
    if (sw.probe <= sw.set) {
      sw.cooling = false;
      //set status to Idle
      sw.stat = "Idle";
      sw.compressor = millis();
      sw.cool_timer_start = 0;

    }
  }
  //Serial.print(", compressor_lag: "); Serial.println(lag_time);
}
void relays(switches &sw, int pincooling, int pinheating) {
  if (sw.cooling) {
    if (sw.on_cooling) {
      digitalWrite(pincooling, LOW);
      sw.stat = "Cooling";

    }
    else {
      digitalWrite(pincooling, HIGH);
      sw.stat = "Cooling OFF";
    }
  }
  else {
    digitalWrite(pincooling, HIGH);
  }

  if (sw.heating) {
    if (sw.on_heating) {
      digitalWrite(pinheating, LOW);
      sw.stat = "Heating";
    }
    else {
      digitalWrite(pinheating, HIGH);
      sw.stat = "Heating OFF";
    }
  }
  else {
    digitalWrite(pinheating, HIGH);
  }
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
//send temperature values
void tempSensorTimer()
{
  // This function describes what will happen with each timer tick. Send temperature info each 10 minutes to avoid
  // using all packets available
  Serial.println("Tick");
  Blynk.virtualWrite(V10, sw1.probe);
  Blynk.virtualWrite(V11, sw2.probe);

}



BLYNK_CONNECTED()
{
  // sync data when the device connects with the cloud.

  Blynk.syncAll();
  Serial.println("Connected and Synced");
}

// retrieve data when the virtual pin value changes

BLYNK_WRITE(V0)
{
  sw1.on_heating = param.asInt();
  Serial.println("DATA");
}

BLYNK_WRITE(V1)
{
  // any code you place here will execute when the virtual pin value changes
  sw1.on_cooling = param.asInt();
}

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

// for a 2nd set of controllers.
BLYNK_WRITE(V5)
{
  // any code you place here will execute when the virtual pin value changes
  sw2.on_heating = param.asInt();
}

BLYNK_WRITE(V6)
{
  // any code you place here will execute when the virtual pin value changes
  sw2.on_cooling = param.asInt();
}


BLYNK_WRITE(V7)
{
  // any code you place here will execute when the virtual pin value changes
  sw2.set = param.asFloat();
}
BLYNK_WRITE(V8)
{
  // any code you place here will execute when the virtual pin value changes
  sw2.delta = param.asFloat();
}
BLYNK_WRITE(V9)
{
  // any code you place here will execute when the virtual pin value changes
  sw2.compressor_lag = param.asFloat() * 60000;

}
