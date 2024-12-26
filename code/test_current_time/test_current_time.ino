#include <Arduino.h>
#include <IRremote.hpp> // include the library
#include <TimeLib.h>
#include "WiFi.h"
#include "ESPAsyncWebSrv.h"
#include "SPIFFS.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>

struct CurrentTime {
  int days;
  char months[4]; // Platz für den Monatsnamen in drei Buchstaben + Nullterminator
  int years;
  int hours;
  int minutes;
  int seconds;
};

struct CurrentTime currentTime;

struct CommandParam {
  int hours;
  int minutes;
  int weekdays[7]; // with sunday at index 0
};

struct CommandParam paramOn = {11, 53, {0, 0, 1, 1, 1, 0, 0}};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("start");
}

void loop() {
  const char* today = "Tue Dec 17 2024 15:01:22 GMT+0100 (Mitteleuropäische Normalzeit)";
  today = "Tue";
  today = "21";
  String todayStr = "21";
  todayStr = "Tue Dec 17 2024 15:01:22 GMT+0100 (Mitteleuropäische Normalzeit)";
  Serial.println("today: " + todayStr);
//  int todaySize = today.length();
//  Serial.println("length: " + todaySize);
//  char todayChr[todaySize];
//  for (int i = 0; i < today.length(); i++) {
//    todayChr[i] = today[i];
//  }
//  Serial.println(String(todayChr));
//  const char* todayPtr = todayChr;
//  Serial.println("today: " + today);

  //char* todayPtr[100];
  //todayPtr = today;

  const char* cstr = todayStr.c_str();
  Serial.println("today char: " + String(cstr));
  
  //sscanf(cstr, "%s", currentTime.days);

  int date;
  sscanf(cstr, "%d", &currentTime.days);
  //char days[3];
  //sscanf(cstr, "%s", &days);

  sscanf(cstr, "%*s %3s %d %d %d:%d:%d %*s", &currentTime.months, &currentTime.days, &currentTime.years, &currentTime.hours, &currentTime.minutes, &currentTime.seconds);
  
  Serial.println(currentTime.days);
  Serial.println("time: " + String(currentTime.hours) + ":" + String(currentTime.minutes) + ":" + String(currentTime.seconds));

  delay(20000);
}
