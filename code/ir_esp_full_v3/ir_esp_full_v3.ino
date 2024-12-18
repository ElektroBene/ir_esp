#include <Arduino.h>
#include <IRremote.hpp> // include the library
#include <TimeLib.h>
#include "WiFi.h"
#include "ESPAsyncWebSrv.h"
#include "SPIFFS.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const char* ssid = "ESP32-Access-Point"; // CHANGE IT
const char* password = "1234567890"; // CHANGE IT

AsyncWebServer server(80);

int compileHour, compileMinute, compileSecond, compileYear, compileDay;
char compileMonth[4];

bool newCommand = false;

unsigned int previousMillis = 0;

struct CommandParam {
  int hours;
  int minutes;
  int weekdays[7]; // with sunday at index 0
};

struct NextCommand {
  uint32_t command;
  int day;
  int hour;
  int minute;
};

struct CommandParam paramOn = {11, 53, {0, 0, 1, 1, 1, 0, 0}};
struct CommandParam paramOff = {22, 30, {0, 0, 1, 1, 1, 0, 0}};
struct CommandParam paramPlaceholder = {0, 0, {0, 0, 0, 0, 0, 0, 0}};

typedef struct zuWartendeZeit
{
  int Tage;
  int Stunden;
  int minute;
  long inMilliSekunden;
}zuWartendeZeit;

struct zuWartendeZeit WaitingTime;

struct CurrentTime {
  int day;
  char month[4]; // Platz für den Monatsnamen in drei Buchstaben + Nullterminator
  int year;
  int hour;
  int minute;
  int second;
};

struct CurrentTime currentTime;

void stringToIntArray(const String str, int* intArr) {
  for (int i = 0; i < 7; i++) {
    if (str[i] == '0') {
      intArr[i] = 0;
    } else if (str[i] == '1') {
      intArr[i] = 1;
    } else {
      // Fehlerbehandlung, falls der String andere Zeichen enthält
      intArr[i] = -1;
    }
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  // setup ir library
  pinMode(22, OUTPUT);
  IrSender.begin(22);

  // setup time library
  sscanf(__TIME__, "%d:%d:%d", &compileHour, &compileMinute, &compileSecond);
  Serial.println("compile time before: " + String(compileHour) + ":" + String(compileMinute) + ":" + String(compileSecond));
  compileSecond += 23;
  if(compileSecond >= 60) {
    compileSecond -= 60;
    compileMinute += 1;
  }
  if(compileMinute >= 60) {
    compileMinute -= 60;
    compileHour += 1;
  }

  Serial.println("compile time after: " + String(compileHour) + ":" + String(compileMinute) + ":" + String(compileSecond));
  
  sscanf(__DATE__, "%s %d %d", &compileMonth, &compileDay, &compileYear);
  Serial.println("compile date: " + String(__DATE__));
  setTime(compileHour, compileMinute, compileSecond, compileDay, monthToInt(compileMonth), compileYear);

  // SPIFFS starten
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  // ESP32 als Access Point konfigurieren
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP address: ");
  Serial.println(IP);

  // HTTP-Server konfigurieren
  //server.on("/", handleRoot);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("ESP32 Web Server: New request received:");  // for debugging
    Serial.println("GET /");        // for debugging
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/setOn", HTTP_GET, [](AsyncWebServerRequest * request) {
    // time
    Serial.println("on request set: " + String(request->getParam("time")->value()) + ", " + String(request->getParam("week")->value()));
    String t = request->getParam("time")->value();
    Serial.println("time: " + t);
    Serial.println("time sub: " + t.substring(0,2));
    Serial.println("time sub: " + t.substring(3,5));
    paramOn.hours = t.substring(0, 2).toInt();
    paramOn.minutes = t.substring(3, 5).toInt();
    // weekdays
    String days = request->getParam("week")->value();
    stringToIntArray(days, paramOn.weekdays);
    Serial.println("time: " + String(paramOn.hours) + ":" + String(paramOn.minutes));
    Serial.print("week: ");
    for (int i = 0; i < 7; i++) {
      Serial.print(paramOn.weekdays[i]);
    }
    Serial.println();
    // current time: 'Tue Dec 17 2024 15:01:22 GMT+0100 (Mitteleuropäische Normalzeit)'
    String today = request->getParam("currentTime")->value();
    int todaySize = today.length();
    Serial.println("length: " + todaySize);
    char todayChr[todaySize];
    for(int i = 0; i < today.length(); i++) {
      todayChr[i] = today[i];
    }
    Serial.println(String(todayChr));
    const char* todayPtr = todayChr;
    Serial.println("today: " + today);
    sscanf(todayPtr, "%*s %3s %d %d %d:%d:%d %*s", currentTime.month, currentTime.day, currentTime.year, currentTime.hour, currentTime.minute, currentTime.second);
    Serial.println("test");
    Serial.println(currentTime.hour);
    Serial.println("today: " + String(currentTime.hour) + ":" + String(currentTime.minute) + ":" + String(currentTime.second));
    setTime(currentTime.hour, currentTime.minute, currentTime.second, currentTime.day, monthToInt(currentTime.month), currentTime.year);
    // html
    request->send(SPIFFS, "/index.html", "text/html");

    newCommand = true;
  });
  server.on("/setOff", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.print("off request set: " + String(request->getParam("time")->value()) + ", " + String(request->getParam("time")->value()));
    String t = request->getParam("time")->value();
    paramOff.hours = t.substring(0, 2).toInt();
    paramOff.minutes = t.substring(3, 5).toInt();
    String days = request->getParam("week")->value();
    stringToIntArray(days, paramOff.weekdays);
    Serial.println("time: " + String(paramOff.hours) + ":" + String(paramOff.minutes));
    Serial.print("week: ");
    for (int i = 0; i < 7; i++) {
      Serial.print(paramOff.weekdays[i]);
    }
    Serial.println();
    request->send(SPIFFS, "/index.html", "text/html");

    newCommand = true;
  });
  server.on("/setPlaceholder", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.print("placeholder request set: " + String(request->getParam("time")->value()) + ", " + String(request->getParam("time")->value()));
    String t = request->getParam("time")->value();
    paramPlaceholder.hours = t.substring(0, 2).toInt();
    paramPlaceholder.minutes = t.substring(3, 5).toInt();
    String days = request->getParam("week")->value();
    stringToIntArray(days, paramPlaceholder.weekdays);
    Serial.println("time: " + String(paramPlaceholder.hours) + ":" + String(paramPlaceholder.minutes));
    Serial.print("week: ");
    for (int i = 0; i < 7; i++) {
      Serial.print(paramPlaceholder.weekdays[i]);
    }
    Serial.println();
    request->send(SPIFFS, "/index.html", "text/html");
    
    newCommand = true;
  });
  server.begin();
}



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

void loop() {
  // get next command
  struct NextCommand next = getNextCmd();
  newCommand = false;

  // get waiting time
  unsigned long waitInterval = getWaitingTime(next);
  // tmp
  int waitInterval2 = waitInterval-60000;
  if(waitInterval2 < 0) {
    waitInterval2 = 0;
  }
  Serial.println("wait for " + String(waitInterval2));

  // wait
  previousMillis = millis();
  unsigned long currentMillis = millis();
  while (true) {
    // check if new parameters are received
    if (newCommand) {
      break;
    }
    // check if waiting time is reached
    currentMillis = millis();
    float currentWaitTimeTmp = (currentMillis - previousMillis) / 1000;
    Serial.println("waited for " + String(currentWaitTimeTmp));
    if (currentMillis - previousMillis >= waitInterval2) {
      break;
    }
    delay(10000);
  }

  Serial.println("current time: " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
  Serial.println("next time: " + String(next.hour) + ":" + String(next.minute));

  if(newCommand) {
    newCommand = false;
  }else{
    while(true) {
      //Serial.println("current time: " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
       if(hour() == next.hour && minute() == next.minute && second() >= 0) {
        sendNec(next);
        break;
       }
       delay(500);
    }
  }

  Serial.println("finished sending command");
  //delay(20000);
}

void sendNec(const NextCommand& next) {
  Serial.println("send nec command");
  IrSender.sendNECRaw(next.command, 0);
  delay(1000);
  IrSender.sendNECRaw(next.command, 0);
  delay(1000);
  IrSender.sendNECRaw(next.command, 0);
}

NextCommand getNextCommandStruct(const CommandParam paramCmd) {
  Serial.println("-----");
  Serial.println("get next command");
  Serial.println("params: " + String(paramCmd.hours) + ":" + String(paramCmd.minutes));
  struct NextCommand nextCmd = { -1, -1, -1};
  // get current weekday
  int today = weekday() - 1;
  int searchDay = today;
  while (1) {
    // check if command should be sent today with current weekday as index for weekdays array
    if (paramCmd.weekdays[searchDay] == 1) {
      Serial.println("send next command on id " + String(searchDay));
      if (searchDay == today) {
        Serial.println("send next command today");
        // check if current time is later than next command time
        String currentTime = String(hour());
        int minut = minute();
        if (minut < 10) {
          currentTime += "0" + String(minut);
        } else {
          currentTime += String(minut);
        }
        int currentTimeInt = currentTime.toInt();
        String cmdTime = String(paramCmd.hours);        
        if (paramCmd.minutes < 10) {
          cmdTime += "0" + String(paramCmd.minutes);
        } else {
          cmdTime += String(paramCmd.minutes);
        }
        int cmdTimeInt = cmdTime.toInt();
        if (cmdTimeInt > currentTimeInt) {
          Serial.println("send time is later today");
          break;
        }
      } else {
        Serial.println("send next command at some point");
        break;
      }
    }
    // no command should be sent today, check for the next days
    searchDay += 1;
    if (searchDay > 6) {
      searchDay = 0;
      Serial.println("go to sunday");
    }
    // break when current weekday is reached again (no days found)
    if (searchDay == today) {
      nextCmd.day = 1000000;
      nextCmd.hour = 1000000;
      nextCmd.minute = 1000000;
      return nextCmd;
    }
  }
  nextCmd.day = searchDay + 1;
  nextCmd.hour = paramCmd.hours;
  nextCmd.minute = paramCmd.minutes;
  return nextCmd;
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

NextCommand getEarliest(const NextCommand& t1, const NextCommand& t2, const NextCommand& t3) {
  NextCommand earliest = t1;
  if (isEarlier(t2, earliest)) {
    earliest = t2;
  }
  if (isEarlier(t3, earliest)) {
    earliest = t3;
  }
  return earliest;
}

NextCommand getNextCmd(void) {
  Serial.println("start getting next cmd");
  struct NextCommand nextOnCmd = getNextCommandStruct(paramOn);
  nextOnCmd.command = 0x46A88248;
  Serial.println("next on cmd: " + String(nextOnCmd.day) + ", " + String(nextOnCmd.hour) + ":" + String(nextOnCmd.minute));
  Serial.println("-----");
  Serial.println("paramOff: " + String(paramOff.hours) + ":" + String(paramOff.minutes));
  struct NextCommand nextOffCmd = getNextCommandStruct(paramOff);
  nextOffCmd.command = 0x46228248;
  Serial.println("next off cmd: " + String(nextOffCmd.day) + ", " + String(nextOffCmd.hour) + ":" + String(nextOffCmd.minute));
  struct NextCommand nextPlaceholderCmd = getNextCommandStruct(paramPlaceholder);
  nextPlaceholderCmd.command = 0x00000000;
  Serial.println("next 3rd cmd: " + String(nextPlaceholderCmd.day) + ", " + String(nextPlaceholderCmd.hour) + ":" + String(nextPlaceholderCmd.minute));

  struct NextCommand earliest = getEarliest(nextOnCmd, nextOffCmd, nextPlaceholderCmd);
  Serial.println("next command: " + String(earliest.command) + " @ " + String(earliest.day) + ", " + String(earliest.hour) + ":" + String(earliest.minute));
  return earliest;
}

//unsigned long getWaitingTime(const NextCommand& next) {
//  unsigned long time = 10000;
//  return time;
//}

unsigned long getWaitingTime(NextCommand Nachsterbefehl) {

  bool carryMinute = false;
  bool carryStunde = false;
  // Carry resetten
  if (Nachsterbefehl.minute - minute() < 0 )
  {
    // Funktioniert
    WaitingTime.minute =  60 + (Nachsterbefehl.minute - minute());
    carryMinute = true;
    Serial.println("Minuten bei gesetztem Carry");
    Serial.println(WaitingTime.minute);

  }
  else
  {
    WaitingTime.minute =  Nachsterbefehl.minute - minute();
    carryMinute = false;
    Serial.println("Minuten bei nicht gesetztem Carry");
    Serial.println(WaitingTime.minute);
  }

  if (carryMinute == true)
  {
    // eine Stunde kürzer
    if ((Nachsterbefehl.hour - 1) - hour() < 0)
    {
      WaitingTime.Stunden = 24 + (Nachsterbefehl.hour - 1) - hour();
      carryStunde = true; // Ein Tag weniger
      Serial.println("Stunden bei gesetzem Carry");
      Serial.println(WaitingTime.Stunden);
    }
    else
    {
      WaitingTime.Stunden = (Nachsterbefehl.hour - 1) - hour();
      carryStunde = false;
      Serial.println("Positive Stunden bei Carry");
      Serial.println(WaitingTime.Stunden);
    }
  }

  else { // Keine Stunde weniger weil kein Carry
    if (Nachsterbefehl.hour - hour() < 0)
    {
      WaitingTime.Stunden = 24 + (Nachsterbefehl.hour - 1) - hour();
      carryStunde = true; // Ein Tag weniger
      Serial.println("Negative Stunden bei nicht gesetztem Carry");
      Serial.println(WaitingTime.Stunden);
    }
    else
    {
      WaitingTime.Stunden = Nachsterbefehl.hour - hour();
      Serial.println("Postive Stunden bei nicht gesetztem Carry");
      Serial.println(WaitingTime.Stunden);
    }
  }


  if (carryStunde == true) {
    // Tag weniger
    if ((Nachsterbefehl.day - 1) - weekday() < 0)
    {
      WaitingTime.Tage = 7 + (Nachsterbefehl.day - 1 - weekday());
      Serial.println("Negative Tage wenn carry gesetzt");
      Serial.println(WaitingTime.Tage);
    }
    else
    {
      WaitingTime.Tage = (Nachsterbefehl.day - 1) - weekday();
      Serial.println("Positive Tage, wenn carry gesetzt");
      Serial.println(WaitingTime.Tage);

    }
  }

  else {
    if (Nachsterbefehl.day - weekday() < 0)
    {
      WaitingTime.Tage = 7 + Nachsterbefehl.day - weekday();
      Serial.println(" negative Tage, wenn carry nicht gesetzt");
      Serial.println(WaitingTime.Tage);
    }
    else
    {
      WaitingTime.Tage = Nachsterbefehl.day - weekday();
      Serial.println("Positive Tage, wenn carry nicht gesetzt");
      Serial.println(WaitingTime.Tage);
    }
  }



  // Berechnung der Zeit in ms

  WaitingTime.inMilliSekunden = WaitingTime.minute * 60 * 1000 + WaitingTime.Stunden * 60 * 60 * 1000 + WaitingTime.Tage * 60 * 60 * 24 * 1000 ;


  return WaitingTime.inMilliSekunden;
};
