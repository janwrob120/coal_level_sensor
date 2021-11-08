#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "confidential.h"


#define TRIGGER_PIN 18
#define ECHO_PIN 19


const char * server = "api.thingspeak.com";
String apiKey = "EISU9XWCMAVVTJ21";
HTTPClient http;

int distance = 0;
int prev_distance = 100;

float measure_distance(byte trigger, byte echo) 
{
  int echo_pulse = 0;
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  echo_pulse = pulseIn(echo, 1, 10000); //measurement of the pulse
  return echo_pulse / 58;
}

//function that checks if the connection is lost and reconnects in such case, 1 if connected properly, 0 if not
bool keep_connected() 
{ 
  if (WiFi.status() != WL_CONNECTED) //in case of lost connection
  {
    digitalWrite(2, 1); //LED indicates lost connection
    WiFi.begin(ssid, password);

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED) {

      if(timeout >=20)
      {
        Serial.println("Failed to reconnect.");
        return 0;
      }
      timeout++;
      delay(500);
      Serial.print(".");
    }
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    digitalWrite(2, 0);
    return 1;
  }
}

bool filter(int d) 
{
  return true;
  if ((d - prev_distance < 13) && (d < 80) && (d > 5)) //difference less than 10
  {
    prev_distance = d;
    return true;
  }
  return false;
}

void setup() 
{
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(2, OUTPUT);
  Serial.begin(9600); // We initialize serial connection so that we could print values from sensor.
  keep_connected();
}

void loop() 
{
  if(keep_connected())
  {
    distance = (int) floor(measure_distance(TRIGGER_PIN, ECHO_PIN));
    Serial.print("Distance:  ");
    Serial.println(distance);
  
    if (filter(distance)) 
    {
      http.begin("https://api.thingspeak.com/update?api_key=EISU9XWCMAVVTJ21&field1=" + String(distance)); //Specify the URL
      int httpCode = http.GET();
      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      } else {
        Serial.println("Error on HTTP request");
      }
      http.end(); //Free the resources
      Serial.println("Waiting...");
      delay(16000);
    } else { //send wrong measured distance
      http.begin("https://api.thingspeak.com/update?api_key=EISU9XWCMAVVTJ21&field2=" + String(distance)); //Specify the URL
      int httpCode = http.GET();
      if (httpCode > 0) 
      {
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      } else {
        Serial.println("Error on HTTP request");
      }
      http.end(); //Free the resources
      Serial.println("Waiting...");
      delay(16000);
    }

  }
 

}
