/*
TCP Kodi

Change KODI_IP, SSID and PASSWORD.

Aktiver Player:
{"jsonrpc": "2.0", "method": "Player.GetActivePlayers", "id": 1}
http://KODI_IP/jsonrpc?request=%7B%22jsonrpc%22%3A%20%222.0%22%2C%20%22method%22%3A%20%22Player.GetActivePlayers%22%2C%20%22id%22%3A%201%7D%0A

Was wird in Player mit der ID 1 gespielt (Video):
{"jsonrpc":"2.0","method":"Player.GetItem","params":{"playerid":1},"id":1}
http://KODI_IP/jsonrpc?request=%7B%22jsonrpc%22%3A%222.0%22%2C%22method%22%3A%22Player.GetItem%22%2C%22params%22%3A%7B%22playerid%22%3A1%7D%2C%22id%22%3A1%7D%0D%0A

Play/Pause Player mit ID 1:
{"jsonrpc":"2.0","method":"Player.PlayPause","params":{"playerid":1},"id":1}
http://KODI_IP/jsonrpc?request=%7B%22jsonrpc%22%3A%222.0%22%2C%22method%22%3A%22Player.PlayPause%22%2C%22params%22%3A%7B%22playerid%22%3A1%7D%2C%22id%22%3A1%7D

Nächstes Element im Player mit ID 1:
{"jsonrpc": "2.0", "method": "Player.GoTo", "params": {"playerid":1, "to": "next"}, id":1}
http://KODI_IP/jsonrpc?request=%7B%22jsonrpc%22%3A%222.0%22%2C%22method%22%3A%22Player.GoTo%22%2C%22params%22%3A%7B%22playerid%22%3A1%2C%22to%22%3A%22next%22%7D%2C%22id%22%3A1%7D%3B

Weitere Mögliche Befehle:,
http://kodi.wiki/view/JSON-RPC_API/v6
{"jsonrpc": "2.0", "method": "Player.GoTo", "params": {"playerid":0, "to": "previous"}, id":1}
{"jsonrpc": "2.0", "method": "System.Suspend", "id":1}
{"jsonrpc":"2.0","id":"1","method":"Player.Open","params":{"item":{"file":"special://profile/playlists/music/Morning.xsp"
*/


#define SSID "[Your SSID]"
#define PASSWORD "[Your Password]"

#define KODI_IP "192.168.178.43"

#define DEBUG true

#define LED_WLAN 13

#define BUTTON1 5
#define BUTTON2 10


#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
SoftwareSerial esp8266(11, 12); // RX, TX

int ID;

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  lcd.begin(16, 2);
  lcd.print("KODI");

  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  lcd.setCursor(0, 1);
  lcd.print("CONNECTED");
}

void loop() {
  String nowPlaying = getPlaying();

  lcd.clear();

  lcd.print("Playing:");
  lcd.setCursor(0, 1);
  lcd.print(nowPlaying);
  
  if (nowPlaying.length() > 16)
  {    
    delay(500);
    for (int positionCounter = 0; positionCounter < nowPlaying.length() - 16; positionCounter++) {
      // scroll one position right:
      lcd.scrollDisplayLeft();
      // wait a bit:
      delay(150);

      if (!digitalRead(BUTTON1)) {
        playPause();
      }
      if (!digitalRead(BUTTON2)) {
        playNext();
      }
    }
  }
  else
  {
    for (int i = 0; i <= 20; i++)
    {
      if (!digitalRead(BUTTON1)) {
        playPause();
      }
      if (!digitalRead(BUTTON2)) {
        playNext();
      }
      delay(100);
    }
  }
}


String getJsonString(String Method, String Params)
{
  String Json = "/jsonrpc?request=%7B%22jsonrpc%22%3A%222.0%22%2C%22method%22%3A%22";
  Json += Method;
  if (Params == "") {
    Json += "%22";
  }
  else
  {
    Json += "%22%2C%22params%22%3A%7B";
    Json += Params;
    Json += "%7D";
  }
  Json += "%2C%22id%22%3A1%7D%0D%0A";
  //%2C%22id%22%3A1%7D%0A
  return Json;
}

int getPlayerID()
{
  String Subpage = getJsonString("Player.GetActivePlayers", "");
  String Host = KODI_IP;

  boolean success = true;

  success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");
  if (success = true)
  {
    String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n\r\n";
    success &= sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">");

    if (sendCom(getRequest, "result")) {
      if (esp8266.findUntil("playerid\":", "]"))
      {
        int ID = esp8266.parseInt();
        return    ID;
      }
    }
  }
  return -1;
}

String getPlaying()
{
  ID = getPlayerID();

  if (ID != -1)
  {
    String sID = String(ID);
    String Subpage = getJsonString("Player.GetItem", "%22playerid%22%3A" + sID);

    String Host = KODI_IP;

    boolean success = true;

    success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");
    if (success = true)
    {
      String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n\r\n";
      success &= sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">");

      if (sendCom(getRequest, "label\":"))
      {
        String label = esp8266.readStringUntil(',');
        debug(label);

        if (label.length() > 32) {
          label = label.substring(0, 32);
        }

        return (label);
      }
    }
  }
  else
  {
    return " - ";
  }
}

void playPause()
{
  debug("Play/Pause");

  if (ID != -1)
  {
    lcd.clear();
    lcd.print("  Play/Pause");
    lcd.setCursor(0, 1);

    getTCP(KODI_IP, getJsonString("Player.PlayPause", "%22playerid%22%3A" + String(ID)));
  }
  while (!BUTTON1) {  }
}

void playNext()
{
  debug("Play Next");
  if (ID != -1)
  {
    lcd.clear();
    lcd.print("  Next");
    lcd.setCursor(0, 1);
    
    getTCP(KODI_IP, getJsonString("Player.GoTo", "%22playerid%22%3A" + String(ID) + "%2C%22to%22%3A%22next%22"));
    //sendCom("AT+CIPCLOSE");
  }
  while (!BUTTON1) {  }
  delay(2000);
}

String getTCP(String Host, String Subpage)
{
  boolean success = true;
  success &= sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK");

  String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n\r\n";
  success &= sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">");
  return sendCom(getRequest);
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
//
//boolean configTCPServer()
//{
//  boolean success = true;
//
//  success &= (sendCom("AT+CIPMUX=1", "OK"));
//  success &= (sendCom("AT+CIPSERVER=1,80", "OK"));
//
//  return success;
//
//}

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

//-----------------------------------------------Controll ESP-----------------------------------------------------

//boolean sendUDP(String Msg)
//{
//  boolean success = true;
//
//  success &= sendCom("AT+CIPSEND=" + String(Msg.length() + 2), ">");    //+",\"192.168.4.2\",90", ">");
//  if (success)
//  {
//    success &= sendCom(Msg, "OK");
//  }
//  return success;
//}


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
