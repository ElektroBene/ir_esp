#include <Arduino.h>
#include <IRremote.hpp> // include the library
#include <TimeLib.h>
#include <WiFi.h>
#include <ESPAsyncWebSrv.h>
#include <SPIFFS.h>

const char* ssid = "iPhone von Benedikt (2)"; // CHANGE IT
const char* password = "bene12345"; // CHANGE IT

/*
const char* ssid = "WG-LAN";
const char* password = "FredTheFridge!RIP";
*/



int compileHour, compileMinute, compileSecond, compileYear, compileDay;
char compileMonth[4];

bool newCommand = false;

unsigned int previousMillis = 0;

struct CommandParam {
  int time[2];
  int weekdays[7]; // with sunday at index 0
};

struct NextCommand {
  uint32_t command;
  int day;
  int hour;
  int minute;
};

struct CommandParam paramOn = {{10, 30}, {0, 0, 1, 1, 1, 0, 0}};
struct CommandParam paramOff = {{16, 30}, {0, 0, 1, 1, 1, 0, 0}};
struct CommandParam paramPlaceholder = {{0, 0}, {0, 0, 0, 0, 0, 0, 0}};

void setupNetwork(void) {
  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
      Serial.print(n);
      Serial.println(" networks found");
      Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
      for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          Serial.printf("%2d",i + 1);
          Serial.print(" | ");
          Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
          Serial.print(" | ");
          Serial.printf("%4d", WiFi.RSSI(i));
          Serial.print(" | ");
          Serial.printf("%2d", WiFi.channel(i));
          Serial.print(" | ");
          switch (WiFi.encryptionType(i))
          {
          case WIFI_AUTH_OPEN:
              Serial.print("open");
              break;
          case WIFI_AUTH_WEP:
              Serial.print("WEP");
              break;
          case WIFI_AUTH_WPA_PSK:
              Serial.print("WPA");
              break;
          case WIFI_AUTH_WPA2_PSK:
              Serial.print("WPA2");
              break;
          case WIFI_AUTH_WPA_WPA2_PSK:
              Serial.print("WPA+WPA2");
              break;
          case WIFI_AUTH_WPA2_ENTERPRISE:
              Serial.print("WPA2-EAP");
              break;
          case WIFI_AUTH_WPA3_PSK:
              Serial.print("WPA3");
              break;
          case WIFI_AUTH_WPA2_WPA3_PSK:
              Serial.print("WPA2+WPA3");
              break;
          case WIFI_AUTH_WAPI_PSK:
              Serial.print("WAPI");
              break;
          default:
              Serial.print("unknown");
          }
          Serial.println();
          delay(10);
      }
  }
  Serial.println("");

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Print the ESP32's IP address
  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.

  // setup ir library
  pinMode(22, OUTPUT);
  IrSender.begin(22);
  
  // setup time library
  sscanf(__TIME__, "%d:%d:%d", &compileHour, &compileMinute, &compileSecond);
  sscanf(__DATE__, "%s %d %d", &compileMonth, &compileDay, &compileYear);
  setTime(compileHour, compileMinute, compileSecond, compileDay, monthToInt(compileMonth), compileYear);

  // setup wlan connection
  setupNetwork();

  // initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // define routes to serve html page and get parameters
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:");  // for debugging
    Serial.println("GET /");        // for debugging
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/setOn", HTTP_GET, [](AsyncWebServerRequest *request){
    paramOn.time = request->getParam("time")->value();
    paramOn.weekdays = request->getParam("week")->value();
    newCommand = true;
    Serial.print("on request set: " + paramOn.time + ", " + paramOn.weekdays);
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/setOff", HTTP_GET, [](AsyncWebServerRequest *request){
    paramOff.time = request->getParam("time")->value();
    paramOff.weekdays = request->getParam("week")->value();
    newCommand = true;
    Serial.print("off request set: " + paramOff.time + ", " + paramOff.weekdays);
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/setPlaceholder", HTTP_GET, [](AsyncWebServerRequest *request){
    paramPlaceholder.time = request->getParam("time")->value();
    paramPlaceholder.weekdays = request->getParam("week")->value();
    newCommand = true;
    Serial.print("placeholder request set: " + paramPlaceholder.time + ", " + paramPlaceholder.weekdays);
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Start the server
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
  Serial.println("wait for " + String(waitInterval));
  
  // wait
  previousMillis = millis();
  unsigned long currentMillis = millis();
  while(true) {
    // check if new parameters are received
    if (newCommand) {
      break;
    }
    // check if waiting time is reached
    currentMillis = millis();
    float currentWaitTimeTmp = (currentMillis - previousMillis)/1000;
    Serial.println("waited for " + String(currentWaitTimeTmp));
    if (currentMillis - previousMillis >= waitInterval) {
      sendNec(next);
      break;
    }
  }

  Serial.println("finished sending command");
  delay(19000);
}

void sendNec(void) {
  Serial.println("send nec command");
  IrSender.sendNECRaw(0x46228248, sRepeats);
  delay(1000);
  IrSender.sendNECRaw(0x46228248, sRepeats);
  delay(1000);
  IrSender.sendNECRaw(0x46228248, sRepeats);
}

NextCommand getNextCommandStruct(const CommandParam& paramCmd) {
  struct NextCommand nextCmd = {-1, -1, -1};
  // get current weekday
  int today = weekday() - 1;
  int searchDay = today;
  while (1) {
    // check if command should be sent today with current weekday as index for weekdays array
    if (paramCmd.weekdays[searchDay] == 1) {
      if (searchDay == today) {
        // check if current time is later than next command time
        String currentTime = String(hour());
        if (minute() < 10) {
          currentTime += "0" + String(minute());
        }else {
          currentTime += String(minute());
        }
        int currentTimeInt = currentTime.toInt();
        String cmdTime = String(paramCmd.time[0]);
        if (paramCmd.time[1] < 10) {
          cmdTime += "0" + String(paramCmd.time[1]);
        }else {
          cmdTime += String(paramCmd.time[1]);
        }
        int cmdTimeInt = cmdTime.toInt();
        if (cmdTimeInt > currentTimeInt) {
          break;
        }
      }else{
        break;
      }
    }
    // no command should be sent today, check for the next days
    searchDay += 1;
    if (searchDay > 6) {
      searchDay = 0;
    }
    // break when current weekday is reached again (no days found)
    if (searchDay == today) {
      nextCmd.day = 1000000;
      nextCmd.hour = 1000000;
      nextCmd.minute = 1000000;
      return nextCmd;
    }
  }
  nextCmd.day = searchDay;
  nextCmd.hour = paramOn.time[0];
  nextCmd.minute = paramOn.time[1];
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
  struct NextCommand nextOffCmd = getNextCommandStruct(paramOff);
  nextOffCmd.command = 0x46228248;
  struct NextCommand nextPlaceholderCmd = getNextCommandStruct(paramPlaceholder);
  nextPlaceholderCmd.command = 0x00000000;

  struct NextCommand earliest = getEarliest(nextOnCmd, nextOffCmd, nextPlaceholderCmd);
  Serial.println("next command: " + + String(earliest.command) + " @ " + String(earliest.day) + ", " + String(earliest.hour) + ":" + String(earliest.minute));
  return earliest;
}

unsigned long getWaitingTime(const NextCommand& next) {
  unsigned long time = 10000;
  return time;
}
