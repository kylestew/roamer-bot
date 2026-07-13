# Rev A Firmware Design

Status: active decisions

This document records firmware behavior that is required by the Rev A hardware. Hardware-level requirements remain in [`../hardware/rev-a-stm32-mcu-requirements.md`](../hardware/rev-a-stm32-mcu-requirements.md).

## Motor and battery baseline

Rev A intentionally follows the proven Pololu Romi motor-power approach:

| Item | Rev A decision |
|---|---|
| Motor | [Pololu #1520](https://www.pololu.com/product/1520), 120:1 mini plastic gearmotor HP |
| Motor specification | 4.5 V intended, 3–6 V comfortable range, 1.25 A stall at 4.5 V |
| Battery | Six rechargeable AA NiMH cells only |
| Pack voltage | About 7.2 V nominal and approximately 8.4 V when freshly charged |
| Motor driver | One DRV8838 per motor |
| Driver supply | Raw reverse-protected and switched battery rail, `VBAT_SW` |
| Normal PWM ceiling | 75% absolute duty cycle |

Alkaline cells are outside the Rev A operating specification. Six alkaline cells have a substantially higher pack voltage than six NiMH cells and must not be treated as an interchangeable option.

Pololu's Romi 32U4 board uses the same motors, six-cell battery arrangement, raw switched battery motor supply, and DRV8838 drivers. Its normal motor API limits output to 300 of 400 PWM counts, or 75% duty. Pololu describes the full 400-count range as turbo mode and warns that it can reduce motor lifetime.

## Motor output policy

- Clamp every open-loop, closed-loop, and host-requested motor command to an absolute duty cycle of `0.75` in normal operation.
- Apply the clamp at the final driver-output boundary as well as at command parsing. A malformed host command or controller wind-up must not bypass it.
- Do not expose turbo/full-duty operation in the normal Rev A host protocol.
- Full-duty testing, if ever needed, is a deliberate diagnostic operation performed locally with a current-limited bench supply and explicit test firmware.
- Battery-voltage feed-forward may reduce duty or improve repeatability below the ceiling. It must not silently raise the 75% ceiling.
- Record both requested and applied duty in telemetry so saturation is visible.

The 75% limit gives a rough effective motor voltage of 5.4 V from a 7.2 V pack and 6.3 V from an 8.4 V fresh pack. PWM is not identical to powering a motor from a lower DC voltage, so the limit does not guarantee current, thermal, or transient margin. Rev A accepts motor-rail transient characterization as deferred Rev B work.

## Driver state and safe stop

The DRV8838 PH/EN interface has an important semantic detail: `EN/PWM = 0` while awake commands brake, not coast. Pulling `nSLEEP` low disables the bridge and allows the motor to coast.

Required behavior:

1. Keep both drivers asleep during reset and early boot.
2. Initialize PWM to zero and direction to a known value before raising either `nSLEEP` signal.
3. Accept motor commands only after initialization and command-channel readiness.
4. On command timeout, malformed command, firmware fault, or explicit disable: set PWM to zero, then pull both `nSLEEP` signals low.
5. Use 1000 ms as the initial command-timeout value; tune it later from command rate and measured stopping distance.
6. Require a new valid command sequence before re-enabling after a timeout or fault.
7. Never create torque during reset, boot, USB enumeration, host disconnect, or brownout recovery.

## Battery and drivetrain telemetry

`BATLEV` is intentionally a coarse battery estimate, not a precision measurement. Use an ADC sample time of at least 41.5 cycles, preferably 55.5 cycles, to accommodate the 100 kΩ/33 kΩ divider's source impedance. Apply modest averaging and the nominal divider ratio. No dedicated filter capacitor or production calibration is required, and firmware must not depend on a narrow BATLEV threshold for safety-critical behavior.

Minimum telemetry:

- measured battery voltage;
- requested and applied duty for each motor;
- active 75% duty ceiling and whether either command is saturated;
- direction and sleep state for each motor;
- encoder count and estimated wheel speed for each side;
- command age and timeout state;
- reset and brownout cause when available; and
- an inferred stall warning when substantial applied duty produces no encoder motion.

The DRV8838 provides neither a fault output nor current sensing, so firmware cannot directly identify over-current or thermal shutdown. Telemetry must not claim that an inferred stall is a measured driver fault.

## Encoder handling

- Verify the physical left/right direction convention during bring-up.
- Positive commands should produce forward wheel motion and positive encoder accumulation on both sides after the documented sign corrections.
- Handle hardware counter wrap with modular deltas and accumulate position in a wider software integer.
- Treat encoder data as unreliable if the switched battery rail falls close to the encoder board's minimum supply voltage.

## Board indicators

- D2, the PC13 status LED, is active-low.
- Drive PC13 low to turn D2 on and high to turn it off.
- Keep PC13 configured so it sinks LED current; do not reconfigure the circuit or firmware around sourcing current from PC13.

## Required validation

Before normal unrestricted use:

1. Bring up each motor independently from a current-limited bench supply at low duty.
2. Verify forward/reverse direction and encoder sign for both sides.
3. Sweep duty through the permitted range and confirm monotonic unloaded speed.
4. Repeat with fresh and depleted six-cell NiMH packs.
5. Run both motors together and confirm clean encoder readings and stable logic power.
6. Briefly test single- and dual-motor stalls within controlled limits while observing supply current and driver temperature.
7. Measure driver and motor temperature during representative driving.
8. Confirm the timeout and every reset/disconnect path puts both drivers to sleep.

Motor-rail transient characterization during braking, reversal, stall release, and power switch-off is planned for Rev B rather than being a Rev A acceptance criterion.

## References

- [Pololu #1520 motor](https://www.pololu.com/product/1520)
- [Pololu Romi motor drivers and encoders](https://www.pololu.com/docs/0J69/3.3)
- [Pololu Romi power architecture](https://www.pololu.com/docs/0J69/3.5)
- [Pololu Romi motor library](https://pololu.github.io/romi-32u4-arduino-library/class_romi32_u4_motors.html)
- [Pololu support explanation of the 75% limit](https://forum.pololu.com/t/isnt-the-9v-of-the-romi-batteries-too-high-for-the-4-5v-gearmotors/17486)
- [TI DRV8838 datasheet](https://www.ti.com/lit/ds/symlink/drv8838.pdf)
