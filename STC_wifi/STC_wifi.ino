#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>

#define BLYNK_TEMPLATE_NAME "STC WIFI"
#define BLYNK_AUTH_TOKEN "dYYjBXsLn7ogY8lDS0AiSVepB7kVlj3V"
#define pincooling1 4
#define pinheating1 5
#define pinbutton_setup 15
#define pinbutton_up 13
#define pinbutton_down 12
#define pinled_cool 14
#define pinled_on 0
#define pinled_heat 3
#define ONE_WIRE_BUS 2

// constants

const char* WIFI_SSID =  "ADROIVA";
const char* WIFI_PASS = "Los5locos";

WiFiClient client;

// variables
struct switches {
  bool cooling;
  bool heating;
  float delta;
  float set;
  unsigned long compressor;
  long compressor_lag;
  float probe;
};
bool button_setup;
bool button_up;
bool button_down;
unsigned long previousMillis = 0;  // Stores the last time the LED was updated
const long interval = 1000;  // Interval at which to blink (milliseconds)
bool blinking;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

struct switches sw1 = {false, false, 0.5, 40, 0, 10000, 0};
struct switches sw2 = {false, false, 0.5, 40, 0, 10000, 0};


void setup() {
  // put your setup code here, to run once:

  pinMode(pincooling1, OUTPUT);
  pinMode(pinheating1, OUTPUT);
  pinMode(pinbutton_setup, INPUT);
  pinMode(pinbutton_up, INPUT);
  pinMode(pinbutton_down, INPUT);
  pinMode(pinled_on, OUTPUT);
  pinMode(pinled_cool, OUTPUT);
  pinMode(pinled_heat, OUTPUT);
  sensors.begin();

  Serial.begin(115200);
  delay(1000);
  //
  //  WiFi.mode(WIFI_STA);
  //  WiFi.begin(WIFI_SSID, WIFI_PASS);
  //  Serial.print("Conectando a:\t");
  //  Serial.println(WIFI_SSID);
  //
  //  // Esperar a que nos conectemos
  //  while (WiFi.status() != WL_CONNECTED)
  //  {
  //    delay(200);
  //    Serial.print('.');
  //  }
  //
  //  // Mostrar mensaje de exito y dirección IP asignada
  //  Serial.println();
  //  Serial.print("Conectado a:\t");
  //  Serial.println(WiFi.SSID());
  //  Serial.print("IP address:\t");
  //  Serial.println(WiFi.localIP());
  //
}

void loop() {
  // put your main code here, to run repeatedly:
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
  relays(sw1, pincooling1, pinheating1);
}

void temperature_control(switches &sw) {
  Serial.print("Probe: "); Serial.print(sw.probe);
  Serial.print(", Set: "); Serial.print(sw.set);
  Serial.print(", Delta: "); Serial.print(sw.delta);
  Serial.print(", Heating: "); Serial.print(sw.heating);
  Serial.print(", Cooling: "); Serial.print(sw.cooling);
  Serial.print(", mmillis: "); Serial.print(millis());
  Serial.print(", compressor: "); Serial.println(millis() - sw.compressor);


  if (!(sw.heating && sw.cooling)) {
    if (sw.probe < (sw.set - sw.delta)) {
      sw.heating = true;
    }
    if ((sw.probe > (sw.set + sw.delta)) && (millis() - sw.compressor > sw.compressor_lag)) {
      sw.cooling = true;
    }
  }
  if (sw.heating) {
    if (sw.probe >= sw.set) {
      sw.heating = false;
    }
  }
  if (sw.cooling) {
    if (sw.probe <= sw.set) {
      sw.cooling = false;
      sw.compressor = millis();
    }
  }
}

void relays(switches &sw, int pincooling, int pinheating) {
  if (sw.cooling) {
    digitalWrite(pincooling, HIGH);
  }
  else {
    digitalWrite(pincooling, LOW);
  }
  if (sw.heating) {
    digitalWrite(pinheating, HIGH);
  }
  else {
    digitalWrite(pinheating, LOW);
  }
  if ((sw.cooling) && (millis() - sw.compressor < sw.compressor_lag)){
    nonBlockingBlink(pinled_on, previousMillis, interval);
  }
  delay(1000);
}

void nonBlockingBlink(int ledPin, unsigned long &previousMillis, unsigned long interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (digitalRead(ledPin) == LOW) {
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }
  }
}
