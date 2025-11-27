# 3D-Audio-Teensy-VR  
**Real-time 3D audio with custom HRTFs (Teensy + Unity + MATLAB)**

## 🎯 Project Overview

This project implements a pipeline for **personalised binaural 3D-audio** using:

- An HRIR/HRTF dataset processed by MATLAB into FIR filter coefficients.  
- A **Teensy 4.x + Audio Shield** running a sketch that applies FIR filtering in real time.  
- (Optionally) A 3D/VR environment (e.g. Unity) where a virtual sound source moves; spatial data (azimuth, distance) is sent via serial to the Teensy to update HRTF filters and volume.  

This allows for spatial audio where the sound appears to come from specific directions — tailored to the listener’s own ears (based on HRIR).

---

## 📂 Repository Structure

3D-Audio-Teensy-VR/
│
├── firmware/
│ └── usb_vr.ino ← Teensy firmware (audio + FIR + HRTF filtering)
│ └── hrtf_filters.h ← Auto-generated HRTF FIR coefficients
│
├── matlab/
│ └── export_hrir_to_teensy.m ← MATLAB script to export filters
│ └── IRC_1038_C_HRIR.mat ← Example compensated HRIR dataset
│
├── unity/ ← Placeholder for Unity / 3D audio integration
│
└── README.md ← This file
└── docs/ ← (Optional) documentation, diagrams, notes


> **Note:** There is no `.gitignore`, and example audio files (e.g. WAVs) are omitted by design.  
> If you use your own audio or wish to demo sound playback, add those files locally (not committed in this repo).

---

## 🛠 Usage

### 1. Generate HRTF FIR Filters (MATLAB)

1. Place your HRIR `.mat` file inside the `matlab/` folder. The structure **must** define:

   - `l_eq_hrir_S` (left-ear data)  
   - `r_eq_hrir_S` (right-ear data)  

   Both should contain:

   - `.content_m` — HRIR impulse responses (channels × samples)  
   - `.azim_v` — vector of azimuth angles (deg)  
   - `.elev_v` — vector of elevation angles (deg)  

   The supplied example file (`IRC_1038_C_HRIR.mat`) follows this format.

2. In `export_hrir_to_teensy.m`, optionally set:

   ```matlab
   targetElev = 0;  % select elevation plane (degrees)
   numTaps   = 128; % # FIR taps to use
   ```

3. Run the script in MATLAB:

   ```matlab
   cd matlab
   export_hrir_to_teensy
   ```

   The script will output:
   ```
   Using X positions at elevation Y deg.
   Wrote ../firmware/hrtf_filters.h
   ```

4. The generated `hrtf_filters.h` contains:

   - `NUM_TAPS`, `NUM_POSITIONS` — constants defining filter dimensions
   - `float hrtfAzimuth[NUM_POSITIONS]` — azimuth list (deg)
   - `int16_t hrtfL[NUM_POSITIONS][NUM_TAPS]` — left-ear filter coefficients
   - `int16_t hrtfR[NUM_POSITIONS][NUM_TAPS]` — right-ear filter coefficients

---

### 2. Compile & Upload Firmware to Teensy

1. Open `firmware/usb_vr.ino` using **Arduino IDE** or **PlatformIO**.

2. Ensure target board is **Teensy 4.x**, with Audio Shield settings configured.

3. Upload sketch to Teensy.

4. Once uploaded, Teensy will:

   - Initialize audio I/O (USB and/or SD card)
   - Load FIR filters from `hrtf_filters.h`
   - Play audio input or WAV file (if provided on SD card)
   - Switch HRTF filters on the fly based on serial commands

---

### 3. (Optional) Use Unity / External Controller for Spatial Audio

You can integrate this with a 3D engine or VR environment. The idea:

- The 3D engine tracks the position of a virtual sound source relative to listener (azimuth + distance).
- It sends a serial message (e.g. via USB) to Teensy with a command encoding:
  - **Play/reset flag**
  - **Distance**
  - **HRTF filter index**
- Teensy then updates the FIR filter and volume accordingly, creating a dynamic binaural audio experience.

---

## 🎛 Teensy Serial Protocol & Filter Switching

The Teensy firmware listens to serial lines of the form:

```
[1-digit play/reset][3-digit distance][2-digit filterIndex]\n
```

**Example:** `105012\n`
- `1` → play/reset flag
- `050` → distance = 50 units (user-defined scale)
- `12` → apply filter index 12 (of `NUM_POSITIONS`)

On receiving this command, Teensy executes:
```cpp
applyHrtf(filterIndex);
setVolumeAccordingToDistance(distance);
```

---

## 📐 Notes / Limitations

- **Single Elevation Plane:** Currently only supports a single elevation plane (default: 0°). Can be extended by processing multiple elevations in MATLAB.
- **No Interpolation:** Only discrete azimuth steps are supported — no interpolation between HRTFs.
- **Mono Audio:** Audio source must be mono (or downmixed) for correct binaural spatialization.
- **Hardware Match:** The Teensy Audio Shield and pin-out must match wiring expected by the sketch.
- **User Audio:** Example audio files (e.g. WAV files) are not included — you must supply your own.

---

## 🧠 What's Included & What's Up to You

| Provided by the Repo | User / Optional |
|---|---|
| MATLAB export script & example HRIR dataset | Your own HRIR files |
| Teensy sketch + generated HRTF filters | Your own audio files (WAV / streaming) |
| Unity folder (placeholder) | 3D scenes, controller scripts, head-tracking, UI |
| Documentation folder (optional) | Extra notes / diagrams / demos
