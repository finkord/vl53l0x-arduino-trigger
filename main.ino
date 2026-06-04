/**
 * @file main.ino
 * @author Volodymyr Fufalko for hackathon "Pixel - tse rozlyuchena vyshyvanka" (in team with Andriy Oliynyk, Zakhar Savchyn, Ivan Varvaruk)
 * @date 21.05.2026
 * @brief High-speed object detection and initialization system using VL53L0X and a tripwire fallback.
 * 
 * This program implements a progressive distance indicator (1m, 0.75m, 0.5m, 0.25m) 
 * with a predictive pre-trigger compensation to account for system latency 
 * (actuator delay of 2ms and object speed of 25m/s). It features safeguard 
 * mechanisms against false triggers and uses a fast hardware interrupt (INT0) 
 * for a physical tripwire sensor as a highly reliable fallback initialization method.
 */

#include <Wire.h>
#include <VL53L0X.h> 

#define WIRE_SENSOR_PIN   2
#define LED_INIT_100CM    4
#define LED_75CM          5
#define LED_50CM          6
#define LED_25CM          7
#define LED_WIRE_BLUE     13

VL53L0X sensor;

// Pre-trigger compensation for high-speed object and actuator delay
const uint16_t SPEED_M_S = 25; 
const uint16_t ACTUATOR_DELAY_MS = 2; 
const uint16_t COMPENSATION_MM = SPEED_M_S * ACTUATOR_DELAY_MS;

unsigned long previousMillis = 0;
unsigned long previousSerialMillis = 0; 
const long interval = 33; // Matches VL53L0X default timing budget

const uint8_t REQUIRED_READINGS = 1; 
uint8_t initConfirmCount = 0; 

volatile bool wireTriggered = false;

// Fast Hardware Interrupt Service Routine (ISR) for the wire sensor
void wireClosedISR() {
  wireTriggered = true;
  PORTB |= B00100000;
  EIMSK &= ~(1 << INT0); // Disable INT0 to prevent contact bounce
}

void setup() {
  Serial.begin(9600);
  
  DDRB |= B00100000;
  DDRD |= B11110000; 

  pinMode(WIRE_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WIRE_SENSOR_PIN), wireClosedISR, FALLING);

  Wire.begin();

  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println("Sensor init failed!");
    while (1) {}
  }

  sensor.startContinuous();
} 

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking loop reading sensor data every 33 ms
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    uint16_t data = sensor.readRangeContinuousMillimeters();
    if (sensor.timeoutOccurred()) { 
      Serial.print("TIMEOUT "); 
    }
    
    controlLed(data);
  }
}

void controlLed(uint16_t dist) {
  // Ignore invalid sensor values
  if (dist == 0 || dist > 8000) {
    initConfirmCount = 0; 
    PORTD &= B00001111;   
    return; 
  }

  uint8_t portMask = 0; 

  if (dist <= (1000 + COMPENSATION_MM)) {
    if (initConfirmCount < REQUIRED_READINGS) {
      initConfirmCount++;
    }
  } else {
    initConfirmCount = 0; 
  }

  if (initConfirmCount >= REQUIRED_READINGS || wireTriggered) {
    portMask |= (1 << 4); 
  }
  
  if (dist <= (750 + COMPENSATION_MM))  portMask |= (1 << 5); 
  if (dist <= (500 + COMPENSATION_MM))  portMask |= (1 << 6); 
  if (dist <= (250 + COMPENSATION_MM))  portMask |= (1 << 7); 

  // Direct Port Manipulation: Updates 4 LEDs in 1 CPU cycle
  PORTD = (PORTD & B00001111) | portMask;

  // Limit UART output
  if (millis() - previousSerialMillis >= 500) {
    previousSerialMillis = millis();
    Serial.print("Dist: "); 
    Serial.print(dist); 
    Serial.println("mm");
  }
}