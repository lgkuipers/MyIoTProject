/*
Google-Ping Example.
Change SSID and PASSWORD
 */

#define SSID "5673RM11"
#define PASSWORD "frankmarcel"

#define DEBUG true

#include <SoftwareSerial.h>

SoftwareSerial esp8266(11, 12);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  // set the data rate for the SoftwareSerial port
  esp8266.begin(19200);

  if (!espConfig()) serialDebug();
  else debug("Config OK");

  if (sendCom("AT+PING=\"www.google.de\"", "OK"))
  {
    Serial.println("Ping OK");
    digitalWrite(13, HIGH);
  }
  else
  {
    Serial.println("Ping Error");
  }
}

void loop() // run over and over
{
  //Start serial Debug Mode - Type Commandos over serial Monitor
  serialDebug();
}

//-----------------------------------------Config ESP8266------------------------------------

boolean espConfig()
{
  boolean success = true;
  esp8266.setTimeout(5000);
  success &= sendCom("AT+RST", "ready");
  esp8266.setTimeout(1000);
  if (configStation(SSID, PASSWORD)) {
    success &= true;
    debug("WLAN Connected");
    debug("My IP is:");
    debug(sendCom("AT+CIFSR"));
  }
  else
  {
    success &= false;
  }
  //shorter Timeout for faster wrong UPD-Comands handling
  success &= sendCom("AT+CIPMODE=0", "OK");
  success &= sendCom("AT+CIPMUX=0", "OK");

  return success;
}


boolean configStation(String vSSID, String vPASSWORT)
{
  boolean success = true;
  success &= (sendCom("AT+CWMODE=1", "OK"));
  esp8266.setTimeout(20000);
  success &= (sendCom("AT+CWJAP=\"" + String(vSSID) + "\",\"" + String(vPASSWORT) + "\"", "OK"));
  esp8266.setTimeout(1000);
  return success;
}

boolean configAP()
{
  boolean success = true;

  success &= (sendCom("AT+CWMODE=2", "OK"));
  success &= (sendCom("AT+CWSAP=\"NanoESP\",\"\",5,0", "OK"));

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
