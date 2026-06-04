# Intelligent Initialization System ("Pixel - tse rozlyuchena vyshyvanka")

This project was developed as part of the hackathon **"Pixel - tse rozlyuchena vyshyvanka"** ("Pixel is an angry vyshyvanka").

**Team:** 
* Volodymyr Fufalko
* Andriy Oliynyk
* Zakhar Savchyn
* Ivan Varvaruk

**Date:** 21.05.2026

---

## Project Description
The device is designed to measure the distance to an object using the **VL53L0X V2** laser rangefinder and provide progressive light signals as the object approaches. 

The program is optimized for high-speed objects, featuring built-in latency prediction for actuator delays, as well as a redundant contact sensor (tripwire type) to ensure reliable initialization in the event of an optical sensor failure.

---

## Key Features & Optimizations

1. **Pre-trigger Compensation**:
   At an object speed of $25\text{ m/s}$ and an actuator delay of $2\text{ ms}$, the object travels $50\text{ mm}$. To compensate for this latency, the system automatically shifts the initialization threshold by $+50\text{ mm}$ (`COMPENSATION_MM`), ensuring the physical actuator triggers exactly at the target distance.

2. **Microcontroller Performance (Direct Port Manipulation)**:
   To minimize the MCU's reaction time, direct port register control (`PORTD` and `PORTB`) is used instead of the standard `digitalWrite()` function. This reduces LED control latency to just a few processor cycles.

3. **Fallback Tripwire Sensor**:
   Connected via hardware interrupt `INT0` (Pin 2). Upon trigger, the blue LED is immediately activated using a fast interrupt service routine (ISR). This guarantees reliability even if the I2C sensor freezes or fails.

4. **Noise Filtering & False Trigger Prevention**:
   * Invalid distance readings ($0$ or $>8000\text{ mm}$) are ignored.
   * The number of consecutive confirmed measurements (`REQUIRED_READINGS`) required before initiating the signal can be customized.

---

## Pinout

* **Pin 2 (INT0)**: Tripwire sensor input (configured with `INPUT_PULLUP`)
* **Pin 4**: Red LED (Initialization signal at 1.0m)
* **Pin 5**: LED 0.75m
* **Pin 6**: LED 0.50m
* **Pin 7**: LED 0.25m
* **Pin 13 (PB5)**: Blue LED (Tripwire trigger indicator)
* **I2C (SDA/SCL)**: VL53L0X distance sensor
