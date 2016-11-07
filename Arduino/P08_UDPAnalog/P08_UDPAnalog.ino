/*
UDP Analog Sensor.
Change SSID and PASSWORD.
*/

#define SSID "5673RM11"
#define PASSWORD "frankmarcel"

#define DEBUG true

#define LED_WLAN 13

#define LED 9
#define SENSOR A6

#include <SoftwareSerial.h>

SoftwareSerial esp8266(11, 12); // RX, TX

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  //Long Timeout needed for RST and settings of WLAN-Data
  esp8266.setTimeout(5000);

  if (sendCom("AT+RST", "ready")) {
    debug("RESET OK");
  }

  if (configStation(SSID, PASSWORD)) {
    debug("WLAN Connected");
    esp8266.setTimeout(1000); //Shorter for CIFSR
    digitalWrite(LED_WLAN, HIGH);
    debug("My IP is:");
    debug(sendCom("AT+CIFSR"));
  }
  if (configUDP()) {
    debug("UDP ready");
  }
  //shorter Timeout for faster wrong UPD-Comands handling
  esp8266.setTimeout(1000);

  pinMode(LED, OUTPUT);
  pinMode(SENSOR, INPUT);
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
        sendUDP("LED=" + String(setLed));
      }
      else {
        debug("Wrong UDP Command");
        sendUDP("Wrong UDP Command");
      }
    }
  }

  sendUDP("Sensor=" + String(analogRead(SENSOR)));
  // delay(500);
}


//-----------------------------------------Config ESP8266------------------------------------

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
