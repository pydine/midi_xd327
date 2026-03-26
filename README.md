# USB MIDI Key Matrix (STM32 Blue Pill + 74HC4067)

This project scans a salvaged Yamaha V50 matrix keyboard (Part Number XD327) and sends USB MIDI Note On/Off from an STM32 Blue Pill flashed with the STM32Duino firmware. The scanning logic uses a 74HC4067 mux to select column contacts and reads 11 row inputs with pull-downs.

## Hardware
- MCU: STM32F103 "Blue Pill" with STM32Duino firmware
- Mux: 74HC4067 (16-channel analog mux)
- Keyboard: Yamaha V50 matrix, Part Number XD327 (Make/Break contacts)
- USB: USBComposite (USB MIDI device)

## Pin Map
### 74HC4067 control
- S0 -> PA9
- S1 -> PA8
- S2 -> PB15
- S3 -> PB14
- MUX_OUT -> PB13 (mux common I/O)

### Row inputs (with internal pull-downs)
- ROW0  -> PB1
- ROW1  -> PB0
- ROW2  -> PA7
- ROW3  -> PA6
- ROW4  -> PA5
- ROW5  -> PA4
- ROW6  -> PA3
- ROW7  -> PA2
- ROW8  -> PA1
- ROW9  -> PA0
- ROW10 -> PA10

### LED
- LedPin -> PC13 (MIDI activity LED, wired to 3.3V)

## Matrix Layout
- Rows: 11
- Columns: 6
- Total keys: 66
- Each key uses Make (MK) and Break (BR) contacts.

The code builds two 6-channel lists from the 74HC4067 truth table:
- BR channels: 0..5
- MK channels: 6..11

## Scanning Flow (State Machine)
Each key runs through a 5-state sequence:
1. Idle (state 0): select BR channel and detect initial press.
2. Debounce/confirm (state 1): BR goes low, advance to MK phase.
3. Make (state 2): select MK channel, measure press time, send Note On.
4. Hold (state 3): wait for MK release.
5. Break (state 4): select BR channel, measure release time, send Note Off.

## Velocity Calculation
- Press and release times are measured with `millis()`.
- A logarithmic transform is applied for a more musical response:
  - `logTime = log_multiplier * log(duration)`
- Values are clamped to `[constrain_min, constrain_max]`.
- Press velocity maps to 127..1, release velocity maps to 1..127.

## MIDI Mapping
The `values[11][6]` matrix defines note numbers per row/column. Adjust this table to match your keyboard layout.

## Build and Upload
1. Install STM32Duino core.
2. Install USBComposite library.
3. Select the STM32F103 board profile in the Arduino IDE.
4. Compile and upload `midi.ino`.

## Notes
- This project depends on USBComposite for MIDI enumeration.
- PC13 LED is active-low on many Blue Pill boards; adjust wiring if needed.
- The keyboard's carbon-film aftertouch is not implemented in firmware yet. On hardware, you can drive the on-board op-amp by +/-12V and feed the aftertouch signal into the MCU ADC for pressure sensing (tested).
