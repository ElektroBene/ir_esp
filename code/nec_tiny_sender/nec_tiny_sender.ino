#include "TinyIRSender.hpp"

void setup() {
  sendNEC(22, 0, 11, 2); // Send address 0 and command 11 on pin 3 with 2 repeats.
}

void loop() {
  sendNEC(22, 0, 11, 2);
}
