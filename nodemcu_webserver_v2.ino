#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <RTClib.h>
#include <EEPROM.h>

RTC_DS3231 rtc;

ESP8266WebServer server(80);
const char *ssid = "sathish";
const char *password = "xxxxxxxxx";

//String default_duration = String(05);

const int mOnHour = 6; //SET TIME TO ON RELAY (24 HOUR FORMAT)
const int mOnMin = 5;
//const int mOffHour = 6; //SET TIME TO OFF RELAY
//const int mOffMin = 10;

const int eOnHour = 18; //SET TIME TO ON RELAY (24 HOUR FORMAT)
const int eOnMin = 5;
//const int eOffHour = 18; //SET TIME TO OFF RELAY
//const int eOffMin = 10;

int mOffHour;
int mOffMin;
int eOffHour;
int eOffMin;
bool m_on = false;
bool e_on = false;
const int m_on_addr = 16;
const int e_on_addr = 17;
const int timer_addr = 15;
int timer_duration;


const int relay = 2;
bool relaystatus;

bool eeprom_write = false;

void setup() {
  //Serial.begin(115200);
  rtc.begin();
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
  server.on("/setduration5", handle_setduration5);
  server.on("/setduration15", handle_setduration15);
  server.on("/setduration30", handle_setduration30);
  server.on("/setduration60", handle_setduration60);
  server.on("/mIrrOff", handle_mIrrOff);
  server.on("/mIrrOn", handle_mIrrOn);
  server.on("/eIrrOff", handle_eIrrOff);
  server.on("/eIrrOn", handle_eIrrOn);
  server.begin();
  //Serial.println("HTTP server started");
  server.send(200, "text/html", SendHTML(relaystatus));
}

String SendHTML(bool relaystat){
  timer_duration=read_duration_eeprom(timer_addr);
  DateTime now = rtc.now();
  char buf2[] = "DD-MM-YY:hh:mm:ss";
  String current_time=String(now.toString(buf2));
  //String last_run = read_eeprom();
  
  int m_irr_status=read_duration_eeprom(m_on_addr);
  int e_irr_status=read_duration_eeprom(e_on_addr);
  if ( m_irr_status == 1 ) {
    m_on = true;
  }
  else {
    m_on = false;
  }
  if ( e_irr_status == 1 ) {
    e_on = true;
  }
  else{
    e_on = false;
  }
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
  //ptr +="<h5>Last irrigation time : "+last_run+"</h3>\n";
  ptr +="<h5>Timer Duration is : "+String(timer_duration)+ " Minutes </h3>\n";
  
   if(relaystat)
  {ptr +="<p>Solenoid Relay Status: ON</p><a class=\"button button-off\" href=\"/relayoff\">OFF</a>\n";}
  else
  {ptr +="<p>Solenoid Relay Status: OFF</p><a class=\"button button-on\" href=\"/relayon\">ON</a>\n";}

   if(m_on)
  {ptr +="<p>Morning Irrgation: ON</p><a class=\"button button-off\" href=\"/mIrrOff\">OFF</a>\n";}
  else
  {ptr +="<p>Morning Irrigation: OFF</p><a class=\"button button-on\" href=\"/mIrrOn\">ON</a>\n";}

   if(e_on)
  {ptr +="<p>Evening Irrgation: ON</p><a class=\"button button-off\" href=\"/eIrrOff\">OFF</a>\n";}
  else
  {ptr +="<p>Evening Irrigation: OFF</p><a class=\"button button-on\" href=\"/eIrrOn\">ON</a>\n";}  

  ptr +="<p>Update Timer to </p><a class=\"button button-on\" href=\"/setduration5\">5 Minutes</a>\n";
  ptr +="<p>Update Timer to</p><a class=\"button button-on\" href=\"/setduration15\">15 Minutes</a>\n";
  ptr +="<p>Update Timer to</p><a class=\"button button-on\" href=\"/setduration30\">30 Minutes</a>\n";  
  ptr +="<p>Update Timer to</p><a class=\"button button-on\" href=\"/setduration60\">60 Minutes</a>\n"; 
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
/*
void update_duration_eeprom(int str_address,String str_name){
  //Serial.println("updating duration ");
  for(int i=0;i<str_name.length();i++){
    //EEPROM.write(0x0F+i, value1[i]);
    EEPROM.write(str_address+i, str_name[i]);
  }
  EEPROM.commit();
  Serial.println("updated duration :"+str_name);
}


String read_duration_eeprom(int str_address,int count){
  String strText; 
  //Serial.println("eeprom address is :"+str_address);
  for(int i=0;i<count;i++) {
    strText = strText + char(EEPROM.read(str_address+i));
  }
  return strText;
}

*/

void handle_mIrrOff(){
  update_duration_eeprom(m_on_addr,0);
  server.send(200, "text/html", SendHTML(relaystatus));
}
void handle_mIrrOn(){
  update_duration_eeprom(m_on_addr,1);
  server.send(200, "text/html", SendHTML(relaystatus));
}

void handle_eIrrOff(){
  update_duration_eeprom(e_on_addr,0);
  server.send(200, "text/html", SendHTML(relaystatus));
}
void handle_eIrrOn(){
  update_duration_eeprom(e_on_addr,1);
  server.send(200, "text/html", SendHTML(relaystatus));
}

void update_duration_eeprom(int address,int duration){
  EEPROM.write(address, duration);
  EEPROM.commit();
  Serial.println("updated duration :");
  Serial.print(duration);
  //Serial.println("Restarting Nodemcu");
  //ESP.restart();
}
int read_duration_eeprom(int address){
  int readInt = EEPROM.read(address);
  return readInt;
}
void handle_setduration5(){
  update_duration_eeprom(15,5);
  server.send(200, "text/html", SendHTML(relaystatus));
}

void handle_setduration15(){
  update_duration_eeprom(15,15);
  server.send(200, "text/html", SendHTML(relaystatus));
}

void handle_setduration30(){
  update_duration_eeprom(15,30);
  server.send(200, "text/html", SendHTML(relaystatus));
}

void handle_setduration60(){
  update_duration_eeprom(15,60);
  server.send(200, "text/html", SendHTML(relaystatus));
}

void handle_relayon(){
  digitalWrite(relay, LOW);
  relaystatus=true;
  eeprom_write=true;
  //write_eeprom();
  server.send(200, "text/html", SendHTML(relaystatus));
}
void handle_relayoff(){
  digitalWrite(relay, HIGH);
  relaystatus=false;
  eeprom_write=false;
 // write_eeprom();
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
  //eeprom_write=false;
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
  int m_irr_status=read_duration_eeprom(m_on_addr);
  int e_irr_status=read_duration_eeprom(e_on_addr);
  
  timer_duration=read_duration_eeprom(timer_addr);
  int m_total_minutes = ((mOnHour * 60) + mOnMin + timer_duration);
  mOffHour = m_total_minutes / 60;
  mOffMin = m_total_minutes % 60;

  int e_total_minutes = ((eOnHour * 60) + eOnMin + timer_duration);
  eOffHour = e_total_minutes / 60;
  eOffMin = e_total_minutes % 60;
  /*
  String timer_duration=read_duration_eeprom(15,2);
  int m_total_minutes = ((mOnHour * 60) + mOnMin + timer_duration.toInt());
  int mOffHour = m_total_minutes / 60;
  int mOnMin = m_total_minutes % 60;

  int e_total_minutes = ((eOnHour * 60) + eOnMin + timer_duration.toInt());
  int eOffHour = e_total_minutes / 60;
  int eOffMin = e_total_minutes % 60;
  */
 //lcd_print("loop start");
  delay(600);
  DateTime now = rtc.now();
  if ( m_irr_status == 1 ) {
  if(now.hour() == mOnHour && now.minute() == mOnMin){
    //write_eeprom();
    digitalWrite(relay, LOW);
    Serial.println("Irrigation Started ");
    Serial.println("Irrigation End time:  ");
    Serial.print(mOffHour+":"+mOffMin);
    Serial.println("Irrigation Duration: ");
    Serial.print(timer_duration);
    }
    
    else if(now.hour() == mOffHour && now.minute() == mOffMin){
      digitalWrite(relay, HIGH);
      //eeprom_write = true;
    }
  }
  else {
    Serial.println("Morning irrigation off");
  }

  if ( e_irr_status == 1 ) {
  if(now.hour() == eOnHour && now.minute() == eOnMin){
  //if(now.hour() == now.hour() && now.minute() == now.minute()){
    //Serial.println("relay On");
    Serial.println("Irrigation Started ");
    Serial.print("Irrigation End time:");
    Serial.print(eOffHour);
    Serial.print(":");
    Serial.print(eOffMin);
    Serial.println();
    digitalWrite(relay, LOW);
    //write_eeprom();
    }
    
    else if(now.hour() == eOffHour && now.minute() == eOffMin){
      digitalWrite(relay, HIGH);
      //eeprom_write=true;
    }
  }
  else {
    Serial.println("Evening irrigation off");
  }
    delay(1000);

server.handleClient();
//Serial.println("loop end");
}
