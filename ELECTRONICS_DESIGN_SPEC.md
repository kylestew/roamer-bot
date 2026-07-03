# Studio Mission Rover Electronics Design Spec

Status: seed

## Purpose

Design the electronics for the next `roamer-bot` revision from scratch, including the motor-control electronics.

The goal is to understand and own the robot's electrical/control stack end to end, not to buy a finished robot controller.

## Agreed Direction

- Build a two-wheel differential-drive rover.
- Build the electronics ourselves.
- Include the motor-driver design as part of the learning.
- It is acceptable to use motor-driver ICs rather than fully discrete MOSFET H-bridges.
- Keep ROS out of the core system for now.
- First milestone is basic motion, encoder logging, and safe stop.
- Chassis priority is fast learning and iteration, not a finished visual/mechanical object.
- Rover scale is small desktop/floor scale.
- Payload is only batteries, compute/brains, and sensors.
- Drive motors are simple brushed DC gear motors with encoders.
- Motion goal is slow, precise indoor movement rather than speed.
- Design around battery power from day one.
- Battery chemistry/source is open for now.
- v0 control is tethered to a laptop/host over USB or serial.
- Motor driver and basic control board should be one self-contained assembly.
- Onboard computer is deferred until the motor/control assembly works.
- The motor/basic-control board will be a fully custom MCU + PCB design, not a dev-board carrier.
- MCU family is undecided and will be selected later.
- PCB should mount directly to the Romi chassis, similar to the Pololu Romi controller board.
- PCB should mate with the Romi encoder boards directly.
- PCB outline does not need to clone the Pololu Romi controller board exactly, as long as required mounting, encoder mating, clearance, and access constraints are preserved.

## First Milestone

Prove this:

> Two motors can be commanded from a laptop or simple host, encoder counts can be logged, and motor output stops safely when commands stop.

Minimum demo:

1. Command left and right motors independently.
2. Read left and right encoder counts.
3. Stream basic telemetry.
4. Stop motors on command timeout.
5. Save a short log from a bench or floor test.

## Initial System Blocks

- Battery or bench supply.
- Power protection / switching.
- Motor-driver circuit.
- MCU.
- Encoder inputs.
- Serial or USB command/telemetry link.
- Status / fault indication.
- Test points for bring-up.

## PCB Revision Strategy

Target two real PCB revisions, not three. If the motor-driver choice feels risky before committing to the first rover board, use an eval board, breakout, breadboardable module, or tiny throwaway experiment to learn motor current, heat, PWM behavior, noise, and protection needs. Do not count that as a project PCB revision unless it becomes part of the rover.

### Rev A: Minimal Romi Replacement Board

Purpose:

- First serious custom rover PCB.
- Replace the purchasable Pololu Romi control board for the minimum useful rover loop.
- Prove motor control, encoder reading, telemetry, and safe stop before higher-level behavior.

Contents:

- MCU.
- USB/programming/debug.
- Two motor-driver channels.
- Direct Romi encoder-board mating.
- Battery input from Romi contacts.
- Basic logic regulation.
- Battery voltage sense.
- Motor enable / safe stop path.
- Host link over USB or serial.
- Power / heartbeat / motor-enable / fault LEDs.
- Generous test points.
- Probe-friendly layout.

Firmware success criteria:

- Laptop can command left and right motors independently.
- Encoders are read reliably.
- Board streams basic telemetry.
- Command timeout stops the motors.
- A short bench or floor run log can be saved on the host.

Defer:

- IMU.
- Raspberry Pi connector.
- Pi power.
- Level shifting for Pi.
- ROS concerns.
- LCD.
- Buzzer.
- Extra user buttons beyond reset/boot if not needed.
- Large sensor expansion.
- Elegant latching power circuit.
- Full Romi 32U4 feature parity.

### Rev B: Integrated Rover Controller

Purpose:

- Cleaner, more capable board after Rev A proves the motor/control stack.
- Fold in lessons from Rev A bring-up and make the assembly more reliable.
- Prepare for higher-level autonomy experiments.

Possible additions:

- Fixes from Rev A bring-up.
- Better power architecture and protection.
- Cleaner motor-driver layout if needed.
- Better connector placement and cable routing.
- More robust battery, voltage, current, and fault telemetry.
- IMU if there is a concrete reason.
- Optional onboard-computer interface.
- Level shifting if an onboard computer requires it.
- Sensor/debug expansion.
- Better mounting and clearance.
- Improved status/fault indication.

Guiding rule:

> Rev A should be easy to probe and hard to destroy, not feature-complete. Rev B should make the proven system cleaner, more robust, and easier to build on.

## Romi Board Design Features To Preserve Or Consider

Primary references:

- Romi 32U4 Control Board User's Guide: https://www.pololu.com/docs/0J69/all
- Romi Chassis Kit - White: https://www.pololu.com/product/3509
- Romi Encoder Pair Kit: https://www.pololu.com/product/3542
- Romi 32U4 Control Board resources: https://www.pololu.com/product/3544/resources
- Pololu Raspberry Pi Romi build: https://www.pololu.com/blog/663/building-a-raspberry-pi-robot-with-the-romi-chassis

The custom board does not need to clone the Romi 32U4 board outline, but these functional features matter:

- Mounting holes: preserve the relevant Romi chassis mounting locations so the board bolts directly to the chassis.
- Encoder mating: preserve the location/orientation of the low-profile encoder headers if the encoder pins point down toward the chassis.
- Battery contact interface: account for the built-in 6xAA bay and battery contacts.
- Power access: include a clear way to switch motor/battery power and monitor battery voltage.
- USB/debug access: place USB/programming/debug connectors where they remain accessible after the board is mounted.
- Status visibility: keep power, heartbeat, motor-enable, and fault indication visible from above.
- Test access: leave room for probe points around motor outputs, encoder lines, battery rail, logic rail, and serial/debug.
- Mechanical clearance: avoid chassis ribs, motor clips, wheels, encoder boards, battery contacts, and the ball caster area.
- Future expansion: consider leaving mounting holes or header space for a later onboard computer, but do not optimize Rev A around Raspberry Pi mounting.

Pololu design features worth learning from but not necessarily copying:

- Dual H-bridge motor drivers.
- Direct encoder-board headers.
- Battery reverse protection and switched battery rail.
- Battery voltage divider.
- USB-powered logic while motor power is off.
- Visible user LEDs/buttons.
- IMU on board.
- Raspberry Pi HAT-style connector and mounting holes.
- Level shifting between MCU and Raspberry Pi.
- Expansion areas with power rails and GPIO.

Assembly lessons from the Pololu Raspberry Pi Romi build:

- Test the control board before mounting it to the chassis.
- Encoder header location can affect future Raspberry Pi / HDMI / cable clearance.
- There are inner and outer encoder-header positions on the Pololu board; the inner position was recommended in their Pi build for better HDMI clearance.
- Encoder boards need to sit flat against the motors and align with the motor top surface.
- To guarantee encoder-header alignment, insert male headers through the encoder boards into the control-board female headers while pushing the motors into their brackets, then solder the headers in place.
- Do not install encoder magnetic discs until soldering is done, to avoid heat damage.
- USB should be able to power logic/debug, but not the motors; motor power should require battery power and an enabled motor rail.
- A visible heartbeat/status indicator is useful once the host computer or later onboard computer is involved.

## Prior Project Reference

Useful lessons from the existing `roamer-bot` code:

- The old Romi project already had the right split: low-level board owns motor control, encoders, battery, buttons/LEDs, and command timeout; host computer owns higher-level behavior.
- The old firmware used a command timeout to stop motion if new commands stopped arriving. Keep that as a non-negotiable v0 safety behavior.
- Kinematics, odometry, and PID were separated into small libraries with desktop tests. Reuse that pattern.
- The old stack used ROS and I2C structs; the new version should start simpler with a custom tethered host protocol, then bridge later only if useful.
- Battery/brownout behavior was a real issue on the Romi/Raspberry Pi stack. Keep motor power and logic/host power concerns visible from the start.

Ballpark references from the old build, not commitments:

- Wheel separation: about 146 mm.
- Wheel circumference: about 215 mm.
- Encoder ticks per wheel revolution: 1440.
- Motor control sample period: 20 ms / 50 Hz.
- Command timeout: 1000 ms.

## Not Decided Yet

- MCU family.
- Motor-driver IC versus discrete H-bridge.
- Exact power architecture.
- Board outline.
- Connector choices.
- Telemetry format.
- Whether motor-driver de-risking needs eval hardware before Rev A.
