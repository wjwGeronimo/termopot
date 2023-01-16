#include <microDS18B20.h>
#include <SoftwareSerial.h>

int RX_PIN = 4; //Arduino RX -> HC-06 TX
int TX_PIN = 5; //Arduino TX -> HC-06 RX
int LED_PIN = 13;
boolean running = true;
int driftPeriod = 15000;
float temp;
float temp2;
float drf;

uint8_t addr1[] = {0x28, 0xFF, 0x5B, 0x4C, 0x61, 0x16, 0x3, 0xF5}; //#1
uint8_t addr2[] = {0x28, 0x1D, 0xDB, 0x49, 0xF6, 0x4D, 0x3C, 0x1}; //#2

MicroDS18B20<12, addr1> sensor;
MicroDS18B20<12, addr2> sensor2;
SoftwareSerial hc06(RX_PIN, TX_PIN);

void setup() {
  Serial.begin(9600);
  hc06.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  drift();
}

void readTemp() {
  bool rsens = sensor.readTemp();
  bool rsens2 = sensor2.readTemp();

  if (rsens) 
  {
    temp = sensor.getTemp();
    Serial.print(temp);
    hc06.print(temp);
  }
  else
  {
    Serial.print("error");
    hc06.print("error");
  }

  Serial.print(';');
  hc06.print(';');
  Serial.print(running);
  hc06.print(running);
  Serial.print(';');
  hc06.print(';');

  if (rsens2)
  {
    temp2 = sensor2.getTemp();
    Serial.print(temp2);
    hc06.print(temp2);
  }
  else 
  {
    Serial.print("error");
    hc06.print("error");
  }

  Serial.print(';');
  hc06.print(';');
  Serial.print(drf);
  hc06.print(drf);
  
  Serial.println();
  hc06.println();
  
  sensor.requestTemp();
  sensor2.requestTemp();
}

void processState() {
  int val = hc06.read();
  if (val == '2') running = !running;
  if (val == '1') running = true;
  if (val == '0') running = false;
  digitalWrite(LED_PIN, running);
}

float drift() {
  static float firstTemp;
  static uint32_t firstState;
  static float delta;

  float secondTemp = temp;
  uint32_t secondState = millis();

  if ((secondState - firstState) >= driftPeriod)
  {
    delta = secondTemp - firstTemp;
    firstTemp = secondTemp;
    firstState = secondState;
  }
  else
  {
    if (firstState == 0) return 1;
  }
  return delta;
}

void processBoil(){
  float dr = drift();
  drf = dr;
  static float lastDr;

  if (running)
  {
    if (dr > 1.5) running = false;
    if (lastDr > dr)
    {
      if (dr < 0.2) running = false;
    }
  }
  else
  {
    if (dr <= -2.0) running = true;
  }
  lastDr = dr;
}

void loop() {
  static uint32_t tmr;
  if ((millis() - tmr) >= 800)
  {
    tmr = millis();
    readTemp();
    processBoil();
  }
  processState();  
}
