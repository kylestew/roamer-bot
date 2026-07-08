# Rev A Motor Driver Spec

Status: active baseline

Selected approach: two integrated single-motor H-bridge ICs, one driver per Romi motor.

This spec defines the motor-driver block behavior for Rev A. It does not choose the exact IC yet.

## Design Intent

Rev A should replace the low-level motor-drive role of the Pololu Romi 32U4 control board while staying easier to probe, understand, and modify.

The board should drive the two Romi brushed DC gearmotors, read encoders through the MCU, stream telemetry to a host, and stop safely if commands stop.

## Motor And Power Targets

Target motors:

- Pololu 120:1 high-power mini plastic gearmotor with extended motor shaft.
- Intended motor operating range: about 3 V to 6 V.
- Reference stall current: about 1.25 A at 4.5 V.

Motor supply:

- Rev A should accept the Romi chassis battery source through the board.
- Bench supply bring-up should also be possible.
- Motor supply should be treated as noisy and separate from logic power.
- Motors should not be powered from USB.

Driver rating target:

- Per-motor driver should tolerate at least the Romi motor stall current long enough for fault handling and test mistakes.
- Driver absolute maximum motor voltage must exceed the highest expected battery voltage with margin.
- If using the six-AA Romi bay, account for fresh alkaline voltage as well as NiMH voltage.

## Required Driver Features

Minimum:

- One H-bridge channel per motor.
- Forward and reverse drive.
- PWM speed control.
- MCU-controllable disable/sleep.
- Overcurrent and thermal protection inside the driver.
- Local VM and logic bypass capacitors.
- Bulk capacitance near the motor-driver supply.

Strongly preferred:

- Fault output readable by MCU.
- Current limiting or current sense output.
- Logic input compatible with the selected MCU voltage.
- Simple PH/EN or DIR/PWM control mode.
- Package that can be hand-inspected and reasonably assembled during prototype work.

Not required for Rev A:

- Regenerative braking optimization.
- Field-oriented control.
- Discrete MOSFET gate drive.
- High-current expansion beyond the Romi motors.

## Control Interface

Each motor-driver block should expose these logical signals to firmware:

- `MOTOR_L_DIR`, `MOTOR_R_DIR`: direction.
- `MOTOR_L_PWM`, `MOTOR_R_PWM`: speed command.
- `MOTOR_L_SLEEP` / `MOTOR_R_SLEEP`, or shared `MOTOR_SLEEP`: driver disable.
- `MOTOR_L_FAULT`, `MOTOR_R_FAULT` if supported by the selected IC.
- `MOTOR_L_CURRENT`, `MOTOR_R_CURRENT` if supported by the selected IC.

Bring-up firmware should support open-loop commands first:

- coast/off
- forward at duty cycle
- reverse at duty cycle
- stop on timeout

Closed-loop firmware should later use encoder counts to estimate wheel RPM and adjust PWM toward a target speed.

## Safe Stop Policy

Rev A must stop motor output when host commands stop arriving.

Initial timeout target:

- 1000 ms, matching the old implementation's ballpark timeout.

Default stop behavior:

- Set PWM to 0.
- Disable drive or coast unless the chosen driver and tests prove that braking is safer and thermally acceptable.
- Keep logic and telemetry alive so the host can see the timeout state.

Fault behavior:

- On driver fault, command PWM to 0 for the affected motor.
- Report fault state in telemetry.
- Require a deliberate host or firmware recovery path before re-enabling motor output.

## Electrical Design Notes

Each motor-driver section should include:

- Short, wide motor-current paths.
- Local ceramic bypass capacitor at the driver motor-supply pin.
- Local ceramic bypass capacitor at the driver logic-supply pin.
- Bulk capacitor on the motor rail near the drivers.
- Test points for motor supply, ground, PWM, direction, sleep/enable, fault, and both motor outputs.
- Clear separation between high-current motor paths and encoder/logic traces.

Remaining schematic/layout work:

- Add shared `+VSW` bulk capacitance near the motor-driver supply entry or driver cluster.
- Apply the DRV8838 layout guide: local VM/VCC bypass, compact motor-current loops, and exposed-pad GND.
- Assign footprints for motor-driver passives, motor/encoder connectors, bulk capacitance, and test points.
- Verify the left/right Romi encoder connector physical orientation against the actual encoder boards before layout/order.

Board-level motor-power section should include:

- Battery input from the Romi contacts.
- Reverse-polarity protection or a bring-up-safe equivalent.
- Motor power switch or enable path.
- Battery voltage sense to the MCU.
- Ground strategy that gives motor current a low-impedance return path without corrupting encoder signals.

## Motor Power Path Needed Work

From schematic review 2026-07-06 (`driver-board/power.kicad_sch`, `driver-board/motor_control.kicad_sch`). VM path currently: BATT (BT1+BT2, 6xAA) -> SW1 (SPST) -> `+VSW` -> DRV8838 VM. Only decoupling present.

Must add before layout:

- [ ] Bulk cap on `+VSW` entry. ~100uF electrolytic/polymer. Prevents VM collapse / DRV8838 UVLO on current spikes. Only 4x 0.1uF exist today, no bulk.
- [ ] ~10uF ceramic per DRV8838 VM pin, in addition to existing 0.1uF.
- [ ] Reverse-polarity protection in battery path (P-FET preferred, or Schottky). None present. Creates the missing `VBAT_PROTECTED` node between raw battery and switch.
- [ ] Fuse / resettable PTC in battery path (~2-3A hold). Protects against OUT short and bring-up wiring mistakes.
- [ ] Fix battery polarity. As wired, `+BATT` net ties to BT cathode (BT1 pin2) and GND to BT anode (BT2 pin1). Stack drawn reversed. Flip BT1/BT2 or nets are backwards on PCB.

Verify / decide:

- [ ] SW1 current rating >=3A (carries both motors + inrush). Mechanical-only rail cutoff; firmware kill relies on nSLEEP alone. OK Rev A; consider MCU load switch Rev B.
- [ ] DRV8838 stall headroom at max VMOTOR. 1.8A max vs ~1.67A stall at 6V. Confirm or pick higher-current driver with fault output.
- [ ] `+VSW` TVS not needed if bulk cap + body diodes suffice; confirm on bench.

Encoder power (schematic is correct as drawn, keep it): Romi Encoder Pair Kit (#3542) VCC = 3.5-18V, so `+3V3` is below the minimum — encoder VCC must be >=3.5V. Outputs A/B are open-drain; pulling them to `+3V3` (R3-R6, 10k) caps swing at 3.3V, STM32-safe. VCC=+5V / pull-up=+3V3 split is the right interface, no level shifter needed.

- [ ] Provide a clean >=3.5V rail for encoder VCC (5V typical). `+5V` referenced but generated nowhere yet (no MCU/regulator sheet).
- [ ] Do not feed encoder VCC from `+VSW`: dips below 3.5V under motor load (encoder brownout at stall) and dies when SW1 off.
- 10k pull-ups to `+3V3` OK for 12 CPR edge rates. Ref: https://www.pololu.com/product/3542

## Telemetry Requirements

Minimum motor-related telemetry:

- commanded left PWM
- commanded right PWM
- commanded direction per motor
- encoder count per side
- estimated wheel speed per side
- battery or motor-rail voltage
- command-timeout state

Preferred telemetry:

- driver fault state per side
- driver current sense per side
- motor-enable state
- brownout or undervoltage state if detectable

## Bring-Up Tests

Bench tests:

1. Power logic from USB with motor supply off; confirm motors cannot move.
2. Power motor rail from current-limited bench supply; command one motor at low duty.
3. Repeat for the other motor.
4. Verify forward/reverse direction mapping.
5. Sweep PWM duty cycle unloaded and log encoder speed.
6. Run both motors simultaneously and confirm encoder counts remain clean.
7. Trigger command timeout and confirm motor output stops.
8. Briefly load or stall a motor within safe limits and confirm current/fault behavior.
9. Check driver and board temperature after a short run.

Floor tests:

1. Low-speed straight command.
2. Low-speed in-place rotation.
3. Stop command.
4. Host disconnect or command stream interruption.
5. Save telemetry log for each run.

## Acceptance Criteria

Rev A motor-driver block is acceptable when:

- Both motors can be driven independently in both directions.
- PWM changes produce smooth, monotonic speed changes over the useful range.
- Encoders remain readable while motors run.
- Command timeout stops the motors.
- Driver faults or abnormal current are visible in telemetry if the selected IC supports them.
- The board survives normal bring-up mistakes with current-limited supply settings.

## References

- Electronics design spec: [`../ELECTRONICS_DESIGN_SPEC.md`](../ELECTRONICS_DESIGN_SPEC.md)
- Pololu Romi 32U4 Control Board User's Guide: https://www.pololu.com/docs/0J69/all
- Pololu Romi Encoder Pair Kit: https://www.pololu.com/product/3542
- TI DRV8838 datasheet: https://www.ti.com/lit/ds/symlink/drv8838.pdf
