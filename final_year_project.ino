#include <LiquidCrystal.h>
#include <Servo.h>
#include <ThingSpeak.h> 
#include <ESP8266WiFi.h> 
#include <TinyGPS++.h> 

const char *ssid = "YOUR WIFI NAME"; 
const char *password = "WIFI PASSWORD"; 
const char *thingSpeakApiKey = "THINGSPEAK API KEY"; 
const int thingSpeakChannelNumber = THINGSPEAK CHANNEL ID; 

const int trigPin = D6;
const int echoPin = D7;
const int led1 = D5; // Green LED
const int led2 = D3; // Red LED
const int buzzerPin = D4; // Buzzer pin
const int irSensorPin = A0; // IR Sensor pin (analog input)
const int gpsSerialRx = D1; // GPS module Rx pin
const int gpsSerialTx = D2; // GPS module Tx pin

LiquidCrystal lcd(D0, D1, D2, D3, D4, D5);
long duration;
int distance;
int irSensorValue; 
Servo myServo;
WiFiClient client; 
TinyGPSPlus gps; 

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);
  myServo.attach(D8);
  
  
  Serial.begin(9600); 
  
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  
  ThingSpeak.begin(client);
}

void loop() {
  
  while (Serial.available() > 0) {
    if (gps.encode(Serial.read())) {
      if (gps.location.isValid()) {
        
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();

        
        Serial.print("GPS - Latitude: ");
        Serial.print(latitude, 6);
        Serial.print("  Longitude: ");
        Serial.println(longitude, 6);

        // Send latitude and longitude to ThingSpeak
        //ThingSpeak.writeField(thingSpeakChannelNumber, 3, latitude, thingSpeakApiKey);
        //ThingSpeak.writeField(thingSpeakChannelNumber, 4, longitude, thingSpeakApiKey);
      }
    }
  }

  
  irSensorValue = analogRead(irSensorPin);

  
  for (int i = 15; i <= 165; i++) {
    myServo.write(i);
    delay(30);
    distance = calculateDistance();

    
    Serial.print("Servo Position: ");
    Serial.print(i);
    Serial.print("  Distance: ");
    Serial.print(distance);
    Serial.print("  IR Sensor Value: ");
    Serial.println(irSensorValue);

    
    lcd.setCursor(0, 0);
    lcd.print("Degree: ");
    lcd.print(i);
    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(distance);
    lcd.print(" IR: ");
    lcd.print(irSensorValue);

    
    if (distance < 30 && irSensorValue > 500) { 
      digitalWrite(led1, LOW);
      digitalWrite(led2, HIGH);
      triggerBuzzer();
      
      
      ThingSpeak.writeField(thingSpeakChannelNumber, 1, distance, thingSpeakApiKey);
      ThingSpeak.writeField(thingSpeakChannelNumber, 2, irSensorValue, thingSpeakApiKey);
    } else {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, LOW);
      noTone(buzzerPin);
    }
  }
}

int calculateDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  return distance;
}

void triggerBuzzer() {
  tone(buzzerPin, 500); 
  delay(500); 
  noTone(buzzerPin); 
}
