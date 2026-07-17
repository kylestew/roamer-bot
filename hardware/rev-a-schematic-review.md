# Rev A Schematic Review

**Review date:** 2026-07-13

**Scope:** `hardware/driver-board` schematics, current PCB/BOM consistency, power and motor assumptions, and the firmware-facing electrical contract.

## Bottom line

No confirmed must-fix circuit fault remains from this review. The remaining schematic-stage decision is how much probe-friendly test access to add before routing. Several deliberate Rev A limitations and Rev B follow-ups are documented below.

## Priority summary

| Priority | Finding | Recommended disposition |
|---|---|---|
| Important | Bring-up observability is sparse | Add accessible power/ground loops and motor-control test access before routing |

## Assumptions most likely to be wrong

- **The BOM reflects placed parts.** It must be regenerated after the final schematic changes; the checked-in CSV still contains the previous L1 selection.
- **The intended operating sequence is followed.** Firmware must initialize and disable the motor drivers safely through reset, boot, command timeout, and power-down paths.

This review covers schematic intent, ratings, interfaces, and document consistency. It is not a substitute for a dedicated PCB layout review of switch-current loops, USB differential routing, grounding, thermal copper, connector mechanics, or fabrication rules.

## Accepted Rev A limitations

- **USB attachment is not gated by VBUS.** R5 permanently enables the D+ pull-up whenever battery-powered 3.3 V is present. Rev A requires batteries for programming and is expected to be connected to an already-powered host. If enumeration fails after connection to an unpowered or starting host, unplugging and reconnecting USB is the accepted recovery. Revisit VBUS-gated attachment for Rev B, unattended use, or USB-compliant product behavior.
- **BATLEV is deliberately approximate.** Rev A retains the 100 kΩ/33 kΩ divider without a filter capacitor. Firmware will use a long ADC sample time and modest averaging, but the result is general battery telemetry rather than a calibrated precision measurement.
- **Motor-rail transient characterization is deferred to Rev B.** Rev A relies on the six-cell NiMH voltage limit, 220 uF rail capacitance, DRV8838 protection, and conservative firmware behavior without proving the peak `VBAT_SW` voltage during regeneration, reversal, stall release, or power-off. Rev B work includes measuring those cases and adding a clamp or revising the power-switch arrangement if the results require it.

## Detailed schematic finding

### 1. Bring-up and fault observability is sparse

The schematic currently exposes `VBAT`, `+3V3`, and `GND` with 2 mm SMD test pads. J3/J4 expose the motor outputs and encoder signals, while the SWD header exposes SWDIO, SWCLK, NRST, +3V3, and GND.

Before routing, add probe access that materially improves fault isolation:

- clip-friendly access to `VBAT_SAFE` and `VBAT_SW`;
- local ground access beside the power and motor-control groups; and
- `nSLEEP`, `EN/PWM`, and `PH` for each motor driver.

Avoid dedicated USB D+/D- test pads because their stubs can degrade signal integrity. Motor-output and encoder test pads are optional because their connector pins are already accessible. A practical implementation is clip-friendly power/ground loops plus an unpopulated grouped header or small pads for motor-control signals.

Because the DRV8838 has no fault output and no current-sense output, firmware should report commanded state, encoder motion, battery voltage, and watchdog/reset cause so a stalled or cycling channel is diagnosable indirectly.

## Firmware-facing electrical requirements

Motor-control behavior, approximate BATLEV sampling, and safety policy are tracked in [`../firmware/DESIGN.md`](../firmware/DESIGN.md).

## Items checked and found reasonable

- Q1's P-channel MOSFET orientation provides reverse-battery protection with the expected body-diode startup direction.
- Q2 and SW2 form a sensible high-side motor-power control arrangement; SW2 carries gate-control current rather than motor current.
- D2 is wired from +3V3 through R21 and the LED into PC13, so PC13 sinks current as required. Firmware treats it as active-low.
- J3/J4 intentionally use a geometrically mirrored connector layout rather than exactly matching the Romi 32U4 connector silkscreen. Viewed from the top, left J4 is `GND, ENC_L_A, ENC_L_B, VBAT_SW, ML+, ML-` from top to bottom; right J3 is `MR-, MR+, VBAT_SW, ENC_R_A, ENC_R_B, GND`. The right-side motor-pin order is deliberate: mirroring the left block produces cleaner routing and makes the two driver layouts easier to understand and maintain. Do not swap `MR+` and `MR-` merely to match the Pololu board. `VBAT_SW` also intentionally replaces the Pololu board's `5V` encoder supply. Wheel-forward motor polarity and encoder count sign remain bring-up checks and may be corrected in firmware.
- L1 is Sunlord `SWPA8065S5R6MT` (`C96977`), 5.6 uH with 8.0 A minimum saturation current and 4.5 A minimum heat-rating current. Its `Inductor_SMD:L_APV_ANR8065` land pattern matches the Sunlord 8 x 8 x 6.5 mm package and provides an accurate-height 3D model.
- Each DRV8838 has local ceramic supply decoupling, the rail has bulk capacitance, exposed pads are grounded, and `nSLEEP` has a default pull-down.
- VDDA is fed from `+3V3` through L2 (39 nH) onto the dedicated `+3V3A` rail, with C22 (10 nF) and C23 (1 uF) placed on the filtered side beside U1 pin 9.
- Encoder boards are powered from `VBAT_SW` while A/B pull-ups go to 3.3 V, matching the open-collector encoder outputs and preventing a 5-8.4 V logic level at the MCU.
- USB CC resistors, ESD protection, series resistors, and D+/D- MCU mapping are broadly sensible. The fixed D+ pull-up is an accepted Rev A limitation documented above.
- The LMR51430 feedback divider calculates to approximately 3.315 V, and the input/output capacitor topology is sensible.
- The battery series connection and major power-net partitioning are coherent.
- KiCad electrical-rules checking previously completed with zero reported violations. ERC does not detect every functional, rating, sourcing, or manufacturing inconsistency, so it must be rerun after the final changes.

## Pre-routing exit checklist

- [ ] Decide and add the final bring-up test loops/pads/header.
- [ ] Regenerate the CSV BOM after all schematic changes are complete.
- [ ] Update the power, motor, MCU, BOM, and assembly documents so they describe the same circuit.
- [ ] Rerun ERC after final schematic edits.
- [ ] Run PCB DRC after placement and routing are complete.

## Source material

Primary component and interface references used for this review:

- [STM32F103x8/xB datasheet](https://www.st.com/resource/en/datasheet/stm32f103cb.pdf)
- [ST AN4879: USB hardware and PCB guidelines](https://www.st.com/resource/en/application_note/an4879-introduction-to-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.pdf)
- [TI DRV8838 datasheet](https://www.ti.com/lit/ds/symlink/drv8838.pdf)
- [TI LMR51430 datasheet](https://www.ti.com/lit/ds/symlink/lmr51430.pdf)
- [Sunlord SWPA inductor series datasheet](https://www.sunlordinc.com/uploads/files/20221122/SWPA%20series%20of%20SMD%20Power%20Inductor.pdf)
- [Pololu Romi encoder pair product information](https://www.pololu.com/product-info-merged/3542)
- [Pololu Romi encoder pair schematic](https://www.pololu.com/file/0J1208/romi-encoder-pair-kit-schematic-diagram.pdf)
- [Pololu Romi 32U4 connector silkscreen reference](../docs/0J7509.1200.jpg)
- [Littelfuse miniSMD resettable PTC datasheet](https://www.littelfuse.com/~/media/electronics/datasheets/resettable_ptcs/littelfuse_ptc_minismdc_datasheet.pdf.pdf)
- [AOS AO4407A P-channel MOSFET datasheet](https://www.aosmd.com/sites/default/files/res/datasheets/AO4407A.pdf)

Project documents reviewed:

- [`driver-board/driver-board.kicad_sch`](driver-board/driver-board.kicad_sch)
- [`driver-board/motor_control.kicad_sch`](driver-board/motor_control.kicad_sch)
- [`driver-board/power.kicad_sch`](driver-board/power.kicad_sch)
- [`driver-board/driver-board.csv`](driver-board/driver-board.csv)
- [`rev-a-stm32-mcu-requirements.md`](rev-a-stm32-mcu-requirements.md)
- [`rev-a-power-spec.md`](rev-a-power-spec.md)
- [`rev-a-motor-driver-spec.md`](rev-a-motor-driver-spec.md)
- [`../ELECTRONICS_DESIGN_SPEC.md`](../ELECTRONICS_DESIGN_SPEC.md)
- [`../STUDIO_MISSION_ROVER_ROADMAP.md`](../STUDIO_MISSION_ROVER_ROADMAP.md)
