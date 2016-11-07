/*
Thingspeak Alarm
Cange SSID and PASSWORD and TwitterKEY
*/

#define SSID "5673RM11"
#define PASSWORD "frankmarcel"

#define TwitterKEY "SDW5R69WTTBWIMVW"

#define SENSOR A6

#define PIEZO 8

#define LED_WLAN 13

#define DEBUG true

#include <SoftwareSerial.h>
#include<LiquidCrystal.h>

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
SoftwareSerial esp8266(11, 12); // RX, TX

const byte thingPost[] PROGMEM = {
  "POST *URL* HTTP/1.1\nHost: api.thingspeak.com\nConnection: close\nContent-Type: application/x-www-form-urlencoded\nContent-Length: *LEN*\n\n*APPEND*\n\0"
};


void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);
  lcd.begin(16, 2);

  lcd.print("Twitter Alarm");

  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  lcd.setCursor(0, 1);
  lcd.print("WLAN CONNECTED");
}

void loop() {
  int sensor = analogRead(SENSOR);
  lcd.clear();
  lcd.print("Brightness:");
  lcd.setCursor(0, 1);
  lcd.print(String(sensor));
  
  debug(String(sensor));
  delay(500);
  if (sensor >= 500)
  {
    alarm(sensor);
  }
}

void alarm(int value)
{
  debug("alarm!");

  sendTwitterPost(TwitterKEY, "Alarm! #NanoESP Value = " + String(value));
  
  while (analogRead(SENSOR) >= 500)
  {
    tone(PIEZO, 400, 500);
    delay(500);
    tone(PIEZO, 800, 500);
    delay(500);
  }

}

boolean sendTwitterPost(String key, String twitterMsg)
{
  boolean success = true;
  String  Host = "api.thingspeak.com";
  String Url = "/apps/thingtweet/1/statuses/update";
  String msg = "&status=" + twitterMsg;
  success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");

  String postRequest = createThingPost(Url, key, msg);
  if (sendCom("AT+CIPSEND=" + String(postRequest.length()), ">"))
  {
    esp8266.print(postRequest);
    debug(postRequest);
    esp8266.find("SEND OK");
    success &= sendCom("AT+CIPCLOSE", "OK");
  }
  else
  {
    success = false;
  }
  return success;
}


//-----------------------------------------ThingsSpeak Functions------------------------------------

//boolean sendThingPost(String key, int value)
//{
//  boolean success = true;
//  String  Host = "api.thingspeak.com";
//  String msg = "field1=" + String(value);
//  success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");
//
//  String postRequest = createThingPost("/update", key, msg);
//
//  if (sendCom("AT+CIPSEND=" + String(postRequest.length()), ">"))
//  {
//    esp8266.print(postRequest);
//    esp8266.find("SEND OK");
//    if (!esp8266.find("CLOSED")) success &= sendCom("AT+CIPCLOSE", "OK");
//  }
//  else
//  {
//    success = false;
//  }
//  return success;
//}

String createThingPost(String url, String key, String msg)
{
  String xBuffer;

  for (int i = 0; i <= sizeof(thingPost); i++)
  {
    char myChar = pgm_read_byte_near(thingPost + i);
    xBuffer += myChar;
  }

  String append = "api_key=" + key + "&" + msg;
  xBuffer.replace("*URL*", url);
  xBuffer.replace("*LEN*", String( append.length()));
  debug(append);
  xBuffer.replace("*APPEND*", append);

  return xBuffer;
}

String createThingGet(String url, String key)
{
  String xBuffer;

  for (int i = 0; i <= sizeof(thingPost); i++)
  {
    char myChar = pgm_read_byte_near(thingPost + i);
    xBuffer += myChar;
  }

  String append = "api_key=" + key;
  xBuffer.replace("POST", "GET");
  xBuffer.replace("*URL*", url);
  xBuffer.replace("*LEN*", String( append.length()));
  xBuffer.replace("*APPEND*", append);

  return xBuffer;
}

//String createThingGet(String url, String key, String msg)
//{
//  String xBuffer;
//
//  for (int i = 0; i <= sizeof(thingPost); i++)
//  {
//    char myChar = pgm_read_byte_near(thingPost + i);
//    xBuffer += myChar;
//  }
//
//  String append = "api_key=" + key + "&" + msg;
//
//  xBuffer.replace("POST", "GET");
//  xBuffer.replace("*URL*", url);
//  xBuffer.replace("*LEN*", String( append.length()));
//  xBuffer.replace("*APPEND*", append);
//
//  return xBuffer;
//}


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
  success &= sendCom("AT+CIPMUX=0", "OK");
  success &= sendCom("AT+CIPMODE=0", "OK");

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
//
//boolean configAP()
//{
//  boolean success = true;
//
//  success &= (sendCom("AT+CWMODE=2", "OK"));
//  success &= (sendCom("AT+CWSAP=\"NanoESP\",\"\",5,0", "OK"));
//
//  return success;
//}
//
//boolean configUDP()
//{
//  boolean success = true;
//
//  success &= (sendCom("AT+CIPMODE=0", "OK"));
//  success &= (sendCom("AT+CIPMUX=0", "OK"));
//  success &= sendCom("AT+CIPSTART=\"UDP\",\"192.168.255.255\",90,91,2", "OK"); //Importand Boradcast...Reconnect IP
//  return success;
//}
//



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
