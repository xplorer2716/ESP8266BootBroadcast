/*
 * ESP8266 UDP Broadcast on Boot
 * Copyright (C) 2026  
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// LED pin - using built-in LED (GPIO2 on most ESP8266 boards)
#define LED_PIN LED_BUILTIN
const int DELAY_100MS=100;

// WiFi credentials
const char* ssid = "<YOUR_SSID>";  // Change to your WiFi SSID
const char* password = "<YOUR_PASSWORD>";  // Change to your WiFi password

// UDP settings
WiFiUDP udp;
const int udpPort = 10666;  // Change to your desired port

const char* udpMessage = "ESP8266_BOOT1";

// State machine for loop management
enum State {
  CONNECTING_WIFI,
  SENDING_BROADCAST,
  BLINKING_NORMAL
};

State currentState = CONNECTING_WIFI;
unsigned long lastBlinkTime = 0;
bool ledState = false;

void setup() {
  // Initialize serial communication at 115200 baud
  Serial.begin(115200);
  delay(DELAY_100MS);
  
  Serial.println("\n\n=================================");
  Serial.println("ESP8266 UDP Broadcast on Boot");
  Serial.println("=================================");
  
  // Configure LED pin as output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Turn off LED initially (HIGH = off on ESP8266)
}

void loop() {
  unsigned long currentTime = millis();
  
  switch (currentState) {
    
    case CONNECTING_WIFI:
    {
      // Fast blinking during WiFi connection (2*DELAY_100MS = 200ms)
      if (WiFi.status() != WL_CONNECTED) {
        // Start WiFi connection on first iteration
        static bool wifiStarted = false;
        if (!wifiStarted) {
          Serial.print("Connecting to WiFi");
          WiFi.begin(ssid, password);
          wifiStarted = true;
        }
        
        // Fast blink LED
        if (currentTime - lastBlinkTime >= 2*DELAY_100MS) {
          ledState = !ledState;
          digitalWrite(LED_PIN, ledState ? LOW : HIGH);
          lastBlinkTime = currentTime;
          Serial.print(".");
        }
      } else {
        // WiFi connected!
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        // Begin UDP
        udp.begin(udpPort);
        
        // Turn LED ON (fixed) and move to broadcast state
        digitalWrite(LED_PIN, LOW);
        currentState = SENDING_BROADCAST;
      }
      break;
    }
      
    case SENDING_BROADCAST: {
      // LED stays ON during broadcast sending
      Serial.println("Sending UDP broadcasts (LED fixed ON)...");
      
      IPAddress broadcastIP(255, 255, 255, 255);
      for (int i = 0; i < 3; i++) {
        udp.beginPacket(broadcastIP, udpPort);
        udp.write(udpMessage);
        udp.endPacket();
        
        Serial.print("Broadcast sent (");
        Serial.print(i + 1);
        Serial.println("/3)");
        
        if (i < 2) {
          delay(DELAY_100MS);
        }
      }
      
      Serial.println("Broadcasts complete. Starting normal LED blink...");
      lastBlinkTime = currentTime;
      currentState = BLINKING_NORMAL;
      break;
    }
      
    case BLINKING_NORMAL: {
      // Slow blinking after broadcasts (DELAY_100MS*10 = 1 second)
      if (currentTime - lastBlinkTime >= DELAY_100MS*10) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? LOW : HIGH);
        Serial.println(ledState ? "LED: ON" : "LED: OFF");
        lastBlinkTime = currentTime;
      }
      break;
    }
  }
}