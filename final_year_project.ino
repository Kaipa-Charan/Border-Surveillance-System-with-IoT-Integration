#include <LiquidCrystal.h>
#include <Servo.h>
#include <ThingSpeak.h> // Include ThingSpeak library
#include <ESP8266WiFi.h> // Include ESP8266 WiFi library
#include <TinyGPS++.h> // Include TinyGPS++ library

const char *ssid = "gopalakrishna"; // Replace with your WiFi SSID
const char *password = "9492937746"; // Replace with your WiFi password
const char *thingSpeakApiKey = "O93LITHIJXNTFVZY"; // Replace with your ThingSpeak API Key
const int thingSpeakChannelNumber = 2484750; // Replace with your ThingSpeak channel number

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
int irSensorValue; // IR sensor value
Servo myServo;
WiFiClient client; // Create a WiFiClient object
TinyGPSPlus gps; // Create a TinyGPS++ object

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);
  myServo.attach(D8);
  
  // Initialize GPS module
  Serial.begin(9600); // GPS module baud rate
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // Read GPS data
  while (Serial.available() > 0) {
    if (gps.encode(Serial.read())) {
      if (gps.location.isValid()) {
        // Get latitude and longitude from GPS
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();

        // Print latitude and longitude
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

  // Read IR sensor data
  irSensorValue = analogRead(irSensorPin);

  // Move servo and read distance
  for (int i = 15; i <= 165; i++) {
    myServo.write(i);
    delay(30);
    distance = calculateDistance();

    // Print servo position, distance, and IR sensor value
    Serial.print("Servo Position: ");
    Serial.print(i);
    Serial.print("  Distance: ");
    Serial.print(distance);
    Serial.print("  IR Sensor Value: ");
    Serial.println(irSensorValue);

    // Display data on LCD
    lcd.setCursor(0, 0);
    lcd.print("Degree: ");
    lcd.print(i);
    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(distance);
    lcd.print(" IR: ");
    lcd.print(irSensorValue);

    // Check conditions for alert and send data to ThingSpeak
    if (distance < 30 && irSensorValue > 500) { // Adjust threshold for IR sensor value
      digitalWrite(led1, LOW);
      digitalWrite(led2, HIGH);
      triggerBuzzer();
      
      // Send data to ThingSpeak
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
  tone(buzzerPin, 500); // Adjust frequency as needed
  delay(500); // Buzzer on for 500 milliseconds
  noTone(buzzerPin); // Turn off the buzzer
}
