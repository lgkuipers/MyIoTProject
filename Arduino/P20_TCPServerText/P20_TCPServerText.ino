/*
TCP-Server for the Textdisplay
Change SSID and PASSWORD.
*/

#define SSID "5673RM11"
#define PASSWORD "frankmarcel"

#define LED_WLAN 13

#define PIEZO 8

#define DEBUG true

const char site[] PROGMEM = {
  "<HTML><HEAD>\n<link rel=\"icon\" href=\"data:;base64,iVBORw0KGgo=\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=2.0, user-scalable=yes\">\n<title>Text</title>\n</HEAD>\n<BODY bgcolor=\"#FFFF99\" text=\"#000000\">\n<FONT size=\"6\" FACE=\"Verdana\">Display Text</FONT>\n<HR><BR>\n<FONT size=\"3\" FACE=\"Verdana\">\nInput display text\n<BR>\n<BR>\n<form method=\"GET\">\n<input type=\"text\" name=\"text\"  size=\"16\" maxlength=\"16\">\n<BR>\n<input type=\"submit\">\n</form>\n<BR>\n<HR>\n</font>\n</HTML>\n\n\0"
};

#include <SoftwareSerial.h>
#include<LiquidCrystal.h>

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

SoftwareSerial esp8266(11, 12); // RX, TX

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);
  lcd.begin(16, 2);

  lcd.print("SERVER - Text");

  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  lcd.setCursor(0, 1);
  lcd.print("WLAN CONNECTED");

  if (configTCPServer())  debug("Server Aktiv"); else debug("Server Error");
}

void loop() {
  String text;
  if (esp8266.available()) // check if the esp is sending a message
  {
    if (esp8266.find("+IPD,"))
    {
      debug("Incomming Request");
      int connectionId = esp8266.parseInt();

      if (esp8266.findUntil("?text=", "\n"))
      {

        text = esp8266.readStringUntil(' ');
        debug(text);
        text.replace("+", " ");
        lcd.clear();
        lcd.print("New message:");
        lcd.setCursor(0, 1);
        lcd.print(text);

        tone(PIEZO, 400, 500);
        delay(500);
        tone(PIEZO, 800, 500);
        delay(500);

      }
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
