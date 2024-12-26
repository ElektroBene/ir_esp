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

struct NextCommand {
  uint32_t command;
  int day;
  int hour;
  int minute;
};

int compileHour, compileMinute, compileSecond, compileYear, compileDay;
char compileMonth[4];

int monthToInt(const char* month) {
  // Array der Monatsnamen
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  // Durchlaufen des Arrays und Vergleich der Monatsnamen
  for (int i = 0; i < 12; ++i) {
    if (strncmp(month, months[i], 3) == 0) {
      return i + 1; // Monat gefunden, Index zurückgeben
    }
  } // Falls der Monat nicht gefunden wird, -1 zurückgeben
  return -1;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  sscanf(__TIME__, "%d:%d:%d", &compileHour, &compileMinute, &compileSecond);  
  sscanf(__DATE__, "%s %d %d", &compileMonth, &compileDay, &compileYear);
  setTime(compileHour, compileMinute, compileSecond, compileDay, monthToInt(compileMonth), compileYear);
}

void loop() {
  NextCommand c1 = {0x46A88248, 5, 18, 30};
  NextCommand c2 = {0x46228248, 5, 16, 59};
  NextCommand c3 = {0x00000000, 4, 17, 40};
  getEarliest(c1, c2, c3);
  delay(20000);
}

bool isEarlier(const NextCommand& a, const NextCommand& b) {
  if (a.day != b.day) {
    return a.day < b.day;
  }
  if (a.hour != b.hour) {
    return a.hour < b.hour;
  }
  return a.minute < b.minute;
}

NextCommand getEarliest(NextCommand t1, NextCommand t2, NextCommand t3) {
  transformCommandTimes(t1, t2, t3);
  NextCommand earliest = t1;
  if (isEarlier(t2, earliest)) {
    earliest = t2;
  }
  if (isEarlier(t3, earliest)) {
    earliest = t3;
  }
  transformTimesBack(earliest);
  printCmd("earliest", earliest);
  return earliest;
}

void transformCommandTimes(NextCommand &t1, NextCommand &t2, NextCommand &t3) {
  int weekdays = weekday();
  int hours = hour();
  int minutes = minute();
  Serial.println("weekday: " + String(weekdays));
  printCmd("t1 before", t1);
  transformTimes(t1, weekdays, hours, minutes);
  printCmd("t1 after", t1);
  printCmd("t2 before", t2);
  transformTimes(t2, weekdays, hours, minutes);
  printCmd("t2 after", t2);
  printCmd("t3 before", t3);
  transformTimes(t3, weekdays, hours, minutes);
  printCmd("t3 after", t3);
}

void transformTimes(NextCommand &t, const int weekdays, const int hours, const int minutes) {
  t.day -= weekdays;
  if(t.day < 0) {
    t.day += 7;
  }
  t.hour -= hours;
  if(t.hour < 0) {
    t.hour += 24;
  }
  t.minute -= minutes;
  if(t.minute < 0) {
    t.minute += 60;
  }
}

void transformTimesBack(NextCommand &t) {
  t.day += weekday();
  if (t.day > 6) {
    t.day -= 7;
  }
  t.hour += hour();
  if (t.hour > 23) {
    t.hour -= 24;
  }
  t.minute += minute();
  if (t.minute > 59) {
    t.minute -= 60;
  }
}

void printCmd(const String txt, const NextCommand &t) {
  Serial.println(txt + " -> next commmand: " + String(t.day) + " " + String(t.hour) + ":" + String(t.minute));
}
