/*
Software Serial Examples.
No change necessary.
 */
#include <SoftwareSerial.h>

SoftwareSerial esp8266(11, 12);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  // set the data rate for the SoftwareSerial port
  esp8266.begin(19200);
  
  //send first AT Command to see if ESP8266-Module responds
  esp8266.println("AT");
}

void loop() // run over and over
{
  if (esp8266.available())
    Serial.write(esp8266.read());
  if (Serial.available())
    esp8266.write(Serial.read());
}


