# Power Architecture Design Thinking

Status: draft

Purpose: compare Rev A power-system approaches before choosing regulators, switches, connectors, and protection parts.

## Power Basics

The power system is the part of the robot that decides where energy comes from, where it goes, and what happens when something goes wrong.

For this rover, there are two very different electrical worlds:

- Motor power: high current, noisy, switched hard by the motor drivers, and full of voltage dips and spikes.
- Logic power: lower current, sensitive to resets, used by the MCU, encoders, USB/debug circuitry, and telemetry.

The main power-design mistake to avoid is pretending these are the same thing. Motors can pull large current when starting, changing direction, or stalling. If that current drop reaches the MCU rail, the board can reset exactly when the robot most needs control.

The second mistake to avoid is letting USB power the motors. USB is useful for programming and telemetry, but it should not unexpectedly make the wheels move.

## The Rails We Need

Rev A should think in named rails:

- `VBAT_RAW`: direct battery input from the Romi contacts or bench supply.
- `VBAT_PROTECTED`: battery after reverse-polarity protection.
- `VMOTOR`: switched/protected motor rail feeding the H-bridges.
- `VLOGIC`: regulated logic rail for the MCU and motor-driver logic inputs.
- `VUSB`: USB bus power from the host computer.
- `VENC`: encoder supply. Rev A uses the switched battery rail for encoder VCC because the Romi encoder boards accept 3.5-18 V, while their open-drain A/B outputs are pulled up to 3.3 V for the MCU.

The exact names can change in KiCad, but the design should keep the concepts separate.

## Approach 1: Romi-Like Full Power System

This approach copies the shape of the Pololu Romi 32U4 board power architecture:

- Battery contacts feed a reverse-protected battery path.
- A latching pushbutton or slide-switch circuit creates a switched battery rail.
- The switched battery rail feeds the motors and a 5 V regulator.
- USB can power the logic without powering motors.
- A power multiplexer chooses USB or regulated 5 V for logic.
- A 3.3 V regulator feeds lower-voltage devices.
- Optional Raspberry Pi power is supported.

Why it is attractive:

- Proven pattern on the board we are replacing.
- Excellent user behavior: USB programming works even when motor power is off.
- Handles battery and USB being plugged in at the same time.
- Can support an onboard computer later.

Tradeoffs:

- More parts and more failure modes.
- More schematic complexity before we have even proven the motor-control loop.
- Power muxing and latching behavior can hide bugs during bring-up.
- Raspberry Pi power support is explicitly not a Rev A goal.

Good fit when:

- The board is becoming a polished robot controller.
- Onboard computer power is required.
- User-facing power-button behavior matters.

Poor fit for Rev A because it risks making power management a project by itself.

## Approach 2: Minimal Split Battery/USB Power

This approach keeps the important safety behavior but removes the advanced power features.

Core idea:

- USB can power logic/debug.
- Battery or bench supply powers motors.
- Motors cannot run unless motor power is present and explicitly enabled.
- Logic and telemetry stay alive while motor output is disabled.

Typical block diagram:

```text
USB 5 V
  -> protected/debug logic input
  -> VLOGIC regulator or selector

Romi battery / bench input
  -> reverse-polarity protection
  -> motor enable switch or load switch
  -> VMOTOR
  -> motor drivers

VBAT/VMOTOR sense
  -> resistor divider
  -> MCU ADC
```

Why it fits Rev A:

- Preserves the most important lesson from the Romi 32U4 board: USB logic power is separate from motor power.
- Avoids powering motors from the host computer.
- Supports current-limited bench-supply bring-up.
- Keeps the schematic understandable.
- Leaves space to add a better power switch or mux in Rev B.

Tradeoffs:

- Less polished than the Romi 32U4 board.
- The operator may need to manage USB and battery/bench power separately.
- Exact behavior when both USB and battery are present must be designed carefully.
- If no power mux is used, backfeed protection must still be considered.

Good fit when:

- The first milestone is motor control, encoder logging, telemetry, and safe stop.
- We want to debug firmware over USB with motors disabled.
- We are not powering a Raspberry Pi from Rev A.

## Approach 3: External Power Modules And Bench-First Board

This approach keeps Rev A's PCB power section minimal by relying on external modules:

- Bench supply or battery feeds motor-driver board directly.
- External regulator module feeds logic.
- USB may be data-only or logic-only.
- Protection is partly handled by lab setup.

Why it is attractive:

- Fastest schematic.
- Fewer board-level power decisions.
- Good for a one-off motor-driver experiment.

Tradeoffs:

- Not a real Romi replacement board.
- Easy to wire incorrectly.
- Harder to reproduce test results.
- Telemetry might not know what the power system is doing.
- Safety depends too much on the bench setup.

Good fit when:

- We decide to build a throwaway motor-driver experiment before Rev A.
- The goal is only to learn current, heat, PWM behavior, and noise.

Poor fit for the integrated Rev A board because Rev A needs battery-aware robot behavior from day one.

## Recommendation

Use a simplified battery-powered Rev A architecture.

The Rev A power system should be simple enough to understand from the schematic, but complete enough to run the robot safely:

- USB is telemetry/debug data only and does not power the board.
- Battery or bench supply powers a switched whole-board rail.
- The switched whole-board rail feeds the motor-driver `VM` pins and the 3.3 V logic regulator.
- Motor supply can be online whenever the board is powered.
- The MCU owns motor disable by holding DRV8838 `SLEEP_N` low and PWM/EN inactive until commands are valid.
- DRV8838 `SLEEP_N` must have hardware pulldowns so reset and boot default to motors disabled.
- The battery path has a fuse or resettable PTC.
- Reverse-polarity protection is optional for the fixed Romi battery-contact path, but should be reconsidered for any external bench/battery connector.
- The MCU can measure motor/battery voltage.
- Logic remains alive during firmware motor disable and command timeout because those states do not remove board power.
- Test points exist for every rail.

Do not copy the full Romi 32U4 latching power switch, Raspberry Pi power path, or USB/battery power mux for Rev A unless a later requirement forces it.

## What Power Teaches

Power design is where the robot becomes real:

- A motor command becomes current.
- Current causes voltage drops.
- Voltage drops can reset logic.
- Switching current creates noise.
- Noise can corrupt encoder readings.
- A weak battery changes control behavior.
- A stalled motor can become a thermal problem.

Good telemetry should expose those effects instead of hiding them. That means Rev A should measure at least the motor/battery rail and should keep enough state to say "motors are disabled", "battery is low", or "command timed out".

## Open Decisions

- Exact battery chemistry for Rev A testing.
- `VLOGIC` is expected to be 3.3 V for STM32; decide whether any separate 5 V rail is needed.
- F1 resettable PTC selected: Littelfuse `MINISMDC260F/16-2`, JLCPCB `C16490`, `1812`, `16 V`, `2.6 A hold`, `5 A trip`.
- Whether to add optional reverse-polarity protection or a bypass footprint for non-Romi power inputs.
- Whether to add a Rev B hardware motor-power switch/load switch after Rev A testing.
- Whether Rev A includes current limiting beyond the motor-driver ICs.
- Whether USB VBUS should be sensed by the MCU, while still not powering logic.

## References

- Pololu Romi 32U4 Control Board User's Guide: https://www.pololu.com/docs/0J69/all
- Pololu Romi 32U4 resources: https://www.pololu.com/product/3544/resources
- TI DRV8838 datasheet: https://www.ti.com/lit/ds/symlink/drv8838.pdf
- Local reference: [`../docs/romi_32u4_control_board.pdf`](../docs/romi_32u4_control_board.pdf)
- Local reference: [`../docs/romi-32u4-control-board-pinout-power.pdf`](../docs/romi-32u4-control-board-pinout-power.pdf)
- Local reference: [`../docs/romi-32u4-control-board-schematic-diagram.pdf`](../docs/romi-32u4-control-board-schematic-diagram.pdf)
