# Romi 32U4 Power Reference

Status: learning reference

Purpose: explain the power architecture of the Pololu Romi 32U4 control board so Rev A can borrow the useful ideas without inheriting all of its complexity.

## What The Romi 32U4 Board Does

The Romi 32U4 control board is more than a motor board. It powers:

- ATmega32U4 MCU.
- Motor-driver logic.
- DRV8838 motor-driver motor supplies.
- Encoders.
- IMU and other onboard features.
- Optional Raspberry Pi through a 5 V rail.
- USB programming/debug interface.

That is why its power architecture is relatively rich. Rev A does not need all of it, but it should learn from the separation between motor power, logic power, and USB power.

## Named Power Nodes

Pololu documents several useful power nodes:

- `VBAT`: direct positive battery voltage from the Romi battery contacts.
- `VRP`: battery voltage after reverse-voltage protection.
- `VSW`: battery voltage after reverse protection and the power switch circuit.
- `VREG`: output of the onboard 5 V switching regulator.
- `5V`: output of the logic power selection circuit.
- `3V3`: output of the 3.3 V LDO regulator.

The most important distinction:

- `VSW` powers the motors through the DRV8838 drivers.
- `5V` powers the MCU, motor-driver logic, and encoders.
- USB can power logic, but not motors.

## Battery Path

The Romi chassis has a six-AA battery compartment. The board connects to the chassis battery contacts.

Power path:

```text
AA battery pack
  -> VBAT
  -> reverse-voltage protection
  -> power switch circuit
  -> VSW
  -> DRV8838 motor VM pins
```

The important Rev A lesson is not the exact latching switch circuit. The lesson is that raw battery power should be protected, switched, measured, and clearly separated from logic power.

## USB And Logic Power

The Romi 32U4 board can be connected to USB for programming and serial communication.

Useful behavior:

- USB can power the MCU and most board logic.
- USB does not power the motors.
- It is safe to have USB connected while battery power is also on.
- Firmware can be uploaded or tested without installing batteries or spinning motors.

The board uses a power-selection circuit so logic can come from USB or the battery-derived regulator. This is elegant, but it is not mandatory for Rev A. Rev A only needs a clear, safe policy for what happens when USB, battery, or both are present.

## Regulated Rails

The Romi 32U4 board uses:

- A 5 V switching regulator from `VSW` to `VREG`.
- A power multiplexer that selects between USB 5 V and battery-derived 5 V for the board's `5V` logic rail.
- A 3.3 V LDO from the selected logic rail.

The 5 V rail supplies the ATmega32U4, motor-driver logic, encoders, and optionally a Raspberry Pi.

For Rev A:

- We do not need to power a Raspberry Pi.
- We might not need a 5 V logic rail if the selected MCU and encoder interface can run at 3.3 V.
- We still need stable logic power that is protected from motor-rail dips.

## Regulator And Power-Selection Breakdown

The Pololu board has several power blocks that are easy to confuse because they all sit between "battery/USB comes in" and "logic/motors run." They each solve a different problem.

### MP4423H 5 V Buck Regulator: `VSW` To `VREG`

Purpose:

- Convert switched battery voltage into regulated 5 V.
- Provide efficient logic/accessory power from the battery pack.
- Supply enough current for the board and, optionally, an attached Raspberry Pi.

Input:

- `VSW`, the switched battery rail after reverse protection and the board power switch.

Output:

- `VREG`, a regulated 5 V rail.

Why it is there:

- Six AA cells are not a stable 5 V source. NiMH cells are about 7.2 V nominal as a pack, and alkaline cells can be higher when fresh.
- The MCU, motor-driver logic, encoders, and Raspberry Pi-style accessories need regulated logic power.
- A switching buck regulator is much more efficient than a linear regulator when stepping a battery pack down to 5 V at hundreds of milliamps or more.

Extra behavior:

- The regulator has a `PG` power-good output.
- It has a shutdown control, `REGSHDN`.
- Pololu documents that disabling this regulator can allow board logic to be sourced from USB instead, if USB is available.

Rev A lesson:

- If Rev A needs battery-powered runtime logic, it needs a buck regulator or another efficient regulator from the protected battery path.
- If Rev A uses a 3.3 V STM32, we might use a battery-to-3.3 V buck directly instead of recreating a 5 V-first architecture.

### TPS2113A Power Multiplexer: `VREG` Or USB To `5V`

Purpose:

- Choose whether the board's main 5 V logic rail comes from battery-derived `VREG` or USB `VBUS`.
- Prevent unsafe contention between two possible 5 V sources.
- Let USB power the MCU for programming/debug while motor power is off.

Inputs:

- `VREG`: 5 V from the battery-powered buck regulator.
- `VBUS`: 5 V from USB.

Output:

- `5V`, the selected logic rail.

Why it is there:

- The board has two legitimate logic-power sources: battery-derived regulated 5 V and USB 5 V.
- Simply tying those sources together would be bad practice and can backfeed a computer, regulator, or accessory.
- The mux gives predictable behavior when USB is plugged in, batteries are installed, or both are present.

Useful behavior:

- Pololu configures the mux to prefer `VREG` when it is healthy.
- If `VREG` is too low, the mux can select USB.
- The board can be programmed over USB with the power switch off and motors unable to run.
- A `STAT` output indicates which source is selected.

Rev A lesson:

- We need an explicit policy for USB plus battery power.
- Rev A does not necessarily need a TPS2113A-class mux, but it must not casually tie USB 5 V to a battery-derived rail.
- A product-grade design should decide between a real mux, ideal-diode ORing, jumper selection, or data-only USB plus battery-derived runtime power.

### 3.3 V LDO: `5V` To `3V3`

Purpose:

- Create a clean 3.3 V rail for lower-voltage onboard devices.

Input:

- `5V`, after the power mux.

Output:

- `3V3`.

Why it is there:

- The Romi 32U4 board's main MCU is a 5 V ATmega32U4, but some peripherals are 3.3 V devices.
- Pololu uses `3V3` for onboard inertial sensors and level-shifting circuitry.
- The 3.3 V rail follows whichever source powers `5V`, so sensors work whether logic is powered from USB or battery-derived 5 V.

Rev A lesson:

- If Rev A uses an STM32, 3.3 V will probably become the main logic rail, not a secondary rail.
- That changes the architecture: the board might need one good 3.3 V regulator instead of a 5 V regulator plus a 3.3 V LDO.
- Keep a separate 5 V rail only if a chosen encoder, sensor, connector, or USB/accessory requirement actually needs it.

### Raspberry Pi Ideal-Diode / Backfeed Protection

Purpose:

- Allow the Romi board to provide 5 V to an attached Raspberry Pi while reducing reverse-current problems.

Power involved:

- Board `5V`.
- Raspberry Pi `RP5V`.
- Raspberry Pi USB power path.

Why it is there:

- The Romi 32U4 board was designed to support a Pi mounted on top.
- A Pi can be powered from the Romi board, or in some setups can have its own USB power.
- Without protection, one 5 V source can backfeed another source through the Pi or board connector.

Rev A lesson:

- This is mostly out of scope for Rev A because onboard computer power is deferred.
- Do not include Pi power circuitry until we have a concrete onboard-computer requirement.
- The broader lesson still applies: every external connector that carries power needs a backfeed story.

## Why There Are So Many Rails

The Romi 32U4 board is solving several user-experience problems at once:

- Motors need battery power and should not run from USB.
- The MCU should be programmable over USB with the robot power switch off.
- Logic should keep working whether power comes from battery or USB.
- Some devices need 5 V, while others need 3.3 V.
- An attached Raspberry Pi may need substantial 5 V power.
- Multiple external power sources must not fight each other.

That is why the board has `VSW`, `VREG`, `5V`, and `3V3` instead of one generic "power" net.

For Rev A, the product-grade version of this thinking is not "copy every rail." It is:

- Name every rail by what it means.
- Define which source can feed each rail.
- Prevent unwanted backfeeding.
- Keep motor power away from USB.
- Keep logic alive for telemetry and fault reporting.
- Add only the regulators required by the selected MCU, encoders, motor drivers, and host interface.

## Battery Voltage Measurement

The Romi 32U4 board measures switched battery voltage with a resistor divider. Pololu documents the battery-level input as one third of the battery voltage, keeping the MCU analog input under 5 V as long as battery voltage is below 15 V.

Why this matters:

- Motor speed depends on motor voltage.
- As batteries discharge, the same PWM command produces less wheel speed.
- Weak batteries can cause brownouts or poor control.
- Telemetry should show battery/motor voltage during tests.

Rev A should include the same concept: divide the battery or motor rail down to the MCU ADC range and log millivolts.

## Brownouts And Resets

The old project already noticed battery and brownout behavior, especially with Raspberry Pi power.

For Rev A, the likely reset causes are simpler:

- motor start current drops battery voltage
- motor stall pulls the rail down
- regulator dropout or undervoltage
- noisy ground return corrupts logic
- USB/battery source interaction is poorly controlled

The fix is not only "use a bigger regulator." The system needs:

- separate motor and logic paths
- local capacitance
- short motor-current loops
- clean ground strategy
- voltage telemetry
- current-limited bring-up

## What To Copy Conceptually

Use these ideas from the Romi 32U4 board:

- USB can power logic/debug without powering motors.
- Motor power comes from the battery side, not USB.
- Motor rail is switched or explicitly enabled.
- Battery/motor rail voltage is measured by the MCU.
- Logic rail is regulated and distinct from motor voltage.
- Power state is visible with LEDs or telemetry.
- Encoders and motor-driver logic get clean logic power.

## What Not To Copy Blindly

Do not copy these into Rev A unless a later decision requires them:

- Full latching pushbutton power circuit.
- Raspberry Pi power output.
- Raspberry Pi shutdown controls.
- USB/battery power multiplexer complexity.
- Full 5 V plus 3.3 V feature set before MCU choice is known.
- Every exposed power header from the Romi 32U4 board.

The Rev A board should be simpler than the Romi 32U4 board because its job is narrower: prove custom motor control, encoder logging, telemetry, and safe stop.

## Rev A Takeaway

Recommended Rev A power shape:

```text
USB
  -> debug/serial
  -> logic power path
  -> MCU stays alive for telemetry

Romi battery contacts or bench supply
  -> reverse-polarity protection
  -> motor enable/switch path
  -> VMOTOR
  -> motor-driver VM pins

VMOTOR or VBAT
  -> resistor divider
  -> MCU ADC
  -> telemetry
```

The board should make these states easy to observe:

- USB present.
- Motor power present.
- Motor output enabled.
- Battery/motor voltage.
- Low-battery or undervoltage warning.
- Command timeout.

## References

- Pololu Romi 32U4 Control Board User's Guide: https://www.pololu.com/docs/0J69/all
- Pololu Romi 32U4 resources: https://www.pololu.com/product/3544/resources
- Local guide: [`../docs/romi_32u4_control_board.pdf`](../docs/romi_32u4_control_board.pdf)
- Local pinout/power PDF: [`../docs/romi-32u4-control-board-pinout-power.pdf`](../docs/romi-32u4-control-board-pinout-power.pdf)
- Local schematic: [`../docs/romi-32u4-control-board-schematic-diagram.pdf`](../docs/romi-32u4-control-board-schematic-diagram.pdf)
