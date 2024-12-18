#include <Arduino.h>
#include <IRremote.hpp> // include the library
#include <TimeLib.h>
#include "WiFi.h"
#include "ESPAsyncWebSrv.h"
#include "SPIFFS.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct CurrentTime {
  int day;
  char month[4]; // Platz für den Monatsnamen in drei Buchstaben + Nullterminator
  int year;
  int hour;
  int minute;
  int second;
};

struct CurrentTime currentTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  String today = "Tue Dec 17 2024 15:01:22 GMT+0100 (Mitteleuropäische Normalzeit)";
  today = "Tue";
//  int todaySize = today.length();
//  Serial.println("length: " + todaySize);
//  char todayChr[todaySize];
//  for (int i = 0; i < today.length(); i++) {
//    todayChr[i] = today[i];
//  }
//  Serial.println(String(todayChr));
//  const char* todayPtr = todayChr;
//  Serial.println("today: " + today);

  char todayPtr[100];
  
  sscanf(todayPtr, "%s", currentTime.day);
  
  Serial.println("test");
  Serial.println(currentTime.hour);
  Serial.println("today: " + String(currentTime.hour) + ":" + String(currentTime.minute) + ":" + String(currentTime.second));

  delay(20000);
}
