# TO BUY

Last reviewed: 2026-07-17

This is the purchasing list for the Rev A milestone: tethered teleoperation, encoder logging, and safe stop. Existing workshop and legacy-Romi inventory has not yet been checked, so items marked "if not already owned" should be verified before ordering.

## Buy Now

- [ ] **8 x AA NiMH cells from one matching batch.** Recommended: standard white Panasonic eneloop, 1.2 V, 1900 mAh minimum. The rover uses six cells; label and keep six as one set and retain two as spares. Buy twelve instead if a complete second six-cell set is useful.
  - Rev A is NiMH-only: about 7.2 V nominal and up to about 8.4 V just charged.
  - Do not use alkaline, 1.5 V lithium, or regulated/rechargeable 1.5 V AA cells.
  - Product reference: <https://www.panasonic.com/global/energy/products/eneloop/en/lineup/eneloop.html>
- [ ] **One individual-channel AA NiMH charger.** Recommended: Panasonic eneloop `BQ-CC63` eight-slot charger, in the UK-plug version. It charges and terminates each of 1-8 cells independently, so all six rover cells can be charged in one pass.
  - Charge the loose cells in the charger, not in the rover.
  - Product reference: <https://www.panasonic.com/global/energy/products/eneloop/en/lineup/charger-bq-cc63.html>
- [x] **One Pololu Romi Chassis Kit — purchased.** Any colour is electrically equivalent; the current project reference is white, Pololu `#3509`.
  - Includes chassis, two 120:1 HP motors, motor clips, two wheels, one ball caster, and the complete battery-contact set.
  - Product reference: <https://www.pololu.com/product/3509>
- [x] **One Pololu Romi Encoder Pair Kit — purchased.** Pololu `#3542`.
  - Includes two encoder boards, two magnetic discs, and the low-profile male/female headers needed by the custom PCB.
  - Product reference: <https://www.pololu.com/product/3542>
- [ ] **#2-56 mounting hardware:** at least four 3/16-inch screws and four nuts; buying a packet of ten gives useful spares. The chassis kit does not include this hardware.

The chassis and encoder kit should be in hand before PCB release so battery-tab polarity, connector fit, header height, and board clearance can be checked physically.

## Needed Before First Power-On

- [ ] **One SWD programmer/debugger with a Cortex-M MIPI-10 cable**, if not already owned. The PCB uses a keyed 10-pin, 1.27 mm ARM SWD connector.
  - Turnkey recommendation: genuine `STLINK-V3SET`, which includes the STDC14-to-MIPI10 cable.
  - Do not buy an `STLINK-V3MINIE` alone for this board; it includes only an STDC14-to-STDC14 cable, so it also needs a separate STDC14-to-MIPI10 adapter cable.
  - Product reference: <https://www.st.com/en/development-tools/stlink-v3set.html>
- [ ] **One long, flexible USB 2.0 data cable** for tethered telemetry: USB-C at the rover end and the connector required by the development laptop at the host end.
  - Recommended starting length: 3 m. This should give useful floor-test range without excessive cable weight or drag.
  - It must carry data, not be a charge-only cable; prefer a USB-IF-certified cable from a reputable manufacturer.
  - USB 2.0 allows up to 5 m for a passive cable. For a longer tether, use a compliant active USB 2.0 extension/repeater rather than chaining passive extension leads.
- [ ] **Light cable strain relief:** a reusable hook-and-loop tie or clip that transfers cable pull to the chassis instead of the PCB's USB-C socket. Leave a small service loop at the connector.
- [ ] **Current-limited bench supply**, if not already owned or borrowable: adjustable through 0-8.5 V, CV/CC operation, output-enable button, and preferably at least 5 A available. Start bring-up with a much lower current limit and one motor at low duty.
- [ ] **Digital multimeter**, if not already owned, with DC voltage, resistance/continuity, diode test, and at least 5 A fused DC-current capability.
- [ ] **Two pairs of safe bench leads:** 4 mm banana leads plus insulated mini-grabbers or hook clips. Final probe/lead choice depends on the board's still-open bench-power and test-loop decision.
- [ ] **Assembly tools**, if missing:
  - temperature-controlled soldering iron and suitable small tip
  - electronics solder and flux
  - solder wick
  - fine tweezers and small pliers
  - flush cutters
  - small Phillips screwdriver
  - isopropyl alcohol and a small cleaning brush
  - eye protection and fume extraction

## Order When Rev A Is Released

Do not place this order until routing, DRC, mechanical checks, BOM regeneration, and the assembler preview are complete.

- [ ] **Five four-layer Rev A PCBs**, with at least three SMT-assembled. Keep more than one assembled board so a damaged bring-up board does not stop firmware work.
- [ ] **SMT assembly from the released BOM and placement files.** Check every extended part, rotation, side, and out-of-stock substitution in the upload preview.
- [ ] **Any hand-soldered test loops or test header** chosen during the final test-access decision.
- [ ] **Spare low-profile 1x6 header pair only if needed.** The new Pololu encoder kit already supplies the normal pair; first verify that its physical parts fit `J3` and `J4` as intended.

The Romi battery terminals and encoder headers will be hand-soldered after the assembled PCBs arrive. They are supplied by the chassis and encoder kits, respectively.

## Borrow or Buy Before Full Motor Characterisation

- [ ] **Two-channel oscilloscope**, ideally 50 MHz or better, with switchable 10x probes and short ground springs. This is useful for checking `VBAT_SW`, 3.3 V droop, PWM, reset, and encoder integrity during motor starts, stops, and reversals. Borrowing one is entirely reasonable for Rev A.

## Deliberately Not Buying for Rev A

- Pololu Romi control board or motor-driver board; the custom PCB replaces it.
- Raspberry Pi or other onboard computer.
- IMU, compass, lidar, camera, range sensors, or sensor expansion hardware.
- A second caster or expansion deck.
- Onboard NiMH charging circuitry.
- Alkaline or 1.5 V lithium AA cells.
