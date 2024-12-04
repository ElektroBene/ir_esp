#include <TimeLib.h>






// hier soll ich für benedikt die zu wartende Zeit herausfinden
// es muss auch der Zustand einer Flag abgefragt werden,
// mit der man abfragen kann ob ein befehl gesendet wurde

// für den nächsten Befehl bekomme ich einen Index


/*
  struct NextCommand {
  int day;
  // das bekomme ich von Benedikt zurückk um zu wissen, an welchem Tag der nächste Befehl zu senden ist.
  int hour;
  int minute;
  };


  bool neuerBefehl;



  struct zuWartendeZeit
  {
  int Tage;
  int Stunden;
  int minute;
  }

  struct zuWartendeZeit WaitingTime;




  struct zuWartendeZeit WarteZeit(struct NextCommand Next)
  {

    if(Next.day - weekday() =< 0)
    {
      // warte Bis zur Ende der Woche
      zuWartendeZeit.Tage = -1;
    }
    else
    {
       zuWartendeZeit.Tage = Next.day - weekday();
    }
    if(Next.hour - hour() =< 0)
    {
      // warte bis zum nächsten Tag
      zuWartendeZeit.Stunde = -1;
    }
    else {
      zuWartendeZeit.Stunde = Next.hour - hour();
    }
    if(Next.minute - minute() =< 0)
      // warte bis zur nächsten Vollenstunde
      zuWartendeZeit.Minute = -1;
  }
  else
  {
    zuWartendeZeit.Minute = Next.minute - minute();
  }

  }


*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct NextCommand {
  int day;
  // das bekomme ich von Benedikt zurückk um zu wissen, an welchem Tag der nächste Befehl zu senden ist.
  int hour;
  int minute;
};

/*
  enum bool{
  FALSE,
  TRUE
  } bool;
  // C kennt keinen Bool
*/

bool FlagNeuerBefehl = true;

bool carryMinute = false;
bool carryStunde = false;

typedef struct zuWartendeZeit
{
  int Tage;
  int Stunden;
  int minute;
  long inMilliSekunden;
} zuWartendeZeit;

struct zuWartendeZeit WaitingTime;




/*
  struct zuWartendeZeit WarteZeit(struct NextCommand Next)
  {

    if(Next.day - weekday() =< 0)
    {
      // warte Bis zur Ende der Woche
      zuWartendeZeit.Tage = -1;
    }
    else
    {
       zuWartendeZeit.Tage = Next.day - weekday();
    }
    if(Next.hour - hour() =< 0)
    {
      // warte bis zum nächsten Tag
      zuWartendeZeit.Stunde = -1;
    }
    else {
      zuWartendeZeit.Stunde = Next.hour - hour();
    }
    if(Next.minute - minute() =< 0)
      // warte bis zur nächsten Vollenstunde
      zuWartendeZeit.Minute = -1;
  }
  else
  {
    zuWartendeZeit.Minute = Next.minute - minute();
  }

  }

*/

struct NextCommand NB = {3, 11, 26};



// Ich bekomme den nachsten Zeitpubkt und möchte die Wartezeit in ms zurückgeben

unsigned long ErmittelwarteZeit(NextCommand Nachsterbefehl) {

  carryMinute = false;
  carryStunde = false;

  if (Nachsterbefehl.minute - minute() < 0 )
  {
    WaitingTime.minute =  60 + (Nachsterbefehl.minute - minute());
    carryMinute = true;
    Serial.println("Minuten bei gesetztem Carry");
    Serial.println(WaitingTime.minute);

  }
  else {
    WaitingTime.minute =  Nachsterbefehl.minute - minute();
    carryMinute = false;
    Serial.println("Minuten bei nicht gesetztem Carry");
    Serial.println(WaitingTime.minute);
  }

  if (carryMinute == true) {
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
      Serial.println("Stunden bei nicht Carry");
      Serial.println(WaitingTime.Stunden);
    }
  }

  else { // Keine Stunde weniger weil kein Carry
    if (Nachsterbefehl.hour - hour() < 0)
    {
      WaitingTime.Stunden = 24 + (Nachsterbefehl.hour - 1) - hour();
      carryStunde = true; // Ein Tag weniger
      Serial.println("Stunden bei gesetzem Carry");
      Serial.println(WaitingTime.Stunden);
    }
    else
    {
      WaitingTime.Stunden = Nachsterbefehl.hour - hour();
      Serial.print("Stunden ohne Carry weniger");
      Serial.println(WaitingTime.Stunden);
    }
  }


  if (carryStunde == true) {
    // Tag weniger
    if ((Nachsterbefehl.day - 1) - weekday() < 0)
    {
      WaitingTime.Tage = 7 + (Nachsterbefehl.day - 1) - weekday();
      Serial.println("Tage wenn carry gesetzt");
      Serial.println(WaitingTime.Tage);
    }
    else
    {
      WaitingTime.Tage = (Nachsterbefehl.day - 1) - weekday();
      Serial.println("Tage, wenn carry nicht gesetzt");
      Serial.println(WaitingTime.Tage);

    }
  }

  else {
    if (Nachsterbefehl.day - weekday() < 0)
    {
      WaitingTime.Tage = 7 + Nachsterbefehl.day - weekday();
      Serial.println("Tage, wenn carry nicht gesetzt");
      Serial.println(WaitingTime.Tage);
    }
    else
    {
      WaitingTime.Stunden = Nachsterbefehl.day - weekday();
      Serial.println("Tage, wenn carry nicht gesetzt");
      Serial.println(WaitingTime.Tage);
    }
  }

  Serial.println("Waiting time: " + String(WaitingTime.Tage) + ":" + String(WaitingTime.Stunden) + ":" + String(WaitingTime.minute));

  // Berechnung der Zeit in ms
  return 1;
};


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
  Serial.println("Hallo");
  Serial.println(weekday());
  Serial.println(hour());

  // setup time library
  int compileHour, compileMinute, compileSecond, compileYear, compileDay;
  char compileMonth[4];
  sscanf(__TIME__, "%d:%d:%d", &compileHour, &compileMinute, &compileSecond);
  sscanf(__DATE__, "%s %d %d", &compileMonth, &compileDay, &compileYear);
  setTime(compileHour, compileMinute, compileSecond, compileDay, monthToInt(compileMonth), compileYear);

}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned int waitingTime = ErmittelwarteZeit(NB);
  Serial.println(waitingTime);
  delay(10000);
}
