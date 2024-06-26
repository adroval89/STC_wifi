#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define pincooling 1
#define pinheating 3
#define pinbutton_setup 15
#define pinbutton_up 13
#define pinbutton_down 12
#define pinled_on 14
#define pinled_cool 0
#define pinled_heat 3
#define ONE_WIRE_BUS 2

// constants


// variables
struct switches {
  bool cooling;
  bool heating;
  float delta;
  float set;
  int compressor;
  float probe;
};
bool button_setup;
bool button_up;
bool button_down;
bool led_on;
bool led_cool;
bool led_heat;


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

struct switches sw1 = {false, false, 0.5, 10, 3, 0};
struct switches sw2 = {false, false, 0.5, 10, 3, 0};


void setup() {
  // put your setup code here, to run once:

  pinMode(pincooling, OUTPUT);
  pinMode(pinheating, OUTPUT);
  pinMode(pinbutton_setup, INPUT);
  pinMode(pinbutton_up, INPUT);
  pinMode(pinbutton_down, INPUT);
  pinMode(pinled_on, OUTPUT);
  pinMode(pinled_cool, OUTPUT);
  pinMode(pinled_heat, OUTPUT);
  sensors.begin();

}

void loop() {
  // put your main code here, to run repeatedly:

  sensors.requestTemperatures(); // for the first ubidots status check and data send
  sw1.probe = sensors.getTempCByIndex(0);
  sw2.probe = sensors.getTempCByIndex(1);
  temperature_control(sw1);
  temperature_control(sw2);

}

void temperature_control(switches &sw) {
  if (!(sw.heating && sw.cooling)) {
    (sw.probe < (sw.set - sw.delta)) ? sw.heating = true : sw.heating = false;
    (sw.probe > (sw.set + sw.delta)) ? sw.cooling = true : sw.cooling = false;
  }
  if (sw1.heating) {
    (sw.probe >= sw.set) ? sw.heating = false : sw.heating = true;
  }
  if (sw1.cooling) {
    (sw.probe <= sw.set) ? sw.cooling = false : sw.cooling = true;
  }
}
