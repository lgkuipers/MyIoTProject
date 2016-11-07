/*
TCP-Server Brightness Sensor (Realativ)
Change SSID and PASSWORD.
*/
#define SSID "5673RM11"
#define PASSWORD "frankmarcel"


#define LED_WLAN 13

#define SENSOR A6

#define DEBUG true


#include <SoftwareSerial.h>

SoftwareSerial esp8266(11, 12); // RX, TX

const char site[] PROGMEM = {
 "<HTML><HEAD>\n<link rel=\"icon\" href=\"data:;base64,iVBORw0KGgo=\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=2.0, user-scalable=yes\">\n<meta http-equiv=\"refresh\" content=\"3\">\n<title>\nBrightness\n</title>\n</HEAD>\n<BODY bgcolor=\"#FFFF99\" text=\"#000000\">\n<FONT size=\"6\" FACE=\"Verdana\">\nBrightness\n</FONT>\n<FONT size=\"3\" FACE=\"Verdana\">\n<HR>\n<BR>\n\t<label for=\"hell\">Current Brightness: *bright*</label> <BR>\n\t<input type=\"range\"  max = \"1024\" name=\"hell\" value=*bright*><BR>\n<BR>\n<HR>\n</font>\n"
};

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  if (configTCPServer())  debug("Server Aktiv"); else debug("Server Error");
}

void loop() {
  String xBuffer;
  if (esp8266.available()) // check if the esp is sending a message
  {
    if (esp8266.find("+IPD,"))
    {
      debug("Incomming Request");
      int connectionId = esp8266.parseInt();

      if (sendWebsite(connectionId, createWebsite())) debug("Website send OK"); else debug("Website send Error");
    }

  }
}

boolean sendWebsite(int connectionId, String webpage)
{
  boolean success = true;

  if (sendCom("AT+CIPSEND=" + String(connectionId) + "," + String(webpage.length()), ">"))
  {
    esp8266.print(webpage);
    esp8266.find("SEND OK");
    success &= sendCom("AT+CIPCLOSE=" + String(connectionId), "OK");
  }
  else
  {
    success = false;
  }
  return success;
}

String createWebsite()
{
  String xBuffer;

  for (int i = 0; i <= sizeof(site); i++)
  {
    char myChar = pgm_read_byte_near(site + i);
    xBuffer += myChar;
  }

  xBuffer.replace("*bright*", String(analogRead(SENSOR)));

  return xBuffer;
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
