#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <RTClib.h>
#include <EEPROM.h>

RTC_DS3231 rtc;

ESP8266WebServer server(80);
const char *ssid = "water";
const char *password = "12345678";

const int mOnHour = 6; //SET TIME TO ON RELAY (24 HOUR FORMAT)
const int mOnMin = 5;
const int mOffHour = 6; //SET TIME TO OFF RELAY
const int mOffMin = 10;

const int eOnHour = 18; //SET TIME TO ON RELAY (24 HOUR FORMAT)
const int eOnMin = 5;
const int eOffHour = 18; //SET TIME TO OFF RELAY
const int eOffMin = 10;


const int relay = 2;
bool relaystatus;
bool eeprom_write = false;

void setup() {
  //Serial.begin(115200);
  //rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  EEPROM.begin(512);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  relaystatus=false;
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_AP);
  boolean result = WiFi.softAP(ssid, password);
  if(result) {
    Serial.println("Ready");
  }
  else {
    Serial.println("Failed!");
  }
  delay(100);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handle_OnConnect);
  server.on("/relayon", handle_relayon);
  server.on("/relayoff", handle_relayoff);
  server.begin();
  //Serial.println("HTTP server started");
  server.send(200, "text/html", SendHTML(relaystatus));
}

String SendHTML(bool relaystat){
  DateTime now = rtc.now();
  char buf2[] = "DD-MM-YY:hh:mm:ss";
  String current_time=String(now.toString(buf2));
  String last_run = read_eeprom();
  
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Garden Irrigation System</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  ptr +="<h5>WebServer time : "+current_time+"</h3>\n";
  ptr +="<h5>Last irrigation time : "+last_run+"</h3>\n";
  
   if(relaystat)
  {ptr +="<p>Solenoid Relay Status: ON</p><a class=\"button button-off\" href=\"/relayoff\">OFF</a>\n";}
  else
  {ptr +="<p>Solenoid Relay Status: OFF</p><a class=\"button button-on\" href=\"/relayon\">ON</a>\n";}
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
void handle_relayon(){
  digitalWrite(relay, LOW);
  relaystatus=true;
  eeprom_write=true;
  write_eeprom();
  server.send(200, "text/html", SendHTML(relaystatus));
}
void handle_relayoff(){
  digitalWrite(relay, HIGH);
  relaystatus=false;
  eeprom_write=false;
  write_eeprom();
  server.send(200, "text/html", SendHTML(relaystatus));
}
void write_eeprom(){
  if (eeprom_write){
  DateTime now = rtc.now();
  char buf3[] = "DD-MM-YY:hh:mm";
  String last_run=now.toString(buf3);
  for(int i=0;i<last_run.length();i++)
  {
    EEPROM.write(0x0F+i, last_run[i]);
  }
  EEPROM.commit();
  eeprom_write=false;
  }
}

String read_eeprom(){
  String strText; 
  for(int i=0;i<14;i++)
  {
    strText = strText + char(EEPROM.read(0x0F+i));
  }
  return strText;
}
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(relaystatus));
}
void loop() {
  Serial.println("loop start");

 //lcd_print("loop start");
  delay(600);
 DateTime now = rtc.now();
  if(now.hour() == mOnHour && now.minute() == mOnMin){
    write_eeprom();
    digitalWrite(relay, LOW);
    }
    
    else if(now.hour() == mOffHour && now.minute() == mOffMin){
      digitalWrite(relay, HIGH);
      eeprom_write = true;
    }
  if(now.hour() == eOnHour && now.minute() == eOnMin){
    Serial.println("relay On");
    digitalWrite(relay, LOW);
    write_eeprom();
    }
    
    else if(now.hour() == eOffHour && now.minute() == eOffMin){
      digitalWrite(relay, HIGH);
      eeprom_write=true;
    }    
    delay(1000);

server.handleClient();
 Serial.println("loop end");
}
