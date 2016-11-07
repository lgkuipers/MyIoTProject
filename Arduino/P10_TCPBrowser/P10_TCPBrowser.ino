/*
TCP Browser.
Change SSID and PASSWORD.
*/

#define SSID "5673RM11"
#define PASSWORD "frankmarcel"

#define DEBUG true

#define LED_WLAN 13

#include <SoftwareSerial.h>

SoftwareSerial esp8266(11, 12); // RX, TX

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);
}

void loop() {
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
  success &= sendCom("AT+CIPMODE=0", "OK");  //So rum scheit wichtig!
  success &= sendCom("AT+CIPMUX=0", "OK");

  return success;
}

boolean configTCPServer()
{
  boolean success = true;

  success &= (sendCom("AT+CIPMUX=1", "OK"));
  success &= (sendCom("AT+CIPSERVER=1,80", "OK"));

  return success;

}

boolean configTCPClient()
{
  boolean success = true;

  success &= (sendCom("AT+CIPMUX=0", "OK"));
  //success &= (sendCom("AT+CIPSERVER=1,80", "OK"));

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

boolean configUDP()
{
  boolean success = true;

  success &= (sendCom("AT+CIPMODE=0", "OK"));
  success &= (sendCom("AT+CIPMUX=0", "OK"));
  success &= sendCom("AT+CIPSTART=\"UDP\",\"192.168.255.255\",90,91,2", "OK"); //Importand Boradcast...Reconnect IP
  return success;
}




//-----------------------------------------------Controll ESP-----------------------------------------------------

boolean sendUDP(String Msg)
{
  boolean success = true;

  success &= sendCom("AT+CIPSEND=" + String(Msg.length() + 2), ">");    //+",\"192.168.4.2\",90", ">");
  if (success)
  {
    success &= sendCom(Msg, "OK");
  }
  return success;
}


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
