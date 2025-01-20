#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define WIFI_SSID "Vivo V29"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyAz7r8LyvZICeBBazaK0-D8L13gf_cXNz4"
#define DATABASE_URL "https://energy-tariff-system-default-rtdb.firebaseio.com"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Pin Definitions
int i1 = 15;
int i2 = 2;
int i3 = 4;
int i4 = 5;
bool led_state;

// LED count variables
float count1 = 0.0;
float count2 = 0.0;
float count3 = 0.0;
float count4 = 0.0;

void setup() {
  pinMode(i1, OUTPUT);
  pinMode(i2, OUTPUT);
  pinMode(i3, OUTPUT);
  pinMode(i4, OUTPUT);
  
  // Initialize Serial communication
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  
  // Initialize Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase sign up successful");
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void updateLED(String ledName, int pin) {
  if (Firebase.ready()) {
    if (Firebase.RTDB.getBool(&fbdo,ledName)) {
      led_state = fbdo.to<bool>();
      digitalWrite(pin, led_state ? HIGH : LOW);
    } else {
      Serial.println(fbdo.errorReason().c_str());
      Serial.println("Error reading data for " + ledName);
    }
  }
}

void loop() {
  // Handle Bluetooth commands
  updateLED("/LED 1 CT", i1);
  updateLED("/LED 2 CT", i2);
  updateLED("/LED 3 CT", i3);
  updateLED("/LED 4 CT", i4);

  // Update count if LED is ON
  if (digitalRead(i1) == HIGH) {
    count1 += 0.005833;
  }
  if (digitalRead(i2) == HIGH) {
    count2 += 0.011667;
  }
  if (digitalRead(i3) == HIGH) {
    count3 += 0.023334;
  }
  if (digitalRead(i4) == HIGH) {
    count4 += 0.029167;
  }

  // Update Firebase with counts
  if (Firebase.ready()) {
    if (Firebase.RTDB.setFloat(&fbdo, "LED 1", count1) &&
        Firebase.RTDB.setFloat(&fbdo, "LED 2", count2) &&
        Firebase.RTDB.setFloat(&fbdo, "LED 3", count3) &&
        Firebase.RTDB.setFloat(&fbdo, "LED 4", count4)) {
      Serial.println("Uploaded");
    } else {
      Serial.println(fbdo.errorReason());
    }
  }
  delay(300); // Add delay to prevent rapid count incrementation
}
