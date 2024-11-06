
/* In diesem Sketch sollen Befehle des NEC-Protokolls implementiert werden
https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol

Das Protokoll ist folgend aufgebaut:

Start:
9 ms high + 4,5 ms low

Adresse 8 bit 
Adresse inverse 8 bit

Command 8 Bit 
Command inverse 8 Bit

Ein Bit ist folgend aufgebaut: 
1: 562,5 micro S High + 1,6875 micro S low
0: 562,5 micro S high + 562,5 micro S low 



*/

const int SendPin = 22;

 void startBit(void){
  digitalWrite(SendPin,HIGH);
  delay(9);
  digitalWrite(SendPin,LOW);
  delay(4.5);
 }

 void send1(void){
  digitalWrite(SendPin,HIGH);
  delay(0.5625);
  digitalWrite(SendPin,LOW);
  delay(1.6875);
 }

  void send0(void){
  digitalWrite(SendPin,HIGH);
  delay(0.5625);
  digitalWrite(SendPin,LOW);
  delay(0.5625);
 }



void setup() {
  // put your setup code here, to run once:
 pinMode(SendPin,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 startBit();
 send1();
 send0();
}
