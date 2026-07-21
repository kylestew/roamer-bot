# Roamer Hardware

Status: **Rev A was ordered from JLCPCB on 2026-07-18 and is in production; fabrication and bring-up verification remain.**

This is the single source of truth for the custom Romi motor-controller hardware. If this document disagrees with the KiCad design, the schematic, PCB, and generated BOM are authoritative and this document must be corrected.

## Objective

Rev A replaces the Pololu Romi controller for the minimum useful rover loop:

- independently command two brushed DC motors;
- count both quadrature encoders in hardware;
- communicate with a laptop over native USB;
- report battery, motion, timeout, and reset state;
- stop both motors safely when commands cease; and
- mount directly to the Romi chassis and encoder boards.

Rev A is intentionally a focused learning board. It does not include an IMU, Raspberry Pi interface or power supply, wireless, charging, a 5 V accessory rail, or a polished multi-source power system.

Success means teleoperation, reliable encoder logging, useful telemetry, and an unconditional command-timeout stop during a short bench or floor run.

## Design Files

- [Top-level schematic](driver-board/driver-board.kicad_sch)
- [Power sheet](driver-board/power.kicad_sch)
- [Motor-control sheet](driver-board/motor_control.kicad_sch)
- [USB sheet](driver-board/usb.kicad_sch)
- [PCB](driver-board/driver-board.kicad_pcb)
- [KiCad project](driver-board/driver-board.kicad_pro)
- [Generated BOM snapshot](driver-board/driver-board.csv)
- [Rev A JLCPCB manufacturing release](releases/rev-a-jlcpcb-2026-07-18.md)
- [Custom footprints](driver-board/CustomParts.pretty/)

The checked-in CSV is a working snapshot, not purchase-ready assembly data. Regenerate and review the BOM and placement file from the final schematic and PCB immediately before ordering.

## Rev A Architecture

### Battery and power

Rev A is designed only for the Romi six-AA bay populated with rechargeable NiMH cells:

- nominal pack voltage: approximately 7.2 V;
- fresh-pack voltage: approximately 8.4 V;
- bench-supply ceiling: 8.5 V; and
- alkaline cells are prohibited because a fresh pack can approach 9.6 V, reducing DRV8838 transient margin and increasing motor stall current beyond the intended operating envelope.

The implemented load-current path is:

```text
BT1+ -> VBAT -> F1 -> Q1 -> VBAT_SAFE -> Q2 -> VBAT_SW
BT1- -> BT2+
BT2- -> GND
```

- F1 is a Littelfuse `MINISMDC260F/16-2` resettable PTC: 16 V, 2.6 A hold, 5 A trip.
- Q1 (`AO3401A`) provides reverse-battery protection.
- Q2 (`AO4407A`) is the high-side whole-board power MOSFET.
- SW2 (`SK-3296S-01-L2`) switches Q2's gate; it does not carry motor or regulator load current.
- `VBAT_SW` feeds both motor-driver VM inputs, both encoder boards, and the 3.3 V buck regulator.
- USB VBUS is used only around the USB interface. It does not power `+3V3`, the encoders, or the motors.

The LMR51430 buck, 5.6 uH L1, and output network generate approximately 3.3 V from `VBAT_SW`. There is no 5 V logic rail, separate `+3V3A` rail, or analog-rail inductor. VDDA is fed directly from `+3V3` with local 10 nF and 1 uF decoupling.

Battery telemetry uses a 100 kOhm / 33 kOhm divider into PB0/ADC1_IN8. Rev A deliberately omits a filter capacitor; firmware must use a long ADC sample time and modest averaging. This is approximate state telemetry, not precision fuel gauging.

### Motor power and protection

Rev A uses one TI DRV8838 per motor. The target motor is the Pololu #1520 120:1 high-power mini plastic gearmotor, specified around 3-6 V with approximately 1.25 A stall current at 4.5 V.

Normal firmware output is capped at 75% absolute PWM duty, following the Pololu Romi operating policy. Full-duty operation is diagnostic-only, with a current-limited supply and explicit test firmware.

Motor-rail capacitance is:

| Driver | VM high-frequency bypass | VM local storage | VCC bypass |
| --- | --- | --- | --- |
| U3, left | C11, 100 nF | C10, 10 uF | C12, 100 nF |
| U4, right | C14, 100 nF | C13, 10 uF | C15, 100 nF |

C16 is a shared 220 uF, 25 V bulk capacitor across `VBAT_SW` and GND. It buffers short current steps and returned energy but does not replace the battery as the sustained current source. Rev A has no TVS or characterized regenerative clamp; startup, stop, reversal, stall-release, and power-off peaks must be measured during bring-up.

Each DRV8838 `nSLEEP` input has a hardware pulldown. Firmware must also initialize `nSLEEP`, PWM/EN, and PH to disabled states before accepting commands. A host timeout, watchdog event, low-battery condition, or unsafe state commands PWM to zero and puts the drivers to sleep while the MCU remains powered.

DRV8838 provides no fault or current-sense output. Diagnose problems through commanded state, encoder response, battery voltage, reset cause, temperature, and measured supply waveforms.

### Motor and encoder connectors

The encoder boards are powered from `VBAT_SW` because their specified VCC range begins above 3.3 V. Their A/B outputs are open-drain and pulled up to `+3V3`, so the STM32 receives safe 3.3 V logic. Firmware must treat encoder counts as unreliable when the battery approaches the encoder's 3.5 V minimum plus margin.

Viewed from the top of the PCB:

| Connector | Top-to-bottom pin order |
| --- | --- |
| J4, left | `GND`, `ENC_L_A`, `ENC_L_B`, `VBAT_SW`, `ML+`, `ML-` |
| J3, right | `MR-`, `MR+`, `VBAT_SW`, `ENC_R_A`, `ENC_R_B`, `GND` |

The right connector is intentionally mirrored for cleaner symmetric routing and does not reproduce the Pololu silkscreen order. Confirm actual wheel-forward polarity and encoder count sign during bring-up; correct direction convention in firmware rather than casually changing the verified connector layout.

### MCU and firmware-facing pin contract

U1 is an `STM32F103CBT6` in LQFP-48 with a 16 MHz HSE crystal, native USB FS, SWD, two hardware encoder timers, two PWM channels, and battery ADC input.

| Function | MCU pin | Peripheral or mode |
| --- | --- | --- |
| Heartbeat LED | PC13 | GPIO, active-low |
| Left sleep | PB12 | GPIO, active-low |
| Left direction | PB13 | GPIO |
| Left PWM | PB14 | TIM1 CH2N |
| Right sleep | PB5 | GPIO, active-low |
| Right direction | PB6 | GPIO |
| Right PWM | PB7 | TIM4 CH2 |
| Left encoder A/B | PA6 / PA7 | TIM3 CH1 / CH2 |
| Right encoder A/B | PB3 / PA15 | TIM2 CH2 / CH1, partial remap 1 |
| Battery level | PB0 | ADC1 IN8 |
| USB D-/D+ | PA11 / PA12 | USB FS |
| SWDIO/SWCLK | PA13 / PA14 | SWD; SWO is not connected |

The MCU contains I2C2 on PB10/PB11, but those package pins are explicit no-connects on Rev A. There are no pullups, connector, or sensor footprint, so Rev A has no usable I2C bus. Remove the stale I2C2 assignment from the CubeMX design artifact before release. User buttons and additional controlled status LEDs are also deferred; Rev A has reset, a power LED, and one MCU heartbeat/status LED.

SWD is the primary programming and recovery path. The 1.27 mm 2x5 header exposes target power, GND, SWDIO, SWCLK, and NRST; the target supplies its own 3.3 V.

### USB policy

J2 is a USB-C USB 2.0 receptacle with 5.1 kOhm CC resistors and USBLC6-2 ESD protection. D+/D- run directly to the STM32 without external series resistors and are routed via-free on `F.Cu` over the solid ground plane.

Rev A is battery-powered and USB-data-only:

- USB alone leaves the board off.
- Battery/bench power with USB disconnected supports standalone operation.
- Battery/bench power plus USB supports programming, commands, and telemetry without backfeeding the board from VBUS.

R5 permanently enables the external D+ pullup whenever board 3.3 V is present; attachment is not gated by VBUS. Rev A therefore assumes connection to an already-powered host. If enumeration fails after connection to an unpowered or booting host, unplugging and reconnecting USB is the accepted recovery. Add VBUS sensing and pullup gating in Rev B if reliable or standards-compliant self-powered attachment is required.

## PCB and Manufacturing Policy

Rev A is a four-layer, approximately 1.6 mm FR-4 board:

| Layer | Primary use |
| --- | --- |
| `F.Cu` | Components, signals, short local power pours, and regulator switching loops |
| `In1.Cu` | Continuous GND plane; never split into motor and logic grounds |
| `In2.Cu` | `VBAT_SW` and `+3V3` power distribution |
| `B.Cu` | Lower-speed signals plus a stitched GND fill |

Rules to preserve:

- Keep USB, clocks, feedback, ADC, reset, and other sensitive signals over continuous ground.
- Do not route a bottom-layer signal across an `In2.Cu` power split without a same-layer or otherwise continuous return reference.
- Keep the U5 VIN, bootstrap, switch-node, inductor, output-capacitor, and feedback loops compact.
- Keep motor-current paths and returns short and broad, with multiple vias where current changes layers.
- Keep the DRV8838 exposed-pad solder areas free of drilled holes. The implemented nearby ordinary GND vias require no via-in-pad fill or cap process.
- Keep motor current away from encoder, USB, crystal, reset, SWD, and BATLEV routing.

The smallest intentional signal vias are 0.55 mm diameter with a 0.25 mm drill. These are within JLCPCB rigid-board capability and have a 0.15 mm radial annular ring. Continue to use 0.70/0.30 mm or larger vias for high-current, thermal, and principal ground paths when space permits.

Silkscreen is front-side only. The board includes assembly and connector labels, polarity and pin-1 marks, test-point labels, board/battery identification, and the capybara artwork. There is no back silkscreen.

Mechanical fit has been checked repeatedly against the Romi chassis, encoder boards, battery contacts, wheels, clips, ribs, caster area, mounting holes, switch access, and USB cable access. Preserve the verified board outline and chassis-critical footprints.

## Current Verification State

As of 2026-07-21:

- JLCPCB order `W2026071807372374` is in production and its PCB production file is confirmed;
- the order contains five PCBs, including two economically assembled PCBAs;
- fabrication completion and physical bring-up remain;
- schematic ERC: **0 violations**;
- PCB DRC: **0 violations**;
- PCB connectivity: **0 unconnected items**;
- SW2 SMD-anchor pad semantics and silkscreen clearances are fixed in both the library footprint and board instance; and
- top, bottom, and isometric views plus a 1:1 chassis comparison have been reviewed.

The KiCad project intentionally excludes several generic checks, including missing courtyards, track endpoints not centered on vias, tuning-profile geometry, footprint filters, and footprint type. A clean report means the active Rev A rule set passes; it does not replace the fabrication preview and manual checks below.

## Fabrication Release Gate

Complete every item before ordering:

- [ ] Remove the unusable I2C2 PB10/PB11 assignment from the CubeMX artifact and keep I2C, user buttons, and extra LEDs explicitly deferred to Rev B.
- [ ] Refill all zones, complete the final GND-stitching pass, and confirm no isolated copper islands.
- [ ] Rerun ERC and PCB DRC with schematic parity, warnings, exclusions, and zero unconnected items reviewed.
- [ ] Reinspect the complete power path, battery series link, Q1/Q2 orientation, both driver VM/output/ground loops, U5 layout, USB routing, NRST, BOOT0, oscillator, and MCU decoupling.
- [ ] Verify both `nSLEEP` pulldowns hold the drivers off during reset, boot, SWD attach, unpowered-MCU states, and firmware initialization.
- [ ] Verify BATLEV stability with motors and the buck switching; accept the unfiltered divider only if firmware sampling is adequate.
- [ ] Confirm the actual JLCPCB stackup and USB geometry, drill and slot sizes, annular rings, mask dams, copper-edge clearance, and silkscreen capability.
- [ ] Regenerate the BOM and CPL/position data from the final design.
- [ ] Fill or explicitly waive every missing JLCPCB field, especially C22, C23, R5, R12, R13, and R16-R19.
- [ ] Manually verify U1, U3/U4, U5, F1, Q1, Q2, SW2, J2, J3/J4, L1, C16, and C17-C21 against current value, package, polarity, voltage/current rating, lifecycle, and stock.
- [ ] Review Gerbers, drills, IPC netlist, BOM, and CPL independently, then inspect the JLCPCB fabrication and assembly previews for side, rotation, pin 1, polarity, DNP state, and footprint alignment.
- [ ] Archive the exact schematic, PCB, fabrication outputs, BOM, CPL, and firmware revision used for the build.

Useful release commands:

```sh
/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli sch erc \
  --severity-all hardware/driver-board/driver-board.kicad_sch

/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli pcb drc \
  --schematic-parity --severity-all \
  hardware/driver-board/driver-board.kicad_pcb
```

## Bring-Up Plan

1. Inspect polarity, orientation, shorts, solder joints, and the unpowered resistance of each rail.
2. Use a current-limited bench supply at a low voltage with motors disconnected; verify `VBAT`, `VBAT_SAFE`, `VBAT_SW`, and `+3V3` in order.
3. Verify SW2/Q2 behavior, Q1 reverse protection, no USB backfeed, and USB-only board-off behavior.
4. Attach SWD, confirm NRST/BOOT behavior, program the safe image, and prove both `nSLEEP` inputs and PWM outputs remain inactive through reset and debugger attach.
5. Confirm the 16 MHz HSE/48 MHz system clock, heartbeat LED, USB enumeration, battery ADC reading, and reset-cause telemetry.
6. Connect one motor and its encoder. Start at low duty, verify direction/count sign, sweep duty conservatively, and test timeout stop.
7. Repeat for the other motor, then operate both together while checking encoder integrity and the 3.3 V rail.
8. With a short oscilloscope ground spring, capture `VBAT_SW` at C16 and each driver during start, stop, reversal, simultaneous start, stall release, and power-off at nominal and fresh-pack voltage.
9. Check the PTC, MOSFETs, regulator, inductor, motor drivers, connectors, and C16 temperature during an extended test.
10. Run low-speed straight, turn, stop, and host-disconnect floor tests while saving telemetry.

Bring-up passes when both motors operate independently in both directions, encoder counts remain clean, PWM response is monotonic, command timeout and watchdog stop the motors, USB remains stable, and motor events do not reset the MCU or violate measured rail margins.

Record Rev A current, voltage transients, temperatures, resets, encoder errors, mechanical fit, and USB behavior. Those measurements—not guesses—select Rev B protection and power changes.

## Critical Parts

| Function | Part | Current JLCPCB/LCSC ID |
| --- | --- | --- |
| MCU | STM32F103CBT6 | C8304 |
| Motor drivers | DRV8838, U3/U4 | C86667 |
| Buck regulator | LMR51430XDDCR | C5185863 |
| Buck inductor | SWPA8065S5R6MT, 5.6 uH | C96977 |
| USB ESD | USBLC6-2SC6 | C7519 |
| USB-C connector | GT-USB-7010ASV | C2988369 |
| Input PTC | MINISMDC260F/16-2 | C16490 |
| Reverse-protection MOSFET | AO3401A | C15127 |
| Power MOSFET | AO4407A | C16072 |
| Power switch | SK-3296S-01-L2 | C500051 |
| Bulk capacitor | 220 uF, 25 V | C190724 |
| HSE crystal | 16 MHz, 3225 | C13738 |

Recheck every ID and stock state at order time; this table records the design, not a sourcing guarantee.

## Rev B Backlog

Rev B begins only after Rev A assembly and measured testing. Priorities are:

1. Correct every demonstrated Rev A electrical, mechanical, sourcing, or firmware-contract defect.
2. Retain USB and add a 3.3 V UART host interface; keep motor safety, encoder counting, timeout, and low-level control on the STM32.
3. Add a 6-axis IMU and an isolatable 3.3 V I2C/Qwiic-style sensor connector.
4. Consider an unpopulated Pi-compatible header, but default to a separately powered host sharing only communications and GND. Do not advertise HAT compatibility without meeting the complete specification.
5. Add VBUS sensing and gated USB attachment; optionally support USB/host-powered logic while guaranteeing the motor rail remains off and no source can backfeed another.
6. Add snubber, clamp/TVS, distributed bulk-capacitance, current-sense, or fault-reporting provisions only where Rev A measurements justify them.
7. Consider a diagnostic motor driver such as DRV8213 and a newer MCU such as STM32G0B1 only if Rev A exposes a concrete limitation.
8. Do not add a magnetometer beside motors or an unrestricted multi-amp Pi supply by default. Remote magnetic sensing and host power require explicit mechanical and power requirements.

Any host reset, disconnect, or brownout must still cause the STM32 watchdog to stop the motors safely.

## References and Asset Provenance

Local vendor references retained as primary sources:

- [STM32F103 datasheet](docs/stm32f103c8.pdf)
- [DRV8838 datasheet](docs/drv8838.pdf)
- [LMR51430 datasheet](docs/lmr51430.pdf)
- [AO4407A / C16072 reference](docs/C16072.pdf)
- [SK-3296S-01-L2 / C500051 reference](docs/C500051.pdf)
- [Reset switch / C720477 reference](docs/C720477.pdf)
- [Alternate switch / C1788495 reference](driver-board/datasheets/C1788495.pdf), not fitted on current Rev A

External design references:

- [Pololu Romi 32U4 control-board guide](https://www.pololu.com/docs/0J69/all)
- [Pololu Romi 32U4 resources](https://www.pololu.com/product/3544/resources)
- [Pololu #1520 motor](https://www.pololu.com/product/1520)
- [Pololu Romi encoder pair](https://www.pololu.com/product/3542)
- [ST USB hardware and PCB guidelines, AN4879](https://www.st.com/resource/en/application_note/an4879-introduction-to-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.pdf)
- [Samsung CL21A106KAYNNNE data](https://product.samsungsem.com/mlcc/CL21A106KAYNNN.do)
- [Raspberry Pi hardware documentation](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html)

3D-model provenance:

- `driver-board/3dmodels/GT-USB-7010ASV.step` comes from the GPL-2.0-licensed [MiSTerLaggy_MiSTer hardware repository](https://github.com/MiSTer-devel/MiSTerLaggy_MiSTer/blob/main/hardware/GT-USB-7010ASV.step).
- `driver-board/3dmodels/SK-3296S-01-L2.wrl` was converted without changing millimetre-scale alignment from the [EasyEDA model](https://modules.easyeda.com/3dmodel/a3deeb560a8c4467ab29e6daf75b62b9) for [LCSC C500051](https://www.lcsc.com/product-detail/Electrical-Switches_XKB-Connectivity-SK-3296S-01-L2_C500051.html).
