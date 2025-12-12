#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT11
#define DHTPin 22

DHT dht(DHTPin, DHTTYPE);

const char* ssid = "gnet442663";
const char* password = "zahzeh2024";
const char* mqttServer = "192.168.1.3";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

int Value;
uint16_t val=0;
#define Adcpin 36   
#define LED1 2
#define LED2 23

unsigned long previous =0;
WiFiClient espClient;
PubSubClient client(espClient);

void callback(String topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.println("Message: ");
  String msg;

  if (topic == "LED1"){
    for (int i = 0; i < length; i++) {
      msg += (char)payload[i];
    }
    Serial.println(msg);
    Serial.print("LED status: ");
    if (msg == "ON"){ 
      digitalWrite(LED1, HIGH);
      Serial.println("ON"); 
    }
    else if (msg == "OFF"){ 
      digitalWrite(LED1, LOW);
      Serial.println("OFF");
    }
  }
  if (topic == "pwm"){
    for (int i = 0; i < length; i++) {
      msg += (char)payload[i];
    }
    Serial.println(msg);
    Serial.print("LED2 status: ");
   /* if (msg == "ON"){ 
      digitalWrite(LED2, HIGH);
      Serial.println("ON"); 
    }
    else if (msg == "OFF"){ 
      digitalWrite(LED2, LOW);
      Serial.println("OFF");

    }*/
    val = msg.toInt();
    uint16_t pwmValue = map(val,0,100,0,1023);
    analogWrite(LED2,pwmValue);
    Serial.print("PWM Value: ");
  }
  Serial.println();
  delay(10);   
}


void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    
    if (client.connect("userid")) { 
      Serial.println("Connected to MQTT");
      client.subscribe("LED1");            //SUBSCRIBE TOPIC
      client.subscribe("pwm");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(Adcpin, INPUT);
}

void loop() 
{
  int ADC = analogRead(Adcpin);
  Value = map(ADC,0,4095,0,100);
  Serial.println(Value);
  char payload[10]; 
  snprintf(payload, sizeof(payload), "%d", Value);
  client.publish("adc", payload);    
  //client.loop();
  Serial.println();
  unsigned long current = millis();
  if(current - previous >5000){
    previous = current;
    float temperature= dht.readTemperature();
    float humidity= dht.readHumidity();

    if(isnan(temperature) || isnan(humidity)){
      Serial.println("Error");
      return;
    }
    
    client.publish("temperature", String(temperature).c_str());
    client.publish("humidity", String(humidity).c_str());

    Serial.print(" Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
  }
  client.loop();
  
}

