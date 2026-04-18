# Spec: Host Rendering Smoothness Improvement (Generic)

## Goal
Improve the visual quality of the SuperWaveform plot by distributing data points evenly along the time axis. This must be a generic optimization that adapts to different sampling rates from the MCU without hardcoding specific values.

## Proposed Changes

### 1. Adaptive Sample Rate Estimation
Instead of a hardcoded rate, `GuiLayer` will estimate the incoming data rate dynamically.
- **State**: 
    - `double m_estimatedPeriod`: Current estimated time between samples.
    - `double m_lastPacketTime`: Real time when the last packet arrived.
    - `int m_pointsInCurrentWindow`: Count of points received in the last 500ms.
- **Algorithm**:
    - Every 500ms, update `m_estimatedPeriod = 0.5 / m_pointsInCurrentWindow`.
    - Use a simple low-pass filter: `m_smoothedPeriod = m_smoothedPeriod * 0.9 + m_estimatedPeriod * 0.1`.

### 2. Elastic Jitter Buffer Logic
When data points are extracted from the network buffer:
- **State**: `double m_virtualClock` (The X-coordinate of the last plotted point).
- **Logic**:
    - `double wallClock = ImGui::GetTime();`
    - For each point:
        - If `(wallClock - m_virtualClock) > m_smoothedPeriod * 100.0` (Resync trigger):
            - `m_virtualClock = wallClock;`
        - Else:
            - `m_virtualClock += m_smoothedPeriod;`
        - Use `m_virtualClock` for plotting.

### 3. UI Controls
Add a "Sampling Info" section to the dashboard to display the detected sample rate (Hz).

## Verification Plan
- **Variable Frequency Test**: Change the MCU's `s_wPhaseStep` and verify that the host adapts its rendering smoothness within seconds.
- **Jitter Test**: Simulate network jitter and verify that the wave remains a continuous line rather than jumping in blocks.
