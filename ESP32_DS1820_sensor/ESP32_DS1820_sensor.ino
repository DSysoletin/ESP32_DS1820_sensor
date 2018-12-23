#define ARDUINO_ESP32_DEV
#include <PubSubClient.h>
#include <Arduino.h>

#include <ESPBASE.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <esp_deep_sleep.h>


WiFiServer TelnetServer(23);  // Optional in case you want to use telnet as Monitor
WiFiClient Telnet;            // Optional in case you want to use telnet as Monitor

ESPBASE Esp;

WiFiClient espClient;
PubSubClient client(espClient);

int reconnect_lastrun=0;

#define ONEWIRE_PIN 4
OneWire oneWire(ONEWIRE_PIN);
DallasTemperature ds1820_sensors(&oneWire);
float temps[2];

//Deep sleep part - from https://github.com/SensorsIot/ESP32-Deep-Sleep/blob/master/TimerWakeUp/TimerWakeUp.ino
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60 /* Time ESP32 will go to sleep (in seconds) */



#define NUM_ATTEMPTS 10
#define ONBOARD_LED 22
void setup() {
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
  Serial.begin(115200);
  ds1820_sensors.begin();
  delay(1000); // give me time to bring up serial monitor 
  
   Esp.initialize();
   if(Esp.WIFI_connected)
   {
      //digitalWrite(LED_BUILTIN, HIGH);
   }
   // put your setup code here, to run once:

   //mqtt
   Serial.println("MQTT server: ");
   Serial.println(config.mqtt_server);
   client.setServer(config.mqtt_server.c_str(), atoi(config.mqtt_port.c_str()));
   esp_deep_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void reconnect() {
  String s;
  
  // Loop until we're reconnected
  //while (!client.connected()) {

  if(millis()-reconnect_lastrun < 5000)
  {
    return ;
  }
  digitalWrite(LED_BUILTIN, LOW);
  
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-"+config.mqtt_prefix+"-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if (client.connect(clientId.c_str())) {
    if (client.connect(clientId.c_str(),config.mqtt_username.c_str(),config.mqtt_password.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }

  //}
  //digitalWrite(LED_BUILTIN, HIGH);
  reconnect_lastrun = millis();
}


void get_temps()
{
  int i;
  int num_sensors;
  float temp;
  num_sensors=ds1820_sensors.getDS18Count();
  Serial.print("Number of ds1820's: ");
  Serial.println(num_sensors);
  for(i=0;i<NUM_ATTEMPTS;i++)
  {
    ds1820_sensors.requestTemperatures();
    delay(1000);
    temp=ds1820_sensors.getTempCByIndex(0);
    Serial.print("Temp(0): ");
    Serial.println(temp);
    temps[0]=temp;
    temp=ds1820_sensors.getTempCByIndex(1);
    Serial.print("Temp(1): ");
    Serial.println(temp);
    temps[1]=temp;
    if((temps[0]!=85.0)&&(temps[0]!=-127.0)&&(temps[1]!=85.0)&&(temps[1]!=-127.0))
    {
      break;
    }
    else
    {
      Serial.println("Error getting sensor data! Retry...");
    }
  }
}

void loop() {
  String s;
  int i=0,state=0;
  long last_millis=0;
  char payload[10];
  static int ap_timer=0;

//  payload="\0\0\0\0\0\0\0\0\0";
  //  WebServer requests handling
  server.handleClient();
  customWatchdog = millis();
  

  //Only do all things if wifi connected. 
  //In other case, config AP should be activated, and we should not try to send data and sleep
  if(Esp.WIFI_connected)
  {
    //We're here - this means that wifi is connected
  
    //Try to connect to mqtt server
    if ((!client.connected())&&(i<5)) {
      reconnect();
      i++;
    }
  
    //If several attempts failed - go to sleep, maybe something wrong with internet connectivity?
    if (!client.connected())
    {
      esp_deep_sleep_start();
    }
    
    client.loop();
    //read temperatures
    customWatchdog = millis();
    get_temps();
    customWatchdog = millis();
    //payload=temps[0];
    dtostrf(temps[0], 5, 3, payload); 
    //Public data to MQTT server
    s = "ESP32_"+config.mqtt_prefix+"0";
    Serial.println(s);
    client.publish(s.c_str(), payload);
    dtostrf(temps[1], 5, 3, payload); 
    //Public data to MQTT server
    s = "ESP32_"+config.mqtt_prefix+"1";
    Serial.println(s);
    client.publish(s.c_str(), payload);

    delay(1000);
    digitalWrite(ONBOARD_LED, HIGH);
    Serial.println("Going to sleep...");
    esp_deep_sleep_start();
  
  } 

  //Blinking LED will indicate that wifi isn't connected - and board in AP mode.
  if(millis()-last_millis>(1000))
  {
    Serial.println("Unable to connect WIFI. AP mode. Will go to sleep after 180 seconds...");
    Serial.println("Time in AP mode: ");
    Serial.println(ap_timer);
    state=1-state;
    digitalWrite(ONBOARD_LED, state);
    last_millis=millis();
    ap_timer++;
    if(ap_timer>180)
    {
      Serial.println("AP timer expired. Going to sleep now.");
      ap_timer=0;
      esp_deep_sleep_start();
    }
  }
}
