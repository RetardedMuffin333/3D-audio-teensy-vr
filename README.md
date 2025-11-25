# 3D Audio Simulation with Teensy and Unity

This project implements a 3D audio simulation using:

- **MATLAB** for processing HRTF/HRIR data and generating FIR filter coefficients
- **Teensy** (Arduino-style programming) for real-time audio playback and filtering
- **Unity + Blender** for a 3D visual scene (e.g. a passing ambulance with a siren) and communication with the Teensy board

The system demonstrates how spatial audio (3D sound) can be created by combining signal processing, embedded hardware, and a game engine / VR environment.

---

## 1. Project Overview

The main idea:

1. Use an **HRTF database** to obtain filters that simulate how sound arrives at the left and right ear for different directions.
2. Process those HRTF responses in **MATLAB** to generate FIR filter coefficients.
3. Load those coefficients into a **Teensy** board, which:
   - plays a mono WAV recording (e.g. siren),
   - filters it with the appropriate HRTF FIR filters,
   - outputs a binaural (stereo) signal to headphones.
4. Control which HRTF filter is used (and therefore the perceived source location) via:
   - buttons (changing azimuth/elevation),
   - a potentiometer (continuous position control),
   - or data received from a **Unity** scene via serial.

The result is a 3D audio simulation where the same audio file can be perceived as coming from different directions in space.

---

## 2. Hardware (Teensy)

### 2.1 Components

- Teensy board (Teensy Audio-capable MCU)
- Headphones / audio output
- Potentiometer
- Push buttons
- USB connection to PC (for serial communication)

### 2.2 Firmware

The Teensy firmware is written in Arduino-style C/C++ and performs the following:

- Initializes the Teensy Audio library / audio pipeline
- Loads FIR filter coefficients (HRTF) into FIR filter objects
- Plays a mono WAV audio file
- Applies HRTF filters to generate stereo output
- Changes the active filter set based on:
  - button presses (discrete position changes),
  - potentiometer position (continuous mapping),
  - azimuth/distance values received via serial from Unity.

Firmware sources are in:

`/firmware/`

---

## 3. MATLAB – HRTF Processing

In MATLAB, the following steps are performed:

1. Load HRTF/HRIR data from a public database.
2. Select the HRTF that best matches the listener's ear shape.
3. Extract FIR filter coefficients for multiple positions (different azimuths and elevations).
4. Store all required filter sets (e.g. for the whole sphere or for a set of angles) in a format that can be transferred to the Teensy.
5. Export the coefficients (e.g. as C arrays or text) to be included in the Teensy firmware.

MATLAB scripts are located in:

`/matlab/`

(Once the MATLAB code is added.)

---

## 4. Unity and Blender – 3D Scene

The visual/VR side of the project uses:

- **Blender** to create or prepare the 3D model (e.g. an ambulance as a sound source).
- **Unity** to:
  - build the scene (e.g. an ambulance driving past the listener),
  - compute the source position (azimuth, distance),
  - send this data over serial to the Teensy board.

### 4.1 Data Flow

- Unity calculates the current position of the sound source in the scene.
- A C# script converts this position into:
  - azimuth angle,
  - distance from listener.
- These values are sent to the Teensy over a serial port.
- Teensy chooses or interpolates the appropriate HRTF filters and adjusts:
  - FIR filter coefficients,
  - volume (e.g. 1/r attenuation with distance).

Unity project / scripts are in:

`/unity/`

Blender scenes (if included) are in:

`/blender/`

---

## 5. System Architecture

High-level block diagram of the system:

- **MATLAB**  
  → Computes HRTF FIR coefficients for many positions  
  → Exports coefficients for Teensy

- **Teensy**  
  → Loads HRTF coefficients  
  → Plays mono WAV audio  
  → Applies FIR filters (HRTF)  
  → Outputs stereo signal to headphones

- **Unity + Blender**  
  → Provides 3D scene and motion (e.g. moving ambulance)  
  → Calculates azimuth + distance  
  → Sends position info via serial to Teensy

Any diagrams or figures are stored in:

`/docs/` or `/media/`

---

## 6. How to Run

> Note: Exact steps depend on your environment and versions. Adjust as needed.

### 6.1 Requirements

- MATLAB (for running and modifying HRTF processing scripts)
- Teensy board with Teensyduino / Arduino IDE
- Unity (version used in the project)
- Blender (optional, for editing scene geometry)
- Headphones or speakers

### 6.2 Steps

1. **MATLAB**  
   - Run the scripts in `/matlab/` to generate or update HRTF FIR coefficients.

2. **Teensy**  
   - Open the firmware in `/firmware/` using Arduino IDE or PlatformIO.
   - Ensure the filter coefficient files are included (e.g. header with arrays).
   - Upload the firmware to the Teensy board.

3. **Unity**  
   - Open the Unity project in `/unity/`.
   - Configure the correct serial port and baud rate in the Unity script.
   - Run the scene (Play mode) and verify that the source moves (e.g. ambulance passes by).

4. **Listen**  
   - Wear headphones.
   - As the scene runs or you rotate / move the source, you should hear the sound moving around your head.

---

## 7. Documentation

All official lab reports and the presentation are in:

`/docs/`

This includes:

- NR_porocila_ALJAZ_MUROVEC_1.pdf – Intro, Teensy + HRTF selection
- NR_porocila_ALJAŽ_MUROVEC_2.pdf – MATLAB HRTF processing and filter export
- NR_porocila_ALJAŽ_MUROVEC_3.pdf – Button-based filter switching on Teensy
- NR_porocila_ALJAZ_MUROVEC_4.pdf – Potentiometer control and Unity integration
- NR_predstavitev_AljazMurovec_GasperSkornik.pdf – Project presentation

---

## 8. Media

Screenshots, photos and visual assets are in:

`/media/`

Example usage in this README:

```markdown
![Teensy setup](media/teensy_setup.png)
![Unity scene](media/unity_scene.png)
