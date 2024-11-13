#include <Arduino.h>
#include <IRremote.hpp> // include the library
#include <TimeLib.h>

int compileHour, compileMinute, compileSecond, compileYear, compileDay;
char compileMonth[4];

int sendHour = 11;
int sendMinute = 44;
int sendSecond = 0;

void setup() {
  // set output pin
  pinMode(22, OUTPUT);

  Serial.begin(115200);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.

  // setup ir library
  IrSender.begin(22); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
  disableLEDFeedback(); // Disable feedback LED at default feedback LED pin


  sscanf(__TIME__, "%d:%d:%d", &compileHour, &compileMinute, &compileSecond);
  sscanf(__DATE__, "%s %d %d", &compileMonth, &compileDay, &compileYear);
  setTime(compileHour, compileMinute, compileSecond, compileDay, monthToInt(compileMonth), compileYear);
}

int monthToInt(const char* month){
  // Array der Monatsnamen
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  // Durchlaufen des Arrays und Vergleich der Monatsnamen
  for (int i = 0; i < 12; ++i) {
    if (strncmp(month, months[i], 3) == 0) {
      return i+1; // Monat gefunden, Index zurückgeben
    } 
  } // Falls der Monat nicht gefunden wird, -1 zurückgeben
  return -1;
}

void loop() {  
  Serial.println(weekday());
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.println(second());
  Serial.println();

  if(hour() == sendHour && minute() == sendMinute && second() == sendSecond){
    Serial.println("send nec command");
  }
  
  delay(1000);
  //IrSender.sendNECRaw(0x46228248, sRepeats); // inverted off
}
