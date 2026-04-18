# Spec: Waveform Generation Quality Improvement

## Goal
Improve the "sinusoidality" and frequency control of the test waveform generator in the BLM example. Resolve the jagged waveform issues caused by incorrect LUT values and fixed-integer phase stepping.

## Proposed Changes

### 1. Corrected Sine Look-Up Table (LUT)
Replace the current `s_ahwSinTable` in `blm_waveform_test.c` with a high-precision, symmetric 256-point table.
- **Points**: 256
- **Range**: `[-32767, 32767]` (Q15-like format)
- **Symmetry**: Ensure `sin(0) = 0`, `sin(90) = 32767`, etc.

### 2. Phase Accumulator Implementation
Replace the 8-bit `s_chPhase` with a 32-bit accumulator.
- **Variables**:
    - `static uint32_t s_wPhaseAcc`: Current phase.
    - `static uint32_t s_wPhaseStep`: Delta phase per 1ms tick.
- **Algorithm**:
    - In every `blm_waveform_test_step()` (1ms):
        - `s_wPhaseAcc += s_wPhaseStep;`
        - `uint8_t index = (uint8_t)(s_wPhaseAcc >> 24);`
        - `int16_t val = s_ahwSinTable[index];`

### 3. Frequency Control
Default frequency set to **5.0 Hz**.
- **Calculation**: `Step = (Frequency * 2^32) / SampleRate`
- For 5Hz @ 1kHz: `Step = (5 * 4294967296) / 1000 = 21474836`.

## Verification Plan
- **Visual Check**: Observe the waveform in SuperWaveform. It should be a smooth, continuous sine wave without visible "steps" or noise at the peaks.
- **Frequency Check**: The wave should complete exactly 5 cycles per second on the time axis.
