#ifndef TH11
#define TH11

#include "DHT.h"

#define DHTPIN D4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const int DHTSensingDelay = 2100;
// 스펙 상 2초에 1번씩 작동

float readTemp();
float readHum();

float readTemp(){
  return dht.readTemperature();
}

float readHum() {
  return dht.readHumidity();
}

#endif