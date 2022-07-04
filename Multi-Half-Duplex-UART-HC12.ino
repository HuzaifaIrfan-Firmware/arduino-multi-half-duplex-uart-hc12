

#include "Multi_Half_Duplex_UART.h"

Multi_Half_Duplex_UART serial(9600);

void setup() {
serial.setup();

}

int count =0;

long long unsigned timepassed=millis();


void loop() {

serial.loop();


if ((millis() - timepassed)>1000){
  serial.intialize_sending(20,"Hi from 10:"+String(count));
  count ++;
  timepassed=millis();
}


}
