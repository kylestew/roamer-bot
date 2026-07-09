# Rev A Power Spec

Status: draft baseline

Selected approach: battery-powered logic and motor rails; USB is telemetry-only.

This spec defines the Rev A power behavior needed to support motor control, encoder logging, telemetry, and safe stop.

## Design Intent

Rev A should be safe and understandable during bring-up:

- USB/debug does not power the board; battery or bench supply powers logic and motors.
- Battery or current-limited bench supply powers the motor rail.
- Motor output requires both firmware permission and available motor power.
- Telemetry reports enough power state to explain resets, weak batteries, and disabled motors.

The goal is not a polished consumer-style power system. The goal is a reliable learning board that makes power behavior visible.

## Power Inputs

Required:

- Romi battery input through the chassis battery contacts.
- USB input for serial telemetry only.
- Bench-supply access for controlled bring-up of the motor rail.

Battery assumptions:

- Romi battery bay holds six AA cells.
- NiMH pack voltage is about 7.2 V nominal.
- Fresh alkaline pack voltage can be higher.
- Motor behavior should be tested conservatively because the Romi motors are small and motor voltage affects speed, current, and heat.

Bench-supply assumptions:

- Bring-up should start with current limiting enabled.
- The board should expose a convenient motor-rail input or test pads so motor drivers can be tested without relying only on installed batteries.

## Rails

Use these conceptual rails in the schematic:

- `VBAT_RAW`: direct battery/bench input.
- `VBAT_PROTECTED`: after reverse-polarity protection.
- `VMOTOR`: switched/enabled motor-driver supply.
- `VUSB`: USB bus voltage, used for USB protection and optional presence sensing only.
- `VLOGIC`: regulated MCU/logic rail.
- `VENC`: encoder board supply, normally derived from `VLOGIC`.

Rules:

- `VMOTOR` must not be sourced from USB.
- `VLOGIC` is sourced from the battery/bench-derived regulator, not USB.
- The MCU should remain powered when motor output is disabled, as long as valid battery/bench-derived logic power is present.
- Motor-driver logic supply should follow `VLOGIC`; motor-driver motor supply should follow `VMOTOR`.

## Required Functions

Minimum Rev A power functions:

- Reverse-polarity protection on battery/bench motor input.
- Motor-rail enable/disable mechanism.
- Logic regulation for the selected MCU voltage.
- USB telemetry behavior that does not source board power or energize motors.
- Battery or motor-rail voltage measurement through an MCU ADC.
- Power LED or visible indicator for logic power.
- Motor-power or motor-enable LED.
- Test points for `VBAT_RAW`, `VBAT_PROTECTED`, `VMOTOR`, `VLOGIC`, `GND`, and ADC sense node.

Strongly preferred:

- Input fuse or resettable fuse for battery/bench motor input.
- Bulk capacitor near motor-driver supply.
- Clear jumper or switch option for early bring-up.
- MCU-readable motor-power-present signal.
- Power-good or undervoltage signal if the selected regulator provides one.

Not required for Rev A:

- Raspberry Pi power output.
- Full USB/battery power multiplexer.
- Latching pushbutton power circuit.
- Software-controlled whole-board shutdown.
- Charging circuit.
- Fuel gauge IC.

## Logic Power Policy

Rev A should support USB telemetry while the board is powered from battery or bench supply.

Preferred behavior:

- USB connected, no battery/bench supply: MCU remains off and telemetry is unavailable.
- Battery/bench connected, USB disconnected: board can run standalone if the logic regulator is powered from the protected battery path.
- USB and battery/bench connected: board runs from battery/bench power, and USB is used only as the host telemetry link.

Rev A decision:

- Do not connect `VUSB` to the logic regulator input.
- Do not diode-OR or mux `VUSB` into `VLOGIC`.
- Keep `VUSB` local to the USB connector, ESD protection, required VBUS capacitance, and optional MCU VBUS-sense divider.
- The board must be powered from battery or bench supply before USB telemetry is expected to work.

The schematic must explicitly prevent unsafe backfeeding between USB, battery-derived logic, and any external host.

## Motor Power Policy

`VMOTOR` should feed only motor-driver motor supply pins and related motor-power measurement/protection components.

Motor output should be disabled when:

- command timeout occurs
- firmware detects a driver fault that requires shutdown
- host requests motor disable
- battery voltage is below a defined safe threshold
- bring-up switch/jumper is off

Open design question:

- Whether timeout disables the whole `VMOTOR` rail or only commands the drivers to coast/sleep.

Rev A baseline:

- Use driver sleep/PWM disable for fast firmware-level timeout.
- Include a hardware motor-power enable path if practical.
- Keep telemetry alive after motor disable.

## Voltage Measurement

Rev A should measure the battery or motor rail with a resistor divider into an MCU ADC.

Requirements:

- Divider output must stay below ADC maximum at the highest expected input voltage.
- Divider impedance should balance battery drain, ADC accuracy, and noise sensitivity.
- ADC input should include a small capacitor or filtering strategy if motor noise corrupts readings.
- Firmware should convert ADC counts to millivolts and log the value.

Initial telemetry fields:

- `vbat_mv` or `vmotor_mv`
- `vlogic_mv` if practical
- `motor_power_present`
- `motor_enable_state`
- `low_battery_warning`

## Protection And Layout

Protection:

- Reverse-polarity protection before `VMOTOR`.
- Fuse or current-limited bring-up path for motor supply.
- Driver overcurrent/thermal protection through selected motor-driver ICs.
- Optional TVS or clamp only if testing shows motor spikes require it.

Capacitance:

- Bulk capacitor near the motor-driver supply entry.
- Local ceramic bypass at each motor-driver `VM` pin.
- Local ceramic bypass at each motor-driver logic supply pin.
- Local bypass at MCU and regulator pins following datasheet layout guidance.

Grounding:

- Motor current should have short, wide return paths.
- Encoder and MCU signals should not share long high-current return paths.
- Place motor-driver bulk capacitance so motor current loops are compact.
- Keep ADC sense routing away from motor outputs and PWM traces where possible.

## Bring-Up Tests

Bench tests:

1. USB only: confirm MCU, logic rail, and motor rail remain off.
2. Battery/bench only: confirm logic rail behavior matches selected source policy.
3. USB plus current-limited bench supply: confirm no backfeed or unexpected rail voltage.
4. Reverse-polarity protection sanity check with a protected/current-limited setup.
5. Motor rail enable on/off check with no motors connected.
6. Motor rail enable on/off check with motors connected.
7. ADC voltage reading check against a multimeter.
8. Low-voltage threshold check by reducing bench supply.
9. Both motors running: confirm logic rail remains stable.
10. Command timeout: confirm motor output disables and telemetry remains alive.

Floor tests:

1. Low-speed straight drive while logging `VMOTOR`.
2. In-place turn while logging `VMOTOR`.
3. Stop/start sequence while watching for MCU resets.
4. Host disconnect while motors are moving.
5. Weak-battery simulation or low bench voltage test.

## Acceptance Criteria

Rev A power architecture is acceptable when:

- USB alone cannot power the MCU or move motors.
- Motor rail can be powered and disabled intentionally.
- MCU remains alive during motor enable/disable transitions.
- Motor/battery voltage is measured and logged.
- Both motors can run without corrupting encoder readings or resetting the MCU.
- Command timeout disables motor output while preserving telemetry.
- Bring-up can be done from a current-limited bench supply.

## References

- Power architecture design thinking: [`power-architecture-design-thinking.md`](power-architecture-design-thinking.md)
- Romi 32U4 power reference: [`romi-32u4-power-reference.md`](romi-32u4-power-reference.md)
- Electronics design spec: [`../ELECTRONICS_DESIGN_SPEC.md`](../ELECTRONICS_DESIGN_SPEC.md)
