#include <Arduino.h>
#include "wiring_private.h"

// See datasheet Section 2.1.2. for more commands

const uint8_t
msgLen = 3,
mean10sec[msgLen] = {0x81, 0x11, 0x6E}, // 10 sec mean, 1 sec update
mean1min[msgLen] = {0x81, 0x12, 0x6D}, // 1 min mean, 10 sec update
mean5min[msgLen] = {0x81, 0x12, 0x6D}, // 5 min mean, 1 min update
temp_rhum[msgLen] = {0x81, 0x14, 0x6B},// temperature and humidity
sleep_power[msgLen] = {0x81, 0x16, 0x69};// sleep or power on

// See datasheet section 2.1.3 for message description
const uint8_t bufferLen = 16;
uint8_t buffer[bufferLen]; //sure that no name conflict with "buffer"?
inline uint16_t buff2word(uint8_t n) {
return (buffer[n] << 8) | buffer[n + 1];
}

Uart SerialNPM (&sercom3, 1, 0, SERCOM_RX_PAD_1, UART_TX_PAD_0); //https://www.arduino.cc/en/Tutorial/SamdSercom

void setup() {
Serial.begin(9600); //Normal Serial RX = 13 TX = 14
SerialNPM.begin(115200, SERIAL_8E1); // NextPM on Serial1 (RX=0; TX=1)//

pinPeripheral(1, PIO_SERCOM); //Assign RX function to pin 1
pinPeripheral(0, PIO_SERCOM);


}


void loop() {
// read the NextPM sensor
SerialNPM.write(mean10sec, msgLen); // request PM data

delay(350);

while (SerialNPM.available() > 0) { //my methode

unsigned int r = SerialNPM.readBytes(buffer, 16); // read PM data

if (r == 16) {
  String reader = "Read: ";
  for (int i = 0; i < 16; i++){
    reader += "0x";
    if (buffer[i] < 0x10)
    reader += "0";
    reader += String(buffer[i], HEX);
    if (i != 15) {
      reader += ", ";
    }
  }

  uint16_t pm01 = word(buffer[9],buffer[10]); //it is x10 here! //Does it work on MKR????
  uint16_t pm25 = word(buffer[11],buffer[12]);
  uint16_t pm10 =  word(buffer[13],buffer[14]);

Serial.print(F("PM1.0 ")); Serial.print(pm01, 1); Serial.print(F(", "));
Serial.print(F("PM2.5 ")); Serial.print(pm25, 1); Serial.print(F(", "));
Serial.print(F("PM10 ")) ; Serial.print(pm10, 1); Serial.print(F(" [ug/m3], "));

}

}
// decode PM message, see datasheet section 2.1.3

SerialNPM.write(temp_rhum, msgLen); // request T/RH data
while (SerialNPM.available() > 0) { //just to wait but the rest out of the while loop...
  SerialNPM.readBytes(buffer, 8); // read T/RH data
}
// decode T/RH message, see datasheet section 2.1.3

float temp = buff2word(3);//  * 0.9754 - 4.2488;
float rhum = buff2word(5);// * 1.1768 - 4.727;

// print the results

Serial.print(temp, 2); Serial.print(F("C, "));
Serial.print(rhum, 2); Serial.println(F("%rh"));

// wait for 10 seconds
delay(10000);
}

void SERCOM3_Handler()
{
SerialNPM.IrqHandler();
}