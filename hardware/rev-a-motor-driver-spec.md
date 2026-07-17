# Rev A Motor Driver Spec

Status: active baseline

Selected approach: two integrated single-motor H-bridge ICs, one driver per Romi motor.

This spec defines the motor-driver block behavior for Rev A. The selected driver is the DRV8838, one per motor.

## Design Intent

Rev A should replace the low-level motor-drive role of the Pololu Romi 32U4 control board while staying easier to probe, understand, and modify.

The board should drive the two Romi brushed DC gearmotors, read encoders through the MCU, stream telemetry to a host, and stop safely if commands stop.

## Motor And Power Targets

Target motors:

- Pololu #1520 120:1 high-power mini plastic gearmotor with extended motor shaft.
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
- Battery chemistry is NiMH only (six-AA bay, ~7.2 V nominal, ~8.4 V fresh charge). Alkaline is excluded: ~9.6 V fresh pack erodes DRV8838 VM margin (11 V op / 12 V abs max) and pushes stall current to ~2.7 A vs the 1.8 A peak rating. See battery chemistry decision in [`rev-a-power-spec.md`](rev-a-power-spec.md).
- Rev A accepts Pololu's proven raw-battery/DRV8838 arrangement with a 75% normal PWM ceiling. The complete output and validation policy is recorded in [`../firmware/DESIGN.md`](../firmware/DESIGN.md).

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
- Keep logic and telemetry alive so the host can see the timeout state while whole-board power remains on.

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
- J3/J4 intentionally use a geometrically mirrored connector layout rather than exactly matching the Romi 32U4 top-side silkscreen. Left J4 is `GND, ENC_L_A, ENC_L_B, VBAT_SW, ML+, ML-` top-to-bottom; right J3 is `MR-, MR+, VBAT_SW, ENC_R_A, ENC_R_B, GND`. The right-side `MR-`, `MR+` order is deliberate because mirroring the left driver gives cleaner, more understandable routing. Do not change it solely to match Pololu. `VBAT_SW` intentionally replaces the reference board's `5V` encoder supply.

Board-level motor-power section should include:

- Battery input from the Romi contacts.
- Input fuse or resettable PTC near the battery entry.
- Whole-board power switch.
- Firmware motor-disable path through DRV8838 `SLEEP_N` and PWM/EN control.
- Battery voltage sense to the MCU.
- Ground strategy that gives motor current a low-impedance return path without corrupting encoder signals.

## Motor Power Path Needed Work

Updated Rev A decision: the main battery switch is whole-board power. Motor-driver `VM` can remain online whenever the board is powered; the MCU disables motor output through DRV8838 `SLEEP_N` and PWM/EN control. This is acceptable for Rev A if the `SLEEP_N` pulldowns keep the drivers asleep through reset and firmware initializes motor pins to a disabled state before accepting commands.

Must add before layout:

- [x] Bulk cap on switched battery/motor rail. Current schematic includes 220uF on `VBAT_SW`.
- [x] ~10uF ceramic per DRV8838 VM pin, in addition to 0.1uF.
- [x] Fuse / resettable PTC in battery path. Protects against PCB shorts, output faults, and bring-up wiring/probing mistakes. Selected F1: Littelfuse `MINISMDC260F/16-2`, JLCPCB `C16490`, `1812`, `16 V`, `2.6 A hold`, `5 A trip`.
- [ ] Verify battery-contact polarity against the actual Romi chassis footprint/mechanics during layout.
- [ ] Verify solderable battery-lug polarity against the actual Romi chassis contacts before routing copper.
- [x] Verify J3/J4 pin ordering and intentional left/right mirroring. The right motor pins deliberately do not match the Romi 32U4 silkscreen: J3 is `MR-, MR+, VBAT_SW, ENC_R_A, ENC_R_B, GND` top-to-bottom. Actual wheel-forward motor polarity and encoder count sign remain bring-up tests and may be corrected in firmware.

Verify / decide:

- [x] SW2 only controls Q2's gate and does not carry system load current. Q2 carries the motors, buck input, and inrush; SW2 needs the correct voltage, footprint, and mechanical action rather than a 3 A contact rating.
- [ ] Confirm DRV8838 `SLEEP_N` pulldowns are present, routed close enough to be reliable, and strong enough for reset/boot default-off behavior.
- [ ] Confirm firmware watchdog, command timeout, and startup GPIO sequencing are treated as part of the Rev A motor safety path.
- [x] Accept the DRV8838 with the six-cell NiMH pack and Pololu-style 75% normal PWM ceiling. Firmware policy and validation requirements are in [`../firmware/DESIGN.md`](../firmware/DESIGN.md).
- [x] Rev A omits a `+VSW` TVS or other clamp and accepts uncharacterized transient margin. Characterize braking, reversal, stall-release, and power-off peaks and reconsider the clamp/power-switch arrangement for Rev B.
- [ ] Reverse-polarity protection can be omitted for the Romi battery-contact path if the physical battery/contact layout prevents reversed input. Reconsider if adding external battery or bench connectors.

Encoder power decision: Romi Encoder Pair Kit (#3542) VCC = 3.5-18V, so `+3V3` is below the minimum. Rev A powers encoder VCC from the switched battery rail (`VBAT_SW` / `+VSW`) instead of adding a separate `+5V` encoder rail. Outputs A/B are open-drain; pulling them to `+3V3` caps the signal swing at 3.3V, STM32-safe. No level shifter is needed.

- [x] Feed encoder VCC from `VBAT_SW` / `+VSW` for Rev A.
- [x] Use the battery-level ADC reading as the encoder-power validity check. Firmware should treat encoder counts as unreliable, disable motor output, or warn when `VBAT_SW` approaches the encoder 3.5V minimum plus margin.
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
- Firmware motor policy: [`../firmware/DESIGN.md`](../firmware/DESIGN.md)
- Pololu #1520 motor: https://www.pololu.com/product/1520
- Pololu Romi 32U4 Control Board User's Guide: https://www.pololu.com/docs/0J69/all
- Pololu Romi 32U4 connector silkscreen reference: [`../docs/0J7509.1200.jpg`](../docs/0J7509.1200.jpg)
- Pololu Romi Encoder Pair Kit: https://www.pololu.com/product/3542
- TI DRV8838 datasheet: https://www.ti.com/lit/ds/symlink/drv8838.pdf
