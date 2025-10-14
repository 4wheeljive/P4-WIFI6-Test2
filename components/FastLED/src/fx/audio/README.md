# FastLED Audio Effects

Real-time audio processing modules for FastLED, optimized for embedded platforms (ESP32).

## Overview

This directory contains two main audio processing modules:

1. **Beat Detector** - Real-time beat detection and tempo tracking for Electronic Dance Music (EDM)
2. **Sound to MIDI** - Monophonic and polyphonic pitch detection with MIDI event generation

Both modules are designed for real-time operation on resource-constrained platforms with minimal latency and CPU overhead.

---

## Beat Detector

**Files:** `beat_detector.h`, `beat_detector.cpp`

### What is it?

The Beat Detector is a complete real-time beat detection and tempo tracking system optimized for Electronic Dance Music (EDM). It analyzes incoming audio and detects when beats occur, estimates the tempo (BPM), and provides callbacks for onset and beat events.

### Key Features

- **SuperFlux onset detection** with vibrato suppression
- **Multi-band spectral flux** (bass/mid/high weighting for kick drums and hi-hats)
- **Adaptive whitening** for polyphonic music
- **Comb filter tempo tracking** (100-150 BPM range optimized for EDM)
- **Real-time peak picking** with adaptive thresholds
- **~4ms processing budget** target for ESP32

### How It Works

The beat detection pipeline consists of three stages:

#### 1. Onset Detection Function (ODF)

Computes a "novelty curve" that spikes when new musical events occur. Several algorithms are available:

- **Energy** - Simple time-domain energy (fastest, least accurate)
- **Spectral Flux** - Positive magnitude difference in frequency domain (good baseline)
- **SuperFlux** ⭐ - Spectral flux with maximum filter & delayed difference (best for EDM)
  - Uses a maximum filter to suppress vibrato and tremolo
  - Compares current spectrum to a delayed spectrum (μ frames ago)
  - Highly effective for percussive onset detection
- **HFC** - High-frequency content (good for cymbals/hi-hats)
- **Complex Domain** - Phase-aware detection (robust to vibrato)
- **Multi-band** - Multi-band spectral flux with custom frequency weighting

**EDM Optimizations:**
- 24 mel bands for spectral analysis
- 3-band weighting: bass (1.5x for kick drums), mid (1.0x), high (1.2x for hi-hats)
- Adaptive whitening suppresses side-chain pumping effects
- Log compression de-emphasizes large harmonics

#### 2. Peak Picking

Detects peaks in the ODF that represent actual musical onsets:

- **Local Maximum** - Simple local maximum within window
- **Adaptive Threshold** - Threshold based on local mean + delta
- **SuperFlux Peaks** ⭐ - Pre/post/avg windows with minimum distance (best for EDM)
  - Uses both maximum and mean windows for robust peak detection
  - Enforces minimum inter-onset interval (30-50ms) to prevent false triggers

#### 3. Tempo Tracking

Estimates the tempo (BPM) and predicts beat times:

- **Autocorrelation** - Simple autocorrelation of ODF
- **Comb Filter** ⭐ - Autocorrelation + comb filter with Rayleigh weighting (recommended for EDM)
  - Emphasizes periodic structure in the ODF
  - Rayleigh weighting favors typical tempos (120 BPM default)
  - Beat phase tracking for predicted beat times
- **Dynamic Programming** - Advanced beat tracking with tempo changes (higher latency)

**Key Concepts:**
- **Autocorrelation** finds repeating patterns in the ODF over time
- **Comb filter** enhances harmonic relationships (e.g., 120 BPM, 60 BPM, 240 BPM)
- **Rayleigh weighting** biases toward musically plausible tempos

### Quick Start

```cpp
#include "fx/audio/beat_detector.h"

// Create configuration (EDM defaults)
BeatDetectorConfig cfg;
cfg.sample_rate_hz = 48000;
cfg.frame_size = 512;
cfg.hop_size = 256;
cfg.tempo_min_bpm = 100.0f;
cfg.tempo_max_bpm = 150.0f;

// Create detector
BeatDetector detector(cfg);

// Set callbacks
detector.onBeat = [](float confidence, float bpm, float timestamp_ms) {
    Serial.print("BEAT! BPM=");
    Serial.print(bpm);
    Serial.print(", confidence=");
    Serial.println(confidence);
};

detector.onOnset = [](float confidence, float timestamp_ms) {
    Serial.print("Onset at ");
    Serial.print(timestamp_ms);
    Serial.println("ms");
};

// In your audio processing loop:
float audioBuffer[512];
// ... fill audioBuffer with audio samples ...
detector.processFrame(audioBuffer, 512);
```

### Configuration Reference

#### Audio Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `sample_rate_hz` | 48000 | Input sample rate in Hz |
| `frame_size` | 512 | Analysis window size in samples (256-1024) |
| `hop_size` | 256 | Step size between frames (typically frame_size/2) |
| `fft_size` | 512 | FFT size (must be power of 2, >= frame_size) |

#### Onset Detection

| Parameter | Default | Description |
|-----------|---------|-------------|
| `odf_type` | SuperFlux | Onset detection function algorithm |
| `num_bands` | 24 | Number of mel bands for spectral analysis |
| `log_compression` | true | Apply log compression to magnitude spectrum |
| `superflux_mu` | 3 | SuperFlux delay parameter (frames to compare) |
| `max_filter_radius` | 2 | SuperFlux maximum filter radius (bins) |
| `adaptive_whitening` | false | Enable adaptive spectral whitening |

#### Peak Picking

| Parameter | Default | Description |
|-----------|---------|-------------|
| `peak_mode` | SuperFluxPeaks | Peak picking strategy |
| `peak_threshold_delta` | 0.07 | Threshold above local mean (0.05-0.15) |
| `peak_pre_max_ms` | 30 | Pre-window for local maximum (ms) |
| `peak_post_max_ms` | 30 | Post-window for local maximum (ms) |
| `peak_pre_avg_ms` | 100 | Pre-window for mean calculation (ms) |
| `peak_post_avg_ms` | 70 | Post-window for mean calculation (ms) |
| `min_inter_onset_ms` | 30 | Minimum time between onsets (ms) |

#### Tempo Tracking

| Parameter | Default | Description |
|-----------|---------|-------------|
| `tempo_tracker` | CombFilter | Tempo tracking algorithm |
| `tempo_min_bpm` | 100.0 | Minimum tempo in BPM |
| `tempo_max_bpm` | 150.0 | Maximum tempo in BPM |
| `tempo_rayleigh_sigma` | 120.0 | Rayleigh prior center (typical tempo) |
| `tempo_acf_window_sec` | 4 | Autocorrelation window size (seconds) |

#### Multi-band Configuration

The `bands` vector defines custom frequency bands with weights. Default EDM configuration:

```cpp
cfg.bands.push_back({60.0f, 160.0f, 1.5f});   // Bass/kick emphasis
cfg.bands.push_back({160.0f, 2000.0f, 1.0f}); // Mid range
cfg.bands.push_back({2000.0f, 8000.0f, 1.2f});// High/hi-hats emphasis
```

### Performance Characteristics

Based on theoretical complexity and implementation analysis:

| Component | Complexity | Estimated Time @ 512 samples |
|-----------|------------|------------------------------|
| Energy ODF | O(N) | ~0.5ms |
| Spectral Flux | O(K) + FFT | ~2ms |
| SuperFlux | O(K·μ) + FFT | ~3ms |
| Autocorrelation | O(N·L) every 0.5s | ~0.5ms (amortized) |
| **Total (SuperFlux + CombFilter)** | - | **~4ms** ✅ |

**Memory footprint:** ~50KB for all buffers (suitable for ESP32 with 520KB SRAM)

### Advanced Usage

#### Custom ODF Types

```cpp
// High-frequency content (good for hi-hats/cymbals)
cfg.odf_type = OnsetDetectionFunction::HFC;

// Multi-band with custom weighting
cfg.odf_type = OnsetDetectionFunction::MultiBand;
cfg.bands.clear();
cfg.bands.push_back({80.0f, 120.0f, 2.0f});   // Focus on kick drum
cfg.bands.push_back({8000.0f, 12000.0f, 1.5f}); // Focus on hi-hats
```

#### Processing Pre-computed FFT

```cpp
// If you already have an FFT for other purposes
float magnitude_spectrum[256];
// ... compute FFT magnitude ...
detector.processSpectrum(magnitude_spectrum, 256, timestamp_ms);
```

#### Accessing Statistics

```cpp
TempoEstimate tempo = detector.getTempo();
Serial.print("BPM: ");
Serial.print(tempo.bpm);
Serial.print(", confidence: ");
Serial.println(tempo.confidence);

Serial.print("Total onsets detected: ");
Serial.println(detector.getOnsetCount());

Serial.print("Total beats detected: ");
Serial.println(detector.getBeatCount());

Serial.print("Current ODF value: ");
Serial.println(detector.getCurrentODF());
```

### Algorithm Details

#### SuperFlux Onset Detection

SuperFlux improves upon basic spectral flux by suppressing vibrato and tremolo:

1. **Delayed Difference**: Compares current spectrum to spectrum from μ frames ago (typically μ=3)
2. **Maximum Filter**: Applies a maximum filter to the delayed spectrum to suppress rapid fluctuations
3. **Positive Rectification**: Only positive differences are counted (increasing energy)

This makes it highly effective for percussive onset detection in EDM tracks with side-chain compression.

#### Comb Filter Tempo Tracking

The comb filter enhances the autocorrelation function by emphasizing harmonic relationships:

1. **Autocorrelation**: Compute correlation of ODF with itself at various lags
2. **Comb Filtering**: For each lag, add contributions from harmonic multiples (2×, 3×, 4×)
3. **Rayleigh Weighting**: Multiply by Rayleigh distribution centered on target tempo
4. **Peak Finding**: Find peak within BPM range (constrained by min/max BPM)

The Rayleigh distribution provides a prior that favors musically plausible tempos around 120 BPM.

### Research Background

Based on academic research in onset detection and beat tracking:

- Böck & Widmer (2013) - Maximum Filter Vibrato Suppression for Onset Detection
- Davies & Plumbley (2004) - Causal Tempo Tracker (ACF + Comb Filter)
- Ellis (2007) - Dynamic Programming Beat Tracker
- Masri (1996) - High-Frequency Content Method
- Subramani et al. (2018) - Energy-Weighted Multi-Band Novelty Functions

### Example

See `examples/BeatDetection/BeatDetection.ino` for a complete example with LED visualization.

---

## Sound to MIDI

**Files:** `sound_to_midi.h`, `sound_to_midi.cpp`

### What is it?

Sound to MIDI converts audio input into MIDI Note On/Off events. It supports both **monophonic** (single note) and **polyphonic** (multiple simultaneous notes) pitch detection, with automatic velocity calculation and anti-jitter filtering.

### Key Features

- **Monophonic pitch detection** using YIN/MPM algorithm
- **Polyphonic pitch detection** using FFT-based peak detection
- **RMS-based velocity** calculation with configurable gain
- **Anti-jitter filtering** (semitone threshold, hold frames, median filter)
- **Onset/offset hysteresis** to prevent rapid note toggling
- **Sliding window STFT** - integrated ring buffer with configurable overlap (hop_size < frame_size)
- **K-of-M multi-frame onset detection** - reduces spurious events by requiring persistence across frames
- **Peak continuity tracking** - per-note frequency/magnitude history for improved stability (polyphonic)
- **Auto-tuning** - automatically adapts thresholds based on input characteristics (noise floor, event rate, jitter)
- **Callback-based event system** for Note On/Off events

### How It Works

#### Monophonic Mode (Single Notes)

Uses a **YIN/MPM-like autocorrelation algorithm** for pitch detection:

1. **Sample Buffering** (if `hop_size < frame_size`): Incoming samples accumulate in ring buffer
2. **Frame Extraction**: Every `hop_size` samples, extract last `frame_size` samples with Hann windowing
3. **Pitch Detection**: Analyzes windowed frame to find fundamental frequency via autocorrelation
4. **Confidence Check**: Only accepts pitch if confidence exceeds threshold (default 0.80, auto-tuned if enabled)
5. **Note Conversion**: Converts frequency to MIDI note number
6. **K-of-M Filtering** (optional): Require pitch to appear in K of last M frames before onset
7. **Stability Filtering**:
   - Semitone threshold - ignore small pitch changes
   - Hold frames - require note to persist before switching
   - Median filter - smooth out noisy detections (auto-adjusted based on jitter)
8. **Onset/Offset Detection**:
   - Onset: Note held for `note_hold_frames` consecutive frames
   - Offset: Silence detected for `silence_frames_off` consecutive frames
9. **Velocity Calculation**: RMS amplitude × gain → MIDI velocity (1-127)
10. **Auto-tuning** (optional): Adapts RMS gate, confidence threshold, median filter size based on noise floor and jitter

#### Polyphonic Mode (Multiple Notes)

Uses **FFT-based spectral peak detection**:

1. **Sample Buffering** (if `hop_size < frame_size`): Incoming samples accumulate in ring buffer
2. **Frame Extraction**: Every `hop_size` samples, extract last `frame_size` samples with windowing
3. **FFT Analysis**: Computes magnitude spectrum with optional windowing (Hann/Hamming/Blackman)
4. **Spectral Conditioning**:
   - Spectral tilt compensation
   - Magnitude smoothing (box, triangular, or adjacent average)
5. **Peak Detection**: Finds spectral peaks above threshold (auto-tuned if enabled)
6. **Parabolic Interpolation**: Refines peak frequencies to sub-bin accuracy
7. **Harmonic Filtering**: Suppresses overtones that are harmonics of fundamentals
8. **Octave Masking**: Filter peaks by octave range (bitmask)
9. **K-of-M Filtering** (optional): Per-note persistence tracking - require K detections in M frames
10. **Peak Continuity**: Track per-note frequency/magnitude history for stable sustained notes
11. **PCP Stabilization** (optional): Pitch class profile helps stabilize note detection
12. **Note Tracking**: Independent onset/offset for each note (128 MIDI notes)
13. **Auto-tuning** (optional): Adapts peak threshold, harmonic filter settings based on noise floor and event rate

### Quick Start - Monophonic

```cpp
#include "fx/audio/sound_to_midi.h"

// Create configuration with sliding window and auto-tuning
SoundToMIDI cfg;
cfg.sample_rate_hz = 16000;
cfg.frame_size = 512;
cfg.hop_size = 256;              // 50% overlap for improved onset detection
cfg.confidence_threshold = 0.82f;

// Optional: Enable K-of-M filtering to reduce spurious events
cfg.enable_k_of_m = true;
cfg.k_of_m_onset = 2;            // Require 2 detections in last 3 frames
cfg.k_of_m_window = 3;

// Optional: Enable auto-tuning for adaptive thresholds
cfg.auto_tune_enable = true;     // Automatically adapts to noise floor

// Create monophonic engine
SoundToMIDIMono engine(cfg);

// Set callbacks
engine.onNoteOn = [](uint8_t note, uint8_t velocity) {
    Serial.print("Note ON: ");
    Serial.print(note);
    Serial.print(" vel=");
    Serial.println(velocity);
};

engine.onNoteOff = [](uint8_t note) {
    Serial.print("Note OFF: ");
    Serial.println(note);
};

// In your audio processing loop (can feed any chunk size):
float audioBuffer[512];
// ... fill audioBuffer with audio samples ...
engine.processFrame(audioBuffer, 512);
```

### Quick Start - Polyphonic

```cpp
#include "fx/audio/sound_to_midi.h"

// Create configuration with sliding window and advanced features
SoundToMIDI cfg;
cfg.sample_rate_hz = 44100;
cfg.frame_size = 2048;
cfg.hop_size = 512;              // 75% overlap for dense chord analysis
cfg.fmin_hz = 80.0f;             // Lower limit for guitar/piano
cfg.fmax_hz = 2000.0f;

// Polyphonic-specific settings
cfg.window_type = WindowType::Hann;
cfg.harmonic_filter_enable = true;
cfg.parabolic_interp = true;
cfg.pcp_enable = true;           // Pitch class profile for stability

// Optional: Enable K-of-M filtering (per-note persistence)
cfg.enable_k_of_m = true;
cfg.k_of_m_onset = 3;            // Require 3 detections in last 4 frames
cfg.k_of_m_window = 4;

// Optional: Enable auto-tuning for dynamic environments
cfg.auto_tune_enable = true;
cfg.auto_tune_peak_margin_db = 6.0f;  // Adaptive peak threshold

// Create polyphonic engine
SoundToMIDIPoly engine(cfg);

// Optional: Monitor auto-tuning adjustments
engine.setAutoTuneCallback([](const char* param, float old_val, float new_val) {
    Serial.print("Auto-tune: ");
    Serial.print(param);
    Serial.print(" ");
    Serial.print(old_val);
    Serial.print(" → ");
    Serial.println(new_val);
});

// Set callbacks (same as monophonic)
engine.onNoteOn = [](uint8_t note, uint8_t velocity) {
    // Multiple notes can trigger simultaneously
    Serial.print("Note ON: ");
    Serial.println(note);
};

engine.onNoteOff = [](uint8_t note) {
    Serial.print("Note OFF: ");
    Serial.println(note);
};

// Process audio (can feed any chunk size)
engine.processFrame(audioBuffer, 2048);
```

### Configuration Reference

#### Audio Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `sample_rate_hz` | 16000 | Input sample rate (16000-48000 Hz) |
| `frame_size` | 512 | Analysis window size (512 for 16kHz, 1024+ for 44kHz) |
| `hop_size` | 512 | Step size between frames (set < frame_size for overlap, e.g., 256 = 50% overlap) |

#### Pitch Detection Range

| Parameter | Default | Description |
|-----------|---------|-------------|
| `fmin_hz` | 40.0 | Minimum detectable frequency (E1 ≈ 41.2 Hz) |
| `fmax_hz` | 1600.0 | Maximum detectable frequency (G6 ≈ 1568 Hz) |

#### Detection Thresholds

| Parameter | Default | Description |
|-----------|---------|-------------|
| `confidence_threshold` | 0.80 | Minimum confidence [0-1] to accept pitch |
| `note_hold_frames` | 3 | Consecutive frames required before Note On |
| `silence_frames_off` | 3 | Consecutive silent frames before Note Off |
| `rms_gate` | 0.010 | RMS threshold below which signal is silent |

#### Velocity Calculation

| Parameter | Default | Description |
|-----------|---------|-------------|
| `vel_gain` | 5.0 | Gain multiplier for RMS → velocity |
| `vel_floor` | 10 | Minimum MIDI velocity (1-127) |
| `velocity_from_peak_mag` | true | Use peak magnitude for velocity (poly only) |

#### Anti-Jitter (Monophonic)

| Parameter | Default | Description |
|-----------|---------|-------------|
| `note_change_semitone_threshold` | 1 | Semitones required to trigger note change |
| `note_change_hold_frames` | 3 | Frames new note must persist before switching |
| `median_filter_size` | 1 | Median filter window (1=off, 3-5 for noisy input, auto-adjusted if auto-tune enabled) |

#### K-of-M Multi-frame Filtering

| Parameter | Default | Description |
|-----------|---------|-------------|
| `enable_k_of_m` | false | Enable K-of-M onset persistence filtering |
| `k_of_m_onset` | 2 | Require K detections in last M frames (monophonic) or per-note (polyphonic) |
| `k_of_m_window` | 3 | Window size M for K-of-M detection |

#### Polyphonic Spectral Processing

| Parameter | Default | Description |
|-----------|---------|-------------|
| `window_type` | Hann | Window function (None, Hann, Hamming, Blackman) |
| `spectral_tilt_db_per_decade` | 0.0 | Spectral tilt compensation (+3 boosts highs) |
| `smoothing_mode` | Box3 | Magnitude smoothing (None, Box3, Tri5, AdjAvg) |
| `peak_threshold_db` | -40.0 | Magnitude threshold for peak detection (dB) |
| `parabolic_interp` | true | Sub-bin frequency refinement |
| `harmonic_filter_enable` | true | Suppress overtones |
| `harmonic_tolerance_cents` | 35.0 | Cents tolerance for harmonic detection |
| `octave_mask` | 0xFF | Bitmask for enabled octaves (bit 0-7) |
| `pcp_enable` | false | Enable pitch class profile stabilizer |

#### Auto-Tuning

Auto-tuning automatically adapts detection thresholds based on input characteristics:

| Parameter | Default | Description |
|-----------|---------|-------------|
| `auto_tune_enable` | false | Enable auto-tuning (master switch) |
| `auto_tune_rms_margin` | 1.8 | RMS gate = noise_floor × margin (1.5-2.0) |
| `auto_tune_peak_margin_db` | 8.0 | Peak threshold = noise_floor_db + margin (6-10 dB) |
| `auto_tune_update_rate_hz` | 5.0 | Update frequency (5-10 Hz recommended) |
| `auto_tune_param_smoothing` | 0.95 | Smoothing factor for updates (0.9-0.99, higher = smoother) |
| `auto_tune_threshold_step` | 0.02 | Step size for threshold adjustments |
| `auto_tune_calibration_time_sec` | 1.0 | Initial calibration period (seconds) |
| `auto_tune_rms_gate_min/max` | 0.005/0.100 | Limits for RMS gate adaptation |
| `auto_tune_confidence_min/max` | 0.60/0.95 | Limits for confidence threshold (mono) |
| `auto_tune_peak_db_min/max` | -60.0/-20.0 | Limits for peak threshold (poly) |
| `auto_tune_notes_per_sec_min/max` | 1.0/10.0 | Target event rate range (mono) |
| `auto_tune_peaks_per_frame_min/max` | 1.0/5.0 | Target peak rate range (poly) |

**What Auto-Tuning Does:**
- **Noise Floor Tracking**: Estimates ambient noise during silence (EMA with slow time constant)
- **Adaptive Gates**: Adjusts `rms_gate` (mono) and `peak_threshold_db` (poly) based on noise floor + margin
- **Jitter Monitoring**: Tracks pitch variance and adjusts `median_filter_size` (mono) for stability
- **Event Rate Control**: Adjusts thresholds to maintain target note/peak rates (prevents floods or missed notes)
- **Hold-time Optimization**: Learns typical note durations and gaps to set optimal `note_hold_frames`
- **Calibration**: 1-second startup period to establish initial noise floor

Enable auto-tuning for environments with varying noise levels, dynamic range, or when optimal settings are unknown.

### Advanced Usage

#### Sliding Window with Custom Overlap

```cpp
SoundToMIDI cfg;
cfg.frame_size = 1024;
cfg.hop_size = 256;     // 75% overlap (4× analysis density)

SoundToMIDIMono engine(cfg);

// Feed audio in arbitrary chunk sizes - engine handles buffering
float chunk[128];
while (audio_available) {
    read_audio(chunk, 128);
    engine.processFrame(chunk, 128);  // Analysis triggered every hop_size samples
}
```

**Overlap Guidelines:**
- **50% overlap** (hop = frame/2): Good balance, 2× CPU cost
- **75% overlap** (hop = frame/4): Better onset detection, 4× CPU cost
- **No overlap** (hop = frame): Legacy mode, minimal CPU

#### K-of-M Filtering for Noise Rejection

```cpp
cfg.enable_k_of_m = true;
cfg.k_of_m_onset = 2;     // Require 2 detections
cfg.k_of_m_window = 3;    // In last 3 frames

// Monophonic: Reduces spurious onset/offset events
// Polyphonic: Per-note tracking - each MIDI note requires K detections
```

**Recommended Settings:**
- **Clean input**: K=2, M=3 (minimal filtering)
- **Moderate noise**: K=2, M=4 or K=3, M=4
- **High noise**: K=3, M=5 (aggressive filtering, slight latency increase)

#### Auto-Tuning Monitoring

```cpp
engine.setAutoTuneCallback([](const char* param_name, float old_val, float new_val) {
    Serial.print("Auto-tune: ");
    Serial.print(param_name);
    Serial.print(" changed from ");
    Serial.print(old_val);
    Serial.print(" to ");
    Serial.println(new_val);
});

const AutoTuneState& state = engine.getAutoTuneState();
Serial.print("Noise floor: ");
Serial.println(state.noise_rms_est);
Serial.print("Confidence EMA: ");
Serial.println(state.confidence_ema);
Serial.print("Event rate: ");
Serial.println(state.event_rate_ema);
Serial.print("Pitch variance: ");
Serial.println(state.pitch_variance_ema);

// Check if still in calibration phase
if (state.in_calibration) {
    Serial.println("Calibrating...");
}
```

#### Polyphonic Runtime Adjustments

```cpp
SoundToMIDIPoly poly(cfg);

// Adjust sensitivity
poly.setPeakThresholdDb(-35.0f);  // More sensitive

// Filter octave range
poly.setOctaveMask(0x3C);  // Only octaves 2-5 (bits 2,3,4,5)

// Boost high frequencies
poly.setSpectralTilt(3.0f);  // +3 dB/decade

// Change smoothing
poly.setSmoothingMode(SmoothingMode::Tri5);  // Triangular 5-point
```

#### Frequency to MIDI Note Conversion

The conversion from frequency to MIDI note number uses the formula:

```
MIDI note = 69 + 12 × log₂(frequency / 440 Hz)
```

Where 69 is MIDI note A4 (440 Hz). For example:
- C4 (Middle C) = 60 (261.63 Hz)
- A4 = 69 (440 Hz)
- C5 = 72 (523.25 Hz)

### Algorithm Details

#### Sliding Window STFT (Integrated into Core)

The sliding window provides overlapped analysis for both mono and poly engines:

**Ring Buffer Management:**
- Size: `frame_size + hop_size` floats
- Accumulates incoming samples regardless of chunk size
- When `accumulated >= hop_size`, triggers analysis

**Frame Extraction:**
- Extract last `frame_size` samples from ring buffer
- Apply Hann window: `w[n] = 0.5 × (1 - cos(2π×n/(N-1)))`
- Pass windowed frame to pitch detection

**Overlap Benefits:**
- Eliminates edge effects at frame boundaries
- Improves onset detection recall (catches events missed between frames)
- Reduces spectral leakage through windowing
- Enables multi-frame K-of-M filtering

#### YIN/MPM Pitch Detection (Monophonic)

The YIN algorithm finds the fundamental frequency by:

1. **Autocorrelation**: Compute difference function d(τ) for various lags τ
2. **Cumulative Mean Normalization**: Normalize d(τ) to get d'(τ)
3. **Threshold Crossing**: Find first τ where d'(τ) < threshold
4. **Parabolic Interpolation**: Refine τ to sub-sample accuracy
5. **Frequency Calculation**: f₀ = sample_rate / τ

**Confidence** is derived from 1 - d'(τ), where lower difference means higher confidence.

#### K-of-M Multi-frame Onset Detection

Reduces spurious events by requiring persistence across frames:

**Monophonic Mode:**
- Circular buffer tracks last M onset/voiced states (bool array)
- On each frame, check if ≥K of last M frames are voiced
- Emit NoteOn only when K-of-M threshold met
- Same logic for NoteOff (require K silent frames in M)

**Polyphonic Mode:**
- Per-note tracking: 128 independent K-of-M counters
- Each MIDI note tracks on_count and off_count
- Note emitted when specific note appears in K of M frames
- Prevents one-frame blips without global suppression

**Latency Impact:**
- Worst case: `(M - K + 1) × hop_size / sample_rate`
- Example: K=2, M=3, hop=256, fs=16000 → 32ms additional latency
- Benefit: Dramatically reduced false trigger rate

#### FFT Peak Detection (Polyphonic)

Polyphonic mode analyzes the frequency spectrum:

1. **Windowing**: Apply window function (Hann/Hamming/Blackman) to reduce spectral leakage
2. **FFT**: Compute magnitude spectrum
3. **Conditioning**: Apply tilt compensation and smoothing
4. **Peak Finding**: Detect local maxima above threshold
5. **Interpolation**: Parabolic interpolation for sub-bin accuracy
6. **Harmonic Filtering**: Remove peaks that are harmonics of lower peaks
7. **Note Tracking**: Manage Note On/Off for each detected note independently

#### Auto-Tuning Adaptive Algorithms

**Noise Floor Estimation:**
- RMS amplitude EMA (α=0.05, slow tracking)
- Spectral magnitude median/EMA in dB
- Updated only during silence (mono) or low peak count (poly)
- Time constant: ~0.5 seconds

**Adaptive Thresholds:**
```
rms_gate = max(min_rms, noise_rms_est × margin)
peak_threshold_db = noise_mag_db_est + margin_db
```

**Jitter Monitoring (Monophonic):**
- Tracks pitch variance as squared semitone difference
- High variance (>1.0): increase median_filter_size to 3-5
- Low variance (<0.25): reduce to 1 for minimal latency

**Event Rate Control:**
- Monophonic: Track note-on events per second
- Polyphonic: Track peaks per frame
- Adjust thresholds to maintain target ranges
- Prevents both floods and starvation

**Hold-time Optimization:**
- EMA of note durations and inter-note gaps
- `note_hold_frames = 0.75 × avg_duration / hop_period`
- `silence_frames_off = 0.5 × avg_gap / hop_period`
- Bounded to reasonable ranges (1-10 frames)

**Calibration Phase:**
- 1-second startup period (configurable)
- Establishes initial noise floor
- No events emitted during calibration
- Smooth transition to normal operation

### Use Cases

#### Monophonic Applications
- Singing/vocal pitch tracking
- Single-instrument recording (flute, trumpet, etc.)
- Melody extraction
- Pitch-to-CV conversion for synthesizers

#### Polyphonic Applications
- Guitar/piano chord detection
- Multi-voice harmony analysis
- Polyphonic MIDI recording
- Music transcription

### Performance Characteristics

#### CPU Usage

**Monophonic Mode:**
- Base processing: ~1-2ms per frame @ 512 samples (ESP32 @ 240MHz)
- With 50% overlap: ~2-4ms (2× analysis rate)
- With 75% overlap: ~4-8ms (4× analysis rate)
- Auto-tuning overhead: <1% additional CPU
- K-of-M filtering: negligible overhead

**Polyphonic Mode:**
- Base processing: ~5-10ms per frame @ 2048 samples (ESP32-S3 @ 240MHz)
- With 50% overlap: ~10-20ms (2× analysis rate)
- With 75% overlap: ~20-40ms (4× analysis rate)
- Auto-tuning overhead: <1% additional CPU
- K-of-M per-note tracking: ~0.5ms for 128 notes

#### Memory Footprint

**Sliding Window Buffers** (per engine):
- Ring buffer: `frame_size + hop_size` floats (e.g., 1024+256 = 5KB @ 75% overlap)
- Window coefficients: `frame_size` floats (e.g., 4KB for 1024 samples)
- Analysis frame: `frame_size` floats (e.g., 4KB)
- **Total**: ~13KB for frame_size=1024, 75% overlap

**Auto-tuning State:**
- AutoTuneState struct: ~200 bytes
- K-of-M history: ~16 bytes (mono) or ~512 bytes (poly, 128 notes × 4 bytes)
- Peak continuity (poly): ~3KB (128 notes × 24 bytes)

**Total Memory (Polyphonic, Full Features):**
- ~20KB for frame_size=1024, 75% overlap, all features enabled
- Suitable for ESP32/ESP32-S3 (512KB SRAM)

#### Latency

**Algorithmic Latency:**
- Input to first analysis: `frame_size / sample_rate` (e.g., 1024/16000 = 64ms)
- Analysis to next analysis: `hop_size / sample_rate` (e.g., 256/16000 = 16ms with 75% overlap)
- K-of-M adds: ≤ `(M-K+1) × hop_size / sample_rate` (e.g., 2 hops = 32ms for K=2, M=3)

**Total Typical Latency:**
- Monophonic (no overlap): 64ms
- Monophonic (75% overlap, K-of-M): 64ms + 32ms = 96ms
- Polyphonic (75% overlap, K-of-M): 128ms + 48ms = 176ms

**Note:** Overlap *reduces* effective latency for onset detection by analyzing more frequently.

#### Recommendations by Platform

**ESP32 / ESP32-S3 (512KB SRAM):**
- ✅ Monophonic: 50-75% overlap, auto-tune, K-of-M
- ✅ Polyphonic: 50% overlap recommended, 75% possible if frame rate allows
- ✅ All features supported

**ESP32-C3 (400KB SRAM):**
- ✅ Monophonic: All features supported
- ⚠️ Polyphonic: Use frame_size=1024, 50% overlap max

**Memory-constrained platforms (<100KB SRAM):**
- ❌ Not recommended - use simpler beat detector or external processing

---

## Platform Requirements

Both modules require platforms with sufficient memory:

```cpp
#if SKETCH_HAS_LOTS_OF_MEMORY
// Beat detector and sound-to-MIDI available
#else
// Not available on memory-constrained platforms
#endif
```

Tested platforms:
- ✅ ESP32 (520KB SRAM)
- ✅ ESP32-S3 (512KB SRAM)
- ❌ Arduino Uno (2KB SRAM - insufficient)
- ❌ Arduino Nano (2KB SRAM - insufficient)

---

## Implementation Status

### Sound to MIDI Features

#### ✅ Core Detection (100% Complete)
- Monophonic YIN/MPM pitch detection
- Polyphonic FFT spectral peak detection
- RMS-based velocity calculation
- Anti-jitter filtering (semitone threshold, hold frames, median filter)
- Onset/offset hysteresis
- Callback-based event system

#### ✅ Sliding Window STFT (100% Complete)
- Internal ring buffer with configurable overlap (`hop_size < frame_size`)
- Automatic Hann windowing when overlap enabled
- Streaming API - feed arbitrary chunk sizes
- Zero API changes - backward compatible
- Integrated into core engines (not external wrapper)

#### ✅ K-of-M Multi-frame Onset Detection (100% Complete)
- Circular buffer tracking last M frames
- Monophonic: onset/offset persistence filtering
- Polyphonic: per-note tracking (independent K-of-M for each MIDI note)
- Configurable via `enable_k_of_m`, `k_of_m_onset`, `k_of_m_window`
- Disabled by default for backward compatibility

#### ✅ Peak Continuity Tracking (Infrastructure Complete, 80%)
- Per-note frequency and magnitude history
- Frames-absent tracking for gap detection
- Foundation for advanced matching algorithms
- **Future**: Cross-frame peak matching with continuity_cents threshold

#### ✅ Auto-Tuning (100% Complete)
- Noise floor estimation (RMS and spectral magnitude)
- Adaptive thresholds (RMS gate, confidence, peak_threshold_db)
- Jitter monitoring with median filter adaptation
- Event rate control (notes/sec for mono, peaks/frame for poly)
- Hold-time optimization based on learned note durations
- Octave statistics and harmonic filter adaptation (poly)
- Calibration phase on startup
- API: `getAutoTuneState()`, `setAutoTuneCallback()`
- <1% CPU overhead, <1KB memory footprint

### Testing Status
- ✅ All existing tests pass (backward compatibility verified)
- ✅ 9 new sliding window tests (4 mono, 5 poly)
- ✅ K-of-M filtering tests
- ✅ Auto-tuning tests
- ✅ Linting passes (C++, Python, JavaScript)
- ✅ Compilation on ESP32 platforms

---

## Examples

- **`examples/BeatDetection/BeatDetection.ino`** - Real-time beat detection with LED visualization
  - Demonstrates all ODF types and configuration options
  - LED strip flashes on beat with BPM-based color coding
  - Comprehensive UI controls for parameter tuning

**Note:** Sound to MIDI examples coming soon demonstrating sliding window, K-of-M, and auto-tuning features.

---

## License

MIT License (same as FastLED)

---

## References

### Beat Detector
1. Böck & Widmer (2013) - Maximum Filter Vibrato Suppression for Onset Detection
2. Davies & Plumbley (2004) - Causal Tempo Tracker (ACF + Comb Filter)
3. Ellis (2007) - Dynamic Programming Beat Tracker
4. Masri (1996) - High-Frequency Content Method
5. Subramani et al. (2018) - Energy-Weighted Multi-Band Novelty Functions

### Sound to MIDI - Core Algorithms
1. De Cheveigné & Kawahara (2002) - YIN, a fundamental frequency estimator for speech and music
2. McLeod & Wyvill (2005) - A smarter way to find pitch (MPM)
3. Smith (2011) - Spectral Audio Signal Processing (parabolic interpolation)

### Sound to MIDI - Advanced Features
See implementation documentation in project root:
- **`AUTO_TUNE_IMPLEMENTATION.md`** - Auto-tuning extension design and implementation
- **`TASK.md`** - Sliding window STFT design and multi-frame logic
- **`TASK2.md`** - Core integration plan for sliding window (completed)

These documents provide detailed algorithms, design decisions, and testing strategies for the advanced features.

---

## Contributing

See project root `CLAUDE.md` for coding standards and guidelines.
