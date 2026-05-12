# Satellite Ground Control Station (GCS) & Mission Simulator

## Overview

This project is a high-performance Ground Control Station (GCS) designed for real-time interaction with a satellite. While the software is a functional control center for an STM32-based satellite (communicating via UART/Radio link), it includes an integrated Software-in-the-Loop (SiL) Simulator.

The simulator emulates a complete orbital cycle, including transitions between sunlight and eclipse, allowing for full system testing without physical hardware.

## Key Features

* Orbital Cycle Simulation: Emulates satellite flight with automatic transitions between Sunlight (charging) and Eclipse (discharging) to test power management logic.

* Hardware-Ready Architecture: Designed to communicate with STM32 microcontrollers using a custom binary protocol.

* Multithreaded Core (POSIX Threads):

    * Telemetry Thread: High-speed parsing and validation of incoming data streams.

Command Gateway: Serialization and transmission of uplink commands.

    * UI Engine: Real-time visualization using the ncurses library.

Robust Data Handling: Implementation of thread-safe ring buffers using mutexes and condition variables to ensure data integrity.

Professional Mission Control Dashboard:

ADCS Status: Attitude Determination and Control (Roll, Pitch, Yaw) and angular velocity.

EPS Data: Battery voltage (dynamic discharge/charge curves) and solar panel efficiency.

Thermal Monitoring: Real-time component temperature simulation.

## Technical Stack

* Target Hardware: STM32 (ARM Cortex-M).

* Language: C (C11 standard).

Interface: ncurses.

Concurrency: pthreads (POSIX Threads).

Build System: GNU Makefile.

## Simulation & Logic

* Sunlight/Shadow Transitions: The satellite enters Earth's shadow periodically, triggering "Safe Mode" or power-saving logic.

Attitude Drift: Simulates uncontrolled rotation, requiring the operator to engage the STAB (Stabilization) command.

Telemetry Framing: Data is encapsulated in frames with integrity checks, mimicking a real radio link.

## Installation & Launch

### 1. Install Dependencies (Linux/WSL)
```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```
### 2. Build and Run
```bash
make
./ground_station
```

### Command Console

Interactive commands available in the prompt:

* SUN / ECLIPSE: Manually override orbital lighting conditions.

* STAB: Engage the ADCS to zero out angular velocities.

* RESET: Soft-reset the On-Board Computer (OBC) simulation.

![Interface Preview](screenshot.png)