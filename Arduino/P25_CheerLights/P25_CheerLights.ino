/*
Thingspeak - Cheerlights
Change SSID and PASSWORD
*/

#define SSID "5673RM11"
#define PASSWORD "frankmarcel"

#define RED 3
#define GREEN 5
#define BLUE 6
#define GND 4

#define LED_WLAN 13

#define DEBUG true

#include <RGBLED.h>
#include <RGBColor.h>

#include <SoftwareSerial.h>
SoftwareSerial esp8266(11, 12); // RX, TX


//* red pin 5; green pin 6, blue pin 9
//* hold 0ms, wait 2ms, common anode
RGBLED rgbLED = RGBLED(RED, GREEN, BLUE, 0, 3, 0);

const byte thingPost[] PROGMEM = {
  "POST *URL* HTTP/1.1\nHost: api.thingspeak.com\nConnection: close\nContent-Type: application/x-www-form-urlencoded\nContent-Length: *LEN*\n\n*APPEND*\n\0"
};

RGBColor red   = RGBColor(100, 0, 0);
RGBColor green = RGBColor(0, 100, 0);
RGBColor blue  = RGBColor(0, 0, 100);

RGBColor cyan   = RGBColor(0, 100, 100);
RGBColor white = RGBColor(255, 255, 255);
RGBColor warmwhite  = RGBColor(100, 87, 87);
RGBColor purple = RGBColor(50, 0, 50);
RGBColor magenta = RGBColor(100, 0, 100);

RGBColor yellow = RGBColor(100, 100 , 0);
RGBColor orange = RGBColor(100, 65 , 0);

RGBColor pink = RGBColor(100, 75 , 100);

String oldColor;

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);
 }

void loop() {

  String color = getCheerlightColor( "api.thingspeak.com",  "/channels/1417/field/1/last.txt");
  debug (color);
  if (color != oldColor)
  {
    if (color == "red") rgbLED.crossfade(red);
    if (color == "green") rgbLED.crossfade(green);
    if (color == "blue") rgbLED.crossfade(blue);
    if (color == "cyan") rgbLED.crossfade(cyan);
    if (color == "white") rgbLED.crossfade(white);
    if (color == "warmwhite") rgbLED.crossfade(warmwhite);
    if (color == "purple") rgbLED.crossfade(purple);
    if (color == "magenta") rgbLED.crossfade(magenta);
    if (color == "yellow") rgbLED.crossfade(yellow);
    if (color == "orange") rgbLED.crossfade(orange);

    oldColor = color;
  }
  delay(10000);
}

String getCheerlightColor(String Host, String Subpage)
{
  boolean success = true;
  String color;
  success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");
  String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n";
  success &= sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">");

  esp8266.println(getRequest);

  if (esp8266.find("+IPD"))
  {
    if (esp8266.find("\r\n\r\n"))
    {
      esp8266.readStringUntil('\n');
      color = esp8266.readStringUntil('\r');
      int msgEnd = esp8266.parseInt();
    }
  }
  sendCom("AT+CIPCLOSE", "OK");
  return color;
}

//-----------------------------------------ThingsSpeak Functions------------------------------------

boolean sendThingPost(String key, int value)
{
  boolean success = true;
  String  Host = "api.thingspeak.com";
  String msg = "field1=" + String(value);
  success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");

  String postRequest = createThingPost("/update", key, msg);

  if (sendCom("AT+CIPSEND=" + String(postRequest.length()), ">"))
  {
    esp8266.print(postRequest);
    esp8266.find("SEND OK");
    if (!esp8266.find("CLOSED")) success &= sendCom("AT+CIPCLOSE", "OK");
  }
  else
  {
    success = false;
  }
  return success;
}

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

String createThingGet(String url, String key, String msg)
{
  String xBuffer;

  for (int i = 0; i <= sizeof(thingPost); i++)
  {
    char myChar = pgm_read_byte_near(thingPost + i);
    xBuffer += myChar;
  }

  String append = "api_key=" + key + "&" + msg;

  xBuffer.replace("POST", "GET");
  xBuffer.replace("*URL*", url);
  xBuffer.replace("*LEN*", String( append.length()));
  xBuffer.replace("*APPEND*", append);

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
