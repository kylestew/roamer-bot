# Rev A Power Spec

Status: draft baseline

Selected approach: battery-powered whole-board rail; USB is telemetry-only.

This spec defines the Rev A power behavior needed to support motor control, encoder logging, telemetry, and safe stop.

## Design Intent

Rev A should be safe and understandable during bring-up:

- USB/debug does not power the board; battery or bench supply powers logic and motors.
- Battery or current-limited bench supply powers the switched board rail.
- Motor supply can be present whenever the board is powered.
- Motor output requires firmware permission; the MCU disables the drivers through PWM/enable control and DRV8838 `SLEEP_N`.
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
- Motor behavior should be tested conservatively because the Romi motors are small and motor voltage affects speed, current, and heat.

Battery chemistry decision (2026-07-10): **NiMH only. Do not run alkaline.**

Why:

- Fresh alkaline pack is ~9.6 V. DRV8838 operating max is 11 V, absolute max 12 V — only ~1.4 V headroom before motor switching spikes reach abs max. NiMH (~7.2 V, ~8.4 V hot off charger) keeps comfortable margin.
- Motor stall current scales with voltage: ~1.25 A at 4.5 V → ~2.7 A at 9.6 V, far past the DRV8838 1.8 A peak rating. OCP self-protects but chatters, and DRV8838 has no fault output so firmware cannot see it. At 7.2 V stall is ~2 A — still above rating but marginal instead of gross.
- Alkaline pack internal resistance (~1 Ω) causes deep sag under stall, risking encoder VCC dropping toward its 3.5 V minimum and buck UVLO (4.5 V) resets.
- Bench-supply bring-up should be capped at 8.5 V to match.

Bench-supply assumptions:

- Bring-up should start with current limiting enabled.
- The board should expose a convenient motor-rail input or test pads so motor drivers can be tested without relying only on installed batteries.

## Rails

Use these conceptual rails in the schematic:

- `VBAT_RAW`: direct battery/bench input.
- `VBAT_PROTECTED`: after input fuse/PTC and any optional reverse-polarity protection.
- `VBAT_SW`: switched whole-board battery rail after the main power switch.
- `VMOTOR`: motor-driver supply, normally the same net as `VBAT_SW` for Rev A.
- `VUSB`: USB bus voltage, used for USB protection and optional presence sensing only.
- `VLOGIC`: regulated MCU/logic rail derived from `VBAT_SW`.
- `VENC`: encoder board supply. Rev A ties this to `VBAT_SW` / `VMOTOR`, not to `+3V3` and not to a separate `+5V` regulator.

Rules:

- `VMOTOR` must not be sourced from USB.
- `VLOGIC` is sourced from the battery/bench-derived regulator, not USB.
- Encoder VCC is sourced from `VBAT_SW`; encoder A/B outputs remain pulled up to `VLOGIC` / `+3V3` for STM32-safe input levels.
- Battery-level telemetry is also the encoder-power sanity check. Firmware should set a low-voltage threshold above the encoder 3.5V minimum and avoid trusting encoder counts below that threshold.
- In Rev A, the main power switch is a whole-board switch. Turning it off removes both motor power and MCU telemetry.
- The MCU remains powered while firmware disables motor output with driver sleep/PWM, because that disable state does not remove `VBAT_SW`.
- Motor-driver logic supply should follow `VLOGIC`; motor-driver motor supply should follow `VMOTOR`.

## Required Functions

Minimum Rev A power functions:

- Input fuse or resettable PTC on the battery/bench input.
- Firmware-controlled motor disable through DRV8838 `SLEEP_N` and PWM/EN control.
- Logic regulation for the selected MCU voltage.
- USB telemetry behavior that does not source board power or energize motors.
- Battery or motor-rail voltage measurement through an MCU ADC.
- Power LED or visible indicator for logic power.
- Motor-power or motor-enable LED.
- Test points for `VBAT_RAW` or battery entry, `VBAT_SW`/`VMOTOR`, `VLOGIC`, `GND`, and ADC sense node.

Strongly preferred:

- Optional reverse-polarity protection if any bench/external input can be connected backwards.
- Bulk capacitor near motor-driver supply.
- Clear jumper or switch option for early bring-up.
- Power-good or undervoltage signal if the selected regulator provides one.

Not required for Rev A:

- Raspberry Pi power output.
- Full USB/battery power multiplexer.
- Latching pushbutton power circuit.
- Separate hardware motor-rail load switch.
- Software-controlled whole-board shutdown.
- Charging circuit.
- Fuel gauge IC.

## Logic Power Policy

Rev A should support USB telemetry while the board is powered from battery or bench supply.

Preferred behavior:

- USB connected, no battery/bench supply: MCU remains off and telemetry is unavailable.
- Battery/bench connected and main switch on, USB disconnected: board can run standalone from the switched battery rail.
- USB and battery/bench connected: board runs from battery/bench power, and USB is used only as the host telemetry link.

Rev A decision:

- Do not connect `VUSB` to the logic regulator input.
- Do not diode-OR or mux `VUSB` into `VLOGIC`.
- Keep `VUSB` local to the USB connector, ESD protection, required VBUS capacitance, and optional MCU VBUS-sense divider.
- The board must be powered from battery or bench supply before USB telemetry is expected to work.
- The main battery switch is whole-board power, not a dedicated motor-disable switch.

The schematic must explicitly prevent unsafe backfeeding between USB, battery-derived logic, and any external host.

## Motor Power Policy

For Rev A, `VMOTOR` is normally the switched battery rail (`VBAT_SW`). It can remain online while firmware disables motor output.

Motor output should be disabled when:

- command timeout occurs
- firmware detects a driver fault that requires shutdown
- host requests motor disable
- battery voltage is below a defined safe threshold
- bring-up firmware has not explicitly enabled the driver

Rev A decision:

- Timeout and host-requested motor disable do not remove `VMOTOR`; they command the DRV8838 drivers to sleep/coast using `SLEEP_N` and PWM/EN.
- The DRV8838 `SLEEP_N` nets must have default pulldowns so motors stay disabled through MCU reset, boot, debugger attach, and firmware startup.
- Firmware must initialize motor GPIOs to the disabled state before accepting commands.
- The watchdog and command timeout are part of the motor safety path.

Rev A baseline:

- Use driver sleep/PWM disable for fast firmware-level timeout.
- Do not require a separate high-side motor-power switch for Rev A.
- Keep telemetry alive after firmware disables motors, as long as whole-board power remains on.

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

- Input resettable PTC close to the battery positive entry. Rev A selected part: Littelfuse `MINISMDC260F/16-2`, JLCPCB `C16490`, `1812`, `16 V`, `2.6 A hold`, `5 A trip`.
- Reverse-polarity protection is optional for the Romi battery-contact path if the mechanical layout prevents reversed battery connection; keep it under consideration for bench/external inputs.
- Current-limited bring-up path through the bench supply during early tests.
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
4. PTC/fuse sanity check with a protected/current-limited setup.
5. Firmware motor-disable check with no motors connected: verify `SLEEP_N` default low and PWM/EN inactive through reset.
6. Firmware motor-disable check with motors connected.
7. ADC voltage reading check against a multimeter.
8. Low-voltage threshold check by reducing bench supply.
9. Both motors running: confirm logic rail remains stable.
10. Command timeout: confirm motor output disables and telemetry remains alive while the main power switch remains on.

Floor tests:

1. Low-speed straight drive while logging `VMOTOR`.
2. In-place turn while logging `VMOTOR`.
3. Stop/start sequence while watching for MCU resets.
4. Host disconnect while motors are moving.
5. Weak-battery simulation or low bench voltage test.

## Acceptance Criteria

Rev A power architecture is acceptable when:

- USB alone cannot power the MCU or move motors.
- Main switch powers the whole board intentionally.
- Motor output can be enabled and disabled by firmware while `VMOTOR` remains online.
- MCU remains alive during firmware motor enable/disable transitions.
- Motor/battery voltage is measured and logged.
- Both motors can run without corrupting encoder readings or resetting the MCU.
- Command timeout disables motor output while preserving telemetry.
- Bring-up can be done from a current-limited bench supply.

## References

- Power architecture design thinking: [`power-architecture-design-thinking.md`](power-architecture-design-thinking.md)
- Romi 32U4 power reference: [`romi-32u4-power-reference.md`](romi-32u4-power-reference.md)
- Electronics design spec: [`../ELECTRONICS_DESIGN_SPEC.md`](../ELECTRONICS_DESIGN_SPEC.md)
