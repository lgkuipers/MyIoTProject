/*
UDP Controll LED.
No change necessary.
*/

#define DEBUG true

#define LED 8

#include <SoftwareSerial.h>

SoftwareSerial esp8266(11, 12); // RX, TX

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);
  
  esp8266.setTimeout(5000);
  if (sendCom("AT+RST", "ready")) {
    debug("RESET OK");
  }

  if (configAP()) {
    debug("AP ready");
  }
  if (configUDP()) {
    debug("UDP ready");
  }
  
  //shorter Timeout for faster wrong UPD-Comands handling
    esp8266.setTimeout(1000);
   
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    
}

void loop() {
  if (esp8266.available())
  {
    if (esp8266.find("+IPD,"))
    {
      if (esp8266.find("led")) {
        int setLed = esp8266.parseInt();
        digitalWrite(LED, setLed);

        debug("LED=" + String(setLed));
        if (sendCom("AT+CIPSEND=7", ">"))
        {
          sendCom("LED=" + String(setLed), "OK");
        }
      }

      else {
        debug("Wrong UDP Command");
        if (sendCom("AT+CIPSEND=19", ">"))
        {
          sendCom("Wrong UDP Command", "OK");
        }
 
      }
    }
  }
}

//-----------------------------------------Config ESP8266------------------------------------

boolean configAP()
{
  boolean success = true;

  success &= (sendCom("AT+CWMODE=2", "OK"));
  success &= (sendCom("AT+CWSAP=\"NanoESP\",\"\",5,0", "OK"));

  return success;
}

boolean configUDP()
{
  boolean success = true;

  success &= (sendCom("AT+CIPMODE=0", "OK"));
  success &= (sendCom("AT+CIPMUX=0", "OK"));
  success &= sendCom("AT+CIPSTART=\"UDP\",\"192.168.4.255\",90,91", "OK"); //UDP Bidirectional and Broadcast
  return success;
}

//-----------------------------------------------Controll ESP-----------------------------------------------------

boolean sendCom(String command, char respond[])
{
  esp8266.println(command);
  if (esp8266.findUntil(respond, "ERROR"))
  {
    return true;
  }
  else
  {
    debug("ESP SEND ERROR: " + command);
    return false;
  }
}

String sendCom(String command)
{
  esp8266.println(command);
  return esp8266.readString();
}

//-------------------------------------------------Debug Functions------------------------------------------------------
void serialDebug() {
  while (true)
  {
    if (esp8266.available())
      Serial.write(esp8266.read());
    if (Serial.available())
      esp8266.write(Serial.read());
  }
}

void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println(Msg);
  }
}
