# Rev A MCU Selection Spec

Status: provisional MCU selected for Rev A schematic work

Decision locked: Rev A will use a bare STM32-family MCU on the custom PCB.

Decision locked for now: use `STM32F103CBT6`.

Open verification: confirm JLCPCB availability/library support and final LQFP-48 pin assignment before PCB release.

Purpose: define the MCU needs clearly enough that part selection becomes a constrained engineering choice instead of a habit-based guess.

## Selection Philosophy

Rev A should look like a small professional embedded motor-control product:

- Bare STM32 MCU on the PCB, not a dev board module.
- 3.3 V MCU logic.
- SWD programming and debugging.
- Hardware timers for motor PWM, encoder counting, and control-loop timing.
- ADC support for battery/motor-rail telemetry.
- Reset, boot, and watchdog behavior designed up front.
- Package that can be assembled, inspected, and reworked during prototype bring-up.
- Enough spare pins to recover from one wiring mistake or feature addition.

The selected MCU is `STM32F103CBT6`, a 128 KB Flash STM32F1 part in the 48-pin LQFP package class evaluated for Rev A. The final schematic pin assignment still needs to prove that USB, SWD, two encoder timers, two PWM outputs, ADC, I2C, buttons, LEDs, and spare GPIO route cleanly.

## Required Product Behavior

The MCU must support:

- independent left/right motor command outputs
- two PWM outputs for motor speed
- two GPIO direction outputs
- one shared GPIO driver sleep output for both motor drivers
- two hardware quadrature encoder interfaces using STM32 timer encoder mode
- battery or motor-rail voltage measurement
- native USB device command/telemetry link to a laptop or host
- one I2C bus for an optional IMU/accelerometer or board peripherals
- one or two user buttons
- two to four MCU-controlled status LEDs
- command-timeout safe stop
- heartbeat/status indication
- SWD debug
- watchdog recovery

Preferred but not mandatory for Rev A:

- logic-rail voltage measurement
- separate motor-enable/fault indicators
- spare UART for debug logs
- spare SPI or second I2C bus for future sensors/expansion
- nonvolatile calibration storage

## Current Motor And Encoder Interface

The motor/encoder schematic now reserves these MCU-facing signals:

```text
DRV_L_PH       left motor direction
DRV_L_EN       left motor PWM
DRV_R_PH       right motor direction
DRV_R_EN       right motor PWM

DRV_SLEEP_N    shared DRV8838 active-low sleep

ENC_L_A        left encoder channel A
ENC_L_B        left encoder channel B
ENC_R_A        right encoder channel A
ENC_R_B        right encoder channel B
```

The DRV8838 logic rail is `+3V3`, so the MCU can drive `PH`, `EN`, and `SLEEP_N` directly.

The Romi encoder boards are powered from `+5V`, but their A/B outputs are open-drain and pulled up to `+3V3`. The MCU therefore sees normal 3.3 V logic levels on all encoder inputs.

## Minimum Pin Budget

| Function | Pins | Requirement |
| --- | ---: | --- |
| Left motor PWM | 1 | Timer PWM output. |
| Right motor PWM | 1 | Timer PWM output. |
| Left motor direction | 1 | GPIO output. |
| Right motor direction | 1 | GPIO output. |
| Shared driver sleep | 1 | GPIO output, default-safe during reset. |
| Left encoder A/B | 2 | Must route to `CH1`/`CH2` of one encoder-capable timer instance. |
| Right encoder A/B | 2 | Must route to `CH1`/`CH2` of a second encoder-capable timer instance. |
| Battery or `+VSW` sense | 1 | ADC input. |
| Host USB | 2 | Native USB device: `USB_DM`/`USB_DP`. |
| Debug UART | 0-2 | Optional only; SWD remains the primary debug path. |
| I2C bus | 2 | Required for optional IMU/accelerometer and future board peripherals. |
| IMU interrupt | 0-1 | Preferred GPIO input if an IMU is populated. |
| SPI bus | 0 or 3-4 | Optional future expansion; not required for Rev A. |
| SWD | 2 | `SWDIO`, `SWCLK`. |
| Reset | 1 | `NRST`, strongly preferred on debug header. |
| User buttons | 1-2 | Reserve GPIOs for bring-up/user input. |
| Status LEDs | 2-4 | Reserve GPIOs for heartbeat, USB/telemetry, motor-enable, and fault/status. |
| Boot/config | 0-2 | Depends on selected STM32 family. |

Minimum practical target:

- At least 12 confirmed application I/O pins before USB/debug/buttons/LEDs.
- At least 5 spare GPIO after all required Rev A signals are assigned.
- Prefer 48-pin LQFP as the starting package.
- Accept 64-pin LQFP if timer pinout, USB, ADC, or routing margin justify it.

## Timer Requirements

Required:

- Two PWM-capable output channels that can route cleanly to `DRV_L_EN` and `DRV_R_EN`.
- Two independent timer instances that support quadrature encoder mode, one per wheel.
- Encoder channels must be counted in hardware from timer input transitions, not by firmware interrupts on every GPIO edge.
- One periodic timebase for the control loop.

Preferred:

- PWM timers with enough resolution at the intended PWM frequency.
- Encoder timers configured for x4 decoding, counting both edges of both channels in hardware.
- Optional timer update interrupts only for overflow tracking or periodic sampling.

Avoid selecting a part where motor PWM and both encoder timers fight over the same limited pins.

GPIO edge interrupts are not the baseline encoder strategy for Rev A. They are a contingency only if the design is intentionally revised and the performance/CPU-load tradeoff is accepted.

## ADC Requirements

Required:

- At least one ADC input for battery or motor-rail voltage sense.

Preferred:

- ADC/DMA support for low-overhead telemetry sampling.

The selected part must allow stable ADC readings in a noisy motor environment. Board layout, filtering, and reference behavior matter as much as the ADC peripheral itself.

## Peripheral Expansion

Required:

- One I2C controller with routable `SCL` and `SDA` pins.
- I2C pins should support normal external pullups to `+3V3`.
- The I2C bus should be available for an optional IMU/accelerometer and lightweight board peripherals.

Preferred:

- One GPIO interrupt input near the I2C bus for an IMU data-ready or interrupt signal.
- A spare SPI bus or a second I2C bus if the selected package has room.
- At least one extra GPIO near the sensor area for chip-select, reset, or address selection.

Rev A does not have to populate an IMU, but the MCU selection should not block adding one. The Pololu Romi 32U4 board includes an IMU, and while we are not copying that feature blindly, preserving the option is useful for later motion sensing, collision detection, and orientation experiments.

## Host Communication

Rev A should target native USB device support on the STM32.

Pros:

- No USB-UART bridge IC.
- Direct firmware update and telemetry path possible.
- Product-like for a tethered robot controller.

MCU requirements:

- Native USB FS device peripheral.
- Usable `USB_DM` and `USB_DP` pins in the selected package.
- Clock plan that satisfies USB timing.

A USB-UART bridge is a fallback only if native USB creates a part-selection or clocking problem. It is not the baseline Rev A target.

## Debug, Reset, And Boot

Required:

- SWD debug access: `SWDIO`, `SWCLK`, GND, target voltage.
- `NRST` exposed on the debug header or accessible test pad.
- Standard ST-style SWD/debug connector compatibility.
- Boot configuration that cannot accidentally put the board into an unusable state during normal reset.
- Firmware watchdog support.

Preferred:

- BOOT pin pulled to normal-run state with a documented override method.
- One spare debug/status GPIO routed to a test point.

Bootloader-only development is not acceptable as the primary workflow.

## Safe Default Requirements

During reset, boot, and debugger attach:

- Motor drivers must remain asleep or disabled.
- PWM outputs must not produce accidental pulses.
- Encoder inputs may float only if external pullups are already present.
- The MCU must be able to boot with motor power absent.
- USB/debug logic must not energize the motors.

The current motor schematic helps this by pulling DRV8838 `SLEEP_N` low with `100k`. MCU pin selection should not fight that default.

## Electrical And Package Requirements

Logic:

- MCU core I/O voltage: `+3V3`.
- GPIO outputs must drive DRV8838 logic inputs at 3.3 V.
- Encoder inputs must tolerate 3.3 V pullups.
- Avoid any required 5 V logic interface.

Clocking:

- Internal oscillator is acceptable only if the selected host link and timing requirements allow it.
- `STM32F103CBT6` native USB requires a clean USB clock plan; Rev A should include an HSE crystal or resonator suitable for generating the 48 MHz USB clock through the PLL.
- PWM, encoder counting, and telemetry timing must be stable enough for closed-loop speed control.

Package:

- Preferred: LQFP-48.
- Acceptable: LQFP-64 if it materially improves pinout/routing.
- Avoid QFN/BGA for Rev A unless there is a strong reason.

Availability:

- Before committing to a part, check current ST lifecycle status and normal distributor availability.
- Avoid parts that are hard to source, marked end-of-life, or only available in inconvenient packages.

## Selected MCU

Provisional Rev A part:

- `STM32F103CBT6`
- STM32F1 family
- Arm Cortex-M3 core
- 128 KB Flash
- 20 KB SRAM
- 48-pin LQFP package class
- 3.3 V logic operation
- native USB FS device support
- SWD debug support
- hardware timers suitable for PWM and quadrature encoder use
- ADC and I2C peripherals for voltage telemetry and optional sensor/peripheral expansion

Selection rationale:

- It satisfies the Rev A motor-control, encoder, USB, ADC, I2C, and debug requirements at lower cost than the initially selected G4 candidate.
- It keeps the assembly-friendly LQFP-48 target.
- It provides 128 KB Flash, which is comfortable for tight non-generated firmware.
- It is an older, simpler Cortex-M3 part, but Rev A does not require the extra G4 motor-control or ADC headroom.
- It should provide enough peripheral headroom for the Rev A motor-control board without moving to a larger package.

Known tradeoffs:

- USB clocking requires an HSE crystal/resonator and PLL plan.
- Pin assignment must account for STM32F1 alternate-function remap behavior.
- Use SWD as the debug interface and free unused JTAG pins in firmware where practical.

Before PCB release:

- Confirm JLCPCB sourcing or assembly-library availability.
- Confirm the exact KiCad symbol/footprint mapping.
- Complete a pin assignment against the candidate evaluation checklist below.

## STM32 Family Starting Points

The search started with:

- STM32G4: strongest fit if we want motor-control headroom, better timers, ADC growth, and current-sense options.
- STM32G0: good fit if simplicity, cost, and pin count matter more than advanced control peripherals.
- STM32F3: useful comparison family for motor-control-friendly peripherals, but check lifecycle and availability before choosing it.
- STM32F1: older mainstream family, but attractive if price and JLCPCB availability are materially better and the Rev A requirements still fit.

Less likely for Rev A:

- STM32F0: older and less attractive for a fresh design.
- STM32F4: more compute than needed unless future onboard processing becomes concrete.
- STM32L series: useful for low-power products, not the main fit for a motor-control bring-up board.

Initial bias was to evaluate STM32G4 and STM32G0 candidates in 48-pin LQFP packages first. `STM32F103CBT6` is the current provisional selection because it appears to satisfy the Rev A requirements at lower cost. Move to 64-pin LQFP only if the final 48-pin pin assignment cannot satisfy USB, encoder timers, I2C, buttons, LEDs, ADC, and spare GPIO cleanly.

## Candidate Evaluation Checklist

For each candidate STM32, answer:

1. Does it have two independent timer instances that explicitly support encoder interface mode?
2. Can `ENC_L_A/B` and `ENC_R_A/B` each route to `CH1`/`CH2` pins on those two timer instances?
3. Does it have two PWM outputs that route cleanly to `DRV_L_EN` and `DRV_R_EN`?
4. Are GPIOs available for `DRV_L_PH`, `DRV_R_PH`, and shared `DRV_SLEEP_N`?
5. Is one ADC pin available for `+VSW` or battery sense?
6. Does it have a routable I2C bus for an optional IMU/accelerometer?
7. Is there a spare GPIO suitable for an IMU interrupt if we populate one?
8. Does it support native USB FS device?
9. Can USB clocking be solved cleanly, with internal recovery or a reasonable external clock part?
10. Can SWD and `NRST` be routed without stealing critical motor/encoder pins?
11. Are GPIOs available for one or two buttons and two to four status LEDs?
12. Is the part available in LQFP-48 or LQFP-64?
13. Does the package leave at least 5 spare GPIO?
14. Is the part currently available from normal distributors?
15. Is the documentation and toolchain support good enough for a first custom board?

## Reserved Signal Names

Use these names in schematic work before final pin assignment:

```text
DRV_L_PH
DRV_L_EN
DRV_R_PH
DRV_R_EN
DRV_SLEEP_N

ENC_L_A
ENC_L_B
ENC_R_A
ENC_R_B

VSW_SENSE
VBAT_SENSE

USB_DM
USB_DP
DBG_UART_TX
DBG_UART_RX

I2C_SCL
I2C_SDA
IMU_INT
SPI_SCK
SPI_MISO
SPI_MOSI

SWDIO
SWCLK
NRST
BOOT0

LED_HEARTBEAT
LED_USB
LED_MOTOR_EN
LED_FAULT
BTN_USER1
BTN_USER2
```

Not every optional signal must be populated on Rev A, but the selected MCU should make room for the preferred set unless doing so clearly bloats the board.

## Acceptance Criteria

The selected STM32 is acceptable for Rev A when:

- It satisfies all required motor, encoder, ADC, debug, and host-link needs.
- It provides two motor PWM outputs and two hardware quadrature encoder interfaces without timer conflicts.
- It provides a routable I2C bus for optional sensor/peripheral expansion.
- It supports native USB FS device operation with a realistic clock plan.
- It supports one or two buttons and two to four status LEDs within the pin budget.
- It leaves at least 5 spare GPIO after required Rev A routing.
- It exposes standard ST-style SWD and reset cleanly.
- It comes in an assembly-friendly package.
- It is sourceable and documented.
- It does not force a dev-board carrier, 5 V MCU logic, or fragile boot/debug workflow.

## References

- Motor driver spec: [`rev-a-motor-driver-spec.md`](rev-a-motor-driver-spec.md)
- Power spec: [`rev-a-power-spec.md`](rev-a-power-spec.md)
- Romi 32U4 power reference: [`romi-32u4-power-reference.md`](romi-32u4-power-reference.md)
