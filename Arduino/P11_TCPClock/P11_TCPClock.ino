/*
TCP Time Sync.

Time Source:
http://chronic.herokuapp.com/

Change SSID and PASSWORD.
*/

#define SSID "5673RM11"
#define PASSWORD "frankmarcel"

#define DEBUG true

#define LED_WLAN 13

#include <SoftwareSerial.h>
#include <Time.h>
#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
SoftwareSerial esp8266(11, 12); // RX, TX

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  lcd.begin(16, 2);
  lcd.print("TCP - CLOCK");


  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  lcd.setCursor(0, 1);
  lcd.print("WLAN CONNECTED");

  getTime("chronic.herokuapp.com", "/utc/in+two+hour"); //Gets Time from Page and Sets it
}

void loop() {
  String shour, sminute, ssecond, sday, smonth;

  delay(1000);

  if (hour() <= 9) shour = "0" + String(hour()); else shour = String(hour()); // adjust for 0-9
  if (minute() <= 9) sminute = "0" + String(minute());  else sminute = String(minute());
  if (second() <= 9) ssecond = "0" + String(second());  else ssecond = String(second());

  if (day() <= 9) sday = "0" + String(day()); else sday = String(day()); // adjust for 0-9
  if (month() <= 9) smonth = "0" + String(month());  else smonth = String(month());

  String Time = " -- " + shour + ":" + sminute + ":" + ssecond + " --";
  String Date = "   " + sday + "-" + smonth + "-" + String(year()) + "   ";

  debug(Date);
  debug(Time);

  lcd.clear();
  lcd.print(Time);

  lcd.setCursor(0, 1);
  lcd.print(Date);
}


String getTCP(String Host, String Subpage)
{
  boolean success = true;

  success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");
  String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n\r\n";
  success &= sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">");

  return sendCom(getRequest);
}

boolean getTime(String Host, String Subpage)
{
  boolean success = true;
  int xyear, xmonth, xday, xhour, xminute, xsecond;  //lokal variables

  success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");
  String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n";
  success &= sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">");

  esp8266.println(getRequest);

  if (esp8266.find("+IPD"))
  {
    if (esp8266.find("\r\n\r\n"))
    {
      xyear = esp8266.parseInt();
      xmonth = esp8266.parseInt();
      xday = esp8266.parseInt();
      xhour = esp8266.parseInt();
      xminute = esp8266.parseInt();
      xsecond = esp8266.parseInt();

      if (xday < 0) xday *= -1;          //Because of date seperator - parseInt detects negativ integer
      if (xmonth < 0) xmonth *= -1;    //Because of date seperator - parseInt detects negativ integer


      setTime(xhour, xminute, xsecond, xday, xmonth, xyear);
      sendCom("AT+CIPCLOSE", "OK");
      return true;
    }
    else return false;
  }
  else return false;
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
