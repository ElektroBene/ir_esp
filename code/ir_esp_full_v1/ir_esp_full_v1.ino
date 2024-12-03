#include <Arduino.h>
#include <IRremote.hpp> // include the library
#include <TimeLib.h>

int compileHour, compileMinute, compileSecond, compileYear, compileDay;
char compileMonth[4];

struct CommandParam {
  int time[2];
  int weekdays[7]; // with sunday at index 0
};

struct NextCommand {
  int day;
  int hour;
  int minute;
};

struct CommandParam paramOn = {{10, 30}, {0, 0, 1, 0, 1, 0, 0}};
struct CommandParam paramOff = {{16, 30}, {0, 0, 1, 0, 1, 0, 0}};
struct CommandParam paramPlaceholder = {{0, 0}, {0, 0, 0, 0, 0, 0, 0}};

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.

  sscanf(__TIME__, "%d:%d:%d", &compileHour, &compileMinute, &compileSecond);
  sscanf(__DATE__, "%s %d %d", &compileMonth, &compileDay, &compileYear);
  setTime(compileHour, compileMinute, compileSecond, compileDay, monthToInt(compileMonth), compileYear);
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
  Serial.println("Weekday: " + String(weekday()));
  Serial.println("Time: " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
  Serial.println("get next cmd");
  struct NextCommand next = getNextCmd();
  Serial.println("final next command: " + String(next.day) + ", " + String(next.hour) + ":" + String(next.minute));
  //if(weekday() == next.day && hour() == next.hour && minute() == next.minute){
  //  Serial.println("send nec command");
  //}
  delay(5000);
}

/*
  bool isZero(int days[7]) {
  for (int i = 0; i < 7; i++) {
    if (days[i] != 0) {
      return false; // Wenn ein Eintrag nicht 0 ist, gibt false zurück
    }
  }
  return true; // Wenn alle Einträge 0 sind, gibt true zurück
  }
*/

NextCommand getNextCommandStruct(const CommandParam& paramCmd) {
  Serial.println("start searching for next command");
  struct NextCommand nextCmd = {-1, -1, -1};
  // get current weekday
  int today = weekday() - 1;
  Serial.println("today:" + String(today));
  int day = today;
  while (1) {
    // check if command should be sent today with current weekday as index for weekdays array
    Serial.println("day: " + String(day));
    if (paramCmd.weekdays[day] == 1) {
      Serial.println("weekday found");
      Serial.println(day);
      Serial.println(today);
      if (day == today) {
        // check if current time is later than next command time
        String currentTime = String(hour()) + String(minute());
        int currentTimeInt = currentTime.toInt();
        String cmdTime = String(paramCmd.time[0]) + String(paramCmd.time[1]);
        int cmdTimeInt = cmdTime.toInt();
        Serial.println("command time: " + String(cmdTimeInt));
        Serial.println("current time: " + String(currentTime));
        if (cmdTimeInt > currentTimeInt) {
          break;
        }
      } else {
        break;
      }
      break;
    }
    // no command should be sent today, check for the next days
    day += 1;
    if (day > 6) {
      day = 0;
    }
    // break when current weekday is reached again (no days found)
    if (day == today) {
      Serial.println("no day found");
      break;
    }
  }
  if (day != today) {
    nextCmd.day = day;
    nextCmd.hour = paramOn.time[0];
    nextCmd.minute = paramOn.time[1];
  }
}

bool isEarlier(const NextCommand& a, const NextCommand& b) {
  if (a.day != b.day) {
    return a.day < b.day;
  } if (a.hour != b.hour) {
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
  Serial.println("start get next cmd");
  struct NextCommand nextOnCmd = getNextCommandStruct(paramOn);
  struct NextCommand nextOffCmd = getNextCommandStruct(paramOff);
  struct NextCommand nextPlaceholderCmd = getNextCommandStruct(paramPlaceholder);

  struct NextCommand earliest = getEarliest(nextOnCmd, nextOffCmd, nextPlaceholderCmd);

  Serial.println("next on command: " + String(nextOnCmd.day) + ", " + String(nextOnCmd.hour) + ":" + String(nextOnCmd.minute));
  Serial.println("next off command: " + String(nextOffCmd.day) + ", " + String(nextOffCmd.hour) + ":" + String(nextOffCmd.minute));
  Serial.println("next 3rd command: " + String(nextPlaceholderCmd.day) + ", " + String(nextPlaceholderCmd.hour) + ":" + String(nextPlaceholderCmd.minute));
  Serial.println("earliest command: " + String(earliest.day) + ", " + String(earliest.hour) + ":" + String(earliest.minute));
  return earliest;
}
