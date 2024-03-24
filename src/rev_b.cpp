#include <Arduino.h>
// zip firmware.bin from the .pio folder to firmware.zip
// convert firmware.zip a header using this tool: https://honeythecodewitch.com/gfx/converter
// set the output type to C/C++ and the name to firmware_rev_b
void setup() {
  Serial.begin(115200);
  Serial.println("Hello from revision B");
}

void loop() {

}
