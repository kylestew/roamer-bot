# Rev B Upgrade Plan

Status: future backlog; Rev A remains the active board and is now feature-frozen.

## Purpose

Rev B should incorporate lessons from Rev A and add a small number of capabilities that are difficult to retrofit cleanly. It should not become a speculative redesign or delay fabrication of Rev A.

The intended Rev B direction is:

- preserve the Romi chassis mounting, battery contacts, motor connectors, and basic power architecture where Rev A proves them sound
- support a Raspberry Pi or similar host without transferring real-time motor safety to that host
- add inertial sensing and a general sensor-expansion path
- improve motor and power diagnostics where Rev A measurements justify the extra circuitry
- improve development power and USB behavior
- keep optional features unpopulated where practical so board area, rather than assembled BOM cost, preserves future choices

## Rev A Development Cutoff

Rev A is feature-frozen. Do not add the IMU, Raspberry Pi header, new motor drivers, a new MCU, a 5 V host rail, a compass, wireless, or other new capabilities before the first fabrication run.

Feature freeze does not mean ordering an electrically incomplete board. The following release work remains part of Rev A:

- finish every required connection, including both motor-driver power, logic, ground, and output routing
- complete the intended power pours, ground planes, via stitching, net classes, and thermal connections
- verify the buck input, switch, output, and feedback loops against the regulator guidance
- verify motor-driver local bypass, local bulk, exposed-pad grounding, and high-current return paths
- run ERC and DRC and resolve every real violation and unconnected required net
- verify battery-contact polarity, motor-connector mirroring, switch pinout, USB routing, and chassis clearances
- review the final BOM and placement files and inspect the assembler upload preview
- archive the exact schematic, PCB, fabrication outputs, BOM, placement file, and firmware revision used for the Rev A build

No known correctness problem should be intentionally deferred merely because Rev B is planned.

## Priority Summary

| Priority | Candidate change | Rev B position |
| --- | --- | --- |
| P0 | Corrections discovered during Rev A assembly and testing | Required |
| P1 | Raspberry Pi/SBC communications interface | Recommended |
| P1 | Onboard 6-axis IMU | Recommended |
| P1 | External 3.3 V sensor connector | Recommended |
| P2 | USB-powered logic with motor power isolated | Recommended if development inconvenience justifies it |
| P2 | Motor-current sensing, current limiting, and fault reporting | Decide from Rev A current and thermal results |
| P2 | Motor snubber, transient-clamp, and extra bulk footprints | Populate values from measurements |
| P3 | Board-provided 5 V Raspberry Pi power | Optional and limited to a defined host class |
| P3 | Newer MCU | Only if Rev A firmware or I/O limits are demonstrated |
| P3 | Remote compass, ToF, bump, line, or wireless modules | Add through expansion rather than integrating everything |

## Raspberry Pi And Similar Host Support

### System Responsibility

The STM32 remains responsible for:

- motor PWM and direction
- encoder counting
- closed-loop wheel control
- current or fault response, if available
- command timeout and unconditional safe stop
- battery and low-level sensor telemetry

The Raspberry Pi or other host may perform navigation, networking, vision, mapping, logging, and high-level planning. Loss, reboot, or brownout of the host must cause the STM32 command watchdog to stop the motors safely.

### Communications Baseline

Rev B should support three host paths, in this order:

1. **USB device:** retain native STM32 USB as the universal configuration, firmware, and telemetry interface. A Pi, laptop, or other USB host can use the same protocol.
2. **3.3 V UART:** provide a direct embedded-host link on the expansion header. UART is simple, generic, full-duplex, and does not require the STM32 to behave as an I2C target.
3. **I2C:** expose the Pi-standard I2C pins as an optional secondary interface for configuration or shared peripherals. Do not make I2C the only host-control path.

SPI can remain a future option unless Rev A telemetry demonstrates that USB and UART bandwidth are inadequate.

The host protocol should be transport-independent where practical. USB and UART should carry the same framed commands and telemetry, including:

- protocol version
- sequence number or timestamp
- command timeout/heartbeat
- left and right targets
- encoder counts and speed
- battery voltage
- reset and fault cause
- IMU samples or summaries
- per-motor current and fault state if Rev B adds those measurements

### Pi-Compatible 40-Pin Header

If mechanical space permits, add an unpopulated Raspberry Pi-compatible 2x20, 2.54 mm GPIO-header footprint. This gives a direct plug-on option while allowing the normal build to omit the connector.

Recommended initial signal use:

| Pi physical pin | Pi signal | Rev B use |
| --- | --- | --- |
| 3 | GPIO2 / SDA1 | Optional `HOST_I2C_SDA` through isolation provision |
| 5 | GPIO3 / SCL1 | Optional `HOST_I2C_SCL` through isolation provision |
| 6 or another ground pin | GND | Required shared signal ground |
| 8 | GPIO14 / TXD | Pi transmit to STM32 `HOST_RX` |
| 10 | GPIO15 / RXD | Pi receive from STM32 `HOST_TX` |
| 7 | GPIO4 | Optional STM32-to-host interrupt/status |
| 11 | GPIO17 | Optional host shutdown-ready or general handshake |
| 27, 28 | ID_SD, ID_SC | Leave unconnected unless a compliant ID EEPROM is added |
| 2, 4 | 5 V | No connection in the self-powered-host baseline |

GPIO assignments must be configurable in firmware and must be checked against boot-time behavior on the supported Pi models. Add small series-resistor footprints on signals driven across the header and default-state resistors where a floating input could create unsafe behavior.

The Pi GPIO interface is 3.3 V. Do not expose it to 5 V, `VBAT`, or motor outputs. Do not tie the Pi 3.3 V pins directly to the board's `+3V3` rail; if host-power presence sensing is useful, sense it through a high-value or buffered input that cannot back-power either board.

The board should only be called a Raspberry Pi HAT if it follows the complete HAT/HAT+ electrical, identification, connector, and mechanical requirements. A plug-compatible GPIO header without the required ID EEPROM and mechanics should be described simply as a Pi-compatible host connector.

### Generic Host Connector

The Pi header does not guarantee compatibility with every Pi-like SBC. If space permits, also provide a small keyed or clearly marked host connector with:

- GND
- `HOST_TX`
- `HOST_RX`
- `HOST_I2C_SDA`
- `HOST_I2C_SCL`
- one interrupt/handshake GPIO
- optional protected host-power output only if the host-power design is populated

This connector should use 3.3 V signaling and have an explicit pin-1 mark and power direction on silkscreen. USB remains the most universal interface for computers that do not share the Pi header pinout.

### Host Power Modes

Rev B must select and document one default host-power mode. It must not leave ambiguous connections between USB, the robot logic rail, and the host 5 V rail.

#### Mode A: Host Powered Separately — Recommended Baseline

- The Pi or other SBC uses its own approved 5 V input or power module.
- The robot board and host share ground and communications only.
- Pi-header 5 V and 3.3 V power pins are not tied to robot-board rails.
- This avoids backfeed, Pi inrush, filesystem shutdown, and combined motor/host brownout problems.
- It works with a wider range of SBC power requirements.

This is the safest Rev B baseline and does not require the robot board to provide a multi-ampere 5 V supply.

#### Mode B: Robot Board Powers A Small Host — Optional

If a single-battery robot is a firm requirement, define the exact supported host before designing the rail. A sensible first target would be a Raspberry Pi Zero 2 W-class host, not an unrestricted promise to power every Raspberry Pi.

This mode requires:

- a separate regulated `HOST_5V` rail, nominally near 5.1 V
- continuous and transient current capability based on the selected host and peripherals
- current limiting or a dedicated fuse
- reverse-current/backfeed protection at the Pi 5 V pins
- controlled startup or soft-start to handle host input capacitance
- local bulk capacitance and a low-impedance ground return that does not corrupt MCU or encoder ground
- power-good or voltage monitoring visible to the STM32
- thermal validation with both motors operating
- re-evaluation of F1, Q1, Q2, battery contacts, copper, and connector ratings for combined motor and host current
- an orderly shutdown handshake and a controlled host load switch if the main switch can remove host power

Official Raspberry Pi guidance lists substantially different supply requirements by model: approximately 2 A for Zero 2 W, 3 A for Pi 4, and up to 5 A for full Pi 5 operation. The existing Rev A input protection and 2.6 A-hold PTC must not be assumed to support motor current plus one of these host loads.

Pi 4- or Pi 5-class board power is out of scope unless the battery, protection path, regulator, thermal design, and mechanical use case are intentionally redesigned around it.

#### Mode C: Host Or USB Powers Robot Logic Only — Optional

Rev B may allow USB VBUS or a protected host 5 V source to power the STM32, IMU, and telemetry while the motor rail remains off. This would make firmware development and host communication possible without batteries.

This requires real source selection or ideal-diode behavior between USB-derived logic and battery-derived logic. It must not energize `VBAT_SW`, the encoder supply, or either motor driver, and it must prevent current from flowing back into the host.

### Mechanical Requirements For A Plug-On Pi

Before committing the 2x20 header location:

- model the intended Pi or uHAT outline above the Romi board
- verify mounting-hole and spacer locations
- preserve access to the Pi microSD card, USB, HDMI, camera, and display connectors as required
- avoid collisions with the robot power switch, USB connector, SWD header, battery contacts, wheels, and chassis structure
- preserve antenna clearance for a wireless Pi model
- define connector gender and whether the Pi mounts above or below the controller board
- verify that an inserted or removed host cannot mechanically stress the IMU area

Use at least the applicable Pi add-on-board spacing rules if the assembly resembles a HAT, even if the board is not marketed as one.

## Onboard IMU

Add one 6-axis accelerometer/gyroscope. An `LSM6DSO`-class part is the current preference because it provides a 3-axis accelerometer, 3-axis gyroscope, FIFO, I2C/SPI, and motion interrupts without requiring a separate sensor processor.

Expected uses:

- improve turn-angle estimates when fused with wheel encoders
- detect wheel slip or unexpected motion
- detect collision, lift, tilt, and rollover events
- improve short-term yaw estimation
- provide motion data to the attached host

Proposed STM32F103 assignment if that MCU remains:

- PB10: I2C2 SCL
- PB11: I2C2 SDA
- PB1 or another suitable spare GPIO: IMU data-ready interrupt

Layout requirements:

- place the IMU near the vehicle rotation center where practical, but away from the maximum board-flex point
- keep it away from the buck inductor, regulator switch node, motor drivers, motor connectors, and mounting-hole stress
- use the selected sensor's recommended land pattern and local decoupling
- maintain a quiet ground reference and keep high-current or fast-switching routes out of its immediate area
- avoid pogo pads, connectors, and mechanical pressure directly beneath it
- mark the sensor X/Y axes and the robot-forward direction on silkscreen
- provide I2C isolation resistors or solder jumpers so the onboard sensor can be disconnected during fault finding

The IMU provides useful relative motion but does not provide drift-free absolute heading.

## Sensor Expansion

Add a 3.3 V I2C expansion connector using a common four-pin order such as Qwiic/STEMMA QT:

- GND
- +3V3
- SDA
- SCL

Also provide at least one nearby interrupt/GPIO pad. Use only one set of fitted I2C pull-ups unless rise-time testing shows otherwise. Include isolation provision between the external connector and the onboard IMU bus so a damaged cable or external module does not permanently disable the onboard sensor.

Good external Rev B peripherals include:

- front or side time-of-flight ranging modules
- bumper switches
- line or floor sensor arrays
- a remote magnetometer
- a small display or user-interface board

A magnetometer should not be placed on the main motor-control PCB as a primary heading reference. Motors, motor current, batteries, steel fasteners, and the surrounding building can distort the field. If compass experiments are desired, mount the magnetometer remotely and treat it as a slowly varying correction that requires in-system calibration.

## Motor Diagnostics And Driver Options

Rev A uses DRV8838 drivers. They are sufficient for basic motor control but do not expose motor current, a fault output, or current regulation.

After Rev A testing, evaluate replacing each driver with a `DRV8213RTE`-class device. Potential benefits include:

- integrated current-sense output
- programmable current regulation
- explicit fault reporting
- hardware stall indication
- higher peak-current capability
- elimination of external power shunts

Two current-sense outputs would require two MCU ADC inputs. The driver change is not footprint-compatible and adds routing, passives, firmware, and validation, so it should be justified by measured stalls, thermal behavior, protection needs, or diagnostic goals.

Retain the wheel encoders even if a future driver offers sensorless speed or ripple counting. The existing encoder feedback is direct, already mechanically available, and useful for odometry.

## Power, USB, And Transient Refinements

Rev B should incorporate only the protection values supported by Rev A measurements. Candidate provisions are:

- unpopulated RC-snubber footprints at each motor connector
- an optional input or motor-rail transient-clamp footprint
- additional local motor-driver bulk-capacitor footprints
- improved thermal-via arrays beneath motor drivers
- revised high-current net classes and solid high-current pad connections
- a revised PTC or fuse rating based on measured combined stall current
- optional whole-board or per-motor current telemetry
- USB VBUS sensing and compliant USB attach/detach behavior
- protected USB/host-to-logic power selection while always isolating the motor rail

Do not choose a transient suppressor solely from the nominal battery voltage. Measure rail overshoot during motor startup, braking, reversal, unplugging, and stall recovery, then select the clamp and capacitance against the real driver limits.

## MCU Decision

Keep the `STM32F103CBT6` if it supports the required firmware, host interface, IMU, ADC channels, and timing. It is already adequate for two encoder timers, motor PWM, native USB, battery ADC, and ordinary sensor handling.

Consider a newer MCU only if Rev A demonstrates a concrete limitation. An `STM32G0B1CB`-class device is one candidate because it offers substantially more RAM, multiple I2C/UART interfaces, modern USB, more flexible DMA, and additional analog resources in a similar package class.

Changing the MCU is a firmware and pinout migration, not a drop-in BOM substitution. It should not be combined with every other Rev B change unless the additional resources are actually required.

## Features Not Recommended By Default

Do not add these merely because board space exists:

- onboard magnetometer as a trusted compass
- NiMH charging circuitry
- an unrestricted 5 V output advertised for any SBC
- microSD storage when the host can log telemetry
- a full 5 V logic rail unrelated to a defined host requirement
- Wi-Fi or Bluetooth silicon when the attached host already provides it
- a Raspberry Pi Compute Module carrier interface
- ROS-specific electrical hardware

These remain valid project options when a concrete mission requires them.

## Rev A Evidence Required Before Rev B Schematic Work

Record the following during Rev A bring-up:

- battery and `VBAT_SW` current at idle, one-motor operation, two-motor operation, startup, reversal, and stall
- `VBAT_SW` overshoot and droop at C16 and at each driver
- buck input and `+3V3` droop during motor events
- regulator, PTC, MOSFET, motor-driver, and connector temperatures
- MCU reset cause and USB stability during repeated motor transitions
- encoder error rate while both motors run
- motor-driver local supply waveform and output ringing
- actual usable battery voltage range with NiMH and any supported alkaline cells
- mechanical fit, switch access, USB access, and battery-contact reliability
- desirability of programming with the main battery switch off
- required host compute class, physical placement, peak power, and peripheral load

Rev B should begin only after these observations are written down and converted into specific requirements.

## Rev B Acceptance Criteria

Rev B is successful when:

- all verified Rev A electrical and mechanical defects are corrected
- the board still fits the Romi chassis without adapters
- USB and UART can each command and monitor the STM32 using the same safety semantics
- loss or reset of an attached host always results in a safe motor stop
- an attached host cannot expose STM32 GPIO to 5 V or back-power an unpowered rail
- any board-provided host power stays within voltage, ripple, current, and temperature limits during simultaneous motor operation
- the IMU produces stable data without excessive switching or mechanical-stress artifacts
- external sensor faults can be isolated from the onboard IMU and core motor controller
- motor current and fault telemetry work if the diagnostic-driver option is fitted
- USB-only or host-only power never energizes the motors

## Cost And Respin Control

Rev A is expected to uncover at least a few practical corrections; that is the purpose of a prototype revision. Keep the cost of learning controlled by:

- ordering a small Rev A quantity
- preserving the outline and mechanically proven footprints in Rev B
- using unpopulated footprints for uncertain snubbers, clamps, headers, and options
- avoiding a new MCU or motor driver until measurements justify it
- separating host communications from optional host power
- testing each power mode with current limits before attaching an SBC
- making one documented Rev B change set instead of several undocumented board edits

If Rev A passes the core motor, encoder, power, USB, and mechanical tests, Rev B is an upgrade rather than an emergency respin.

## Reference Material

- [Rev A power specification](rev-a-power-spec.md)
- [Rev A motor-power layout checklist](rev-a-motor-power-layout-checklist.md)
- [Rev A schematic review](rev-a-schematic-review.md)
- [Romi 32U4 power reference](romi-32u4-power-reference.md)
- [Raspberry Pi GPIO and power documentation](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html)
- [Raspberry Pi add-on board and HAT guidance](https://github.com/raspberrypi/hats)
- [ST LSM6DSO product page](https://www.st.com/en/mems-and-sensors/lsm6dso.html)
- [TI DRV8213 product page](https://www.ti.com/product/DRV8213)
- [ST STM32G0B1CB product page](https://www.st.com/en/microcontrollers-microprocessors/stm32g0b1cb.html)
