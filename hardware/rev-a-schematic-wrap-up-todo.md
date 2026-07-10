# Rev A Schematic Wrap-Up TODO

Use this as the gate before starting PCB routing/layout in earnest.

Current ERC status: clean as of the latest KiCad CLI run, with `0 violations`.

## Must Finish Before Layout

- [ ] Lock F1 part metadata to the selected JLC part: `MINISMDC260F/16-2`, `C16490`, Littelfuse, `1812`, `16 V`, `2.6 A hold`, `5 A trip`.
- [ ] Add or verify F1 datasheet/MPN fields so BOM review does not rely only on the generic `PTC_2.6A_16V` value.
- [ ] Confirm every schematic symbol has the intended footprint, especially motor drivers, motor/encoder connectors, battery lugs, power switch, USB connector, crystal, bulk capacitors, and test points.
- [ ] Confirm all JLCPCB/LCSC fields for assembled parts are populated and point to in-stock parts.
- [ ] Verify battery-lug polarity against the actual Romi chassis solderable battery contacts before routing the battery entry path.
- [ ] Verify left and right motor/encoder connector pin order against the physical motor plus encoder-board assembly.
- [ ] Verify left/right connector mirroring and header orientation so the PCB mates correctly with both encoder boards.
- [ ] Confirm encoder board electrical assumptions: encoder VCC requirement, open-drain A/B outputs, and pullups to `+3V3`.
- [ ] Confirm DRV8838 `SLEEP_N` default pulldowns keep both drivers asleep through MCU reset, boot, and debugger attach.
- [ ] Confirm motor PWM/EN and PH pins default to inactive states in hardware and firmware pin planning.
- [ ] Confirm each DRV8838 has local `VM` and logic-supply bypass capacitors with appropriate footprints.
- [ ] Confirm shared `+VSW`/motor-rail bulk capacitance is present, sized, and has a physical footprint suitable for motor transients.
- [ ] Add or verify ADC battery-sense filtering capacitor and confirm the divider cannot exceed the MCU ADC limit at maximum pack voltage.
- [ ] Verify USB policy in schematic: USB is telemetry/data only and does not power the board or motors.
- [ ] Confirm power LED, heartbeat LED, motor-enable/status LED, and fault/status visibility are represented in the schematic.
- [ ] Confirm enough test points exist for bring-up: `VBAT`, `+VSW`, `+3V3`, `GND`, SWD, UART/USB, motor outputs, `SLEEP_N`, PWM/EN, PH, fault, encoder A/B, and battery ADC.

## Mechanical Checks Before Layout

- [ ] Check PCB outline, mounting holes, and keepouts against the Romi chassis.
- [ ] Check clearance around battery contacts, battery bay, motor clips, encoder boards, wheels, chassis ribs, and caster area.
- [ ] Check access to the main power switch, USB connector, SWD header, reset/boot controls, and probe points.
- [ ] Decide whether any bench-power input is included; if yes, re-evaluate reverse-polarity protection for that input.

## Final KiCad Release Checks

- [ ] Re-run annotation after final schematic edits.
- [ ] Run ERC and keep the report at `0 violations`.
- [ ] Generate a fresh BOM and review all high-risk rows by hand: MCU, regulator, motor drivers, PTC, connectors, switch, crystal, USB, and bulk capacitors.
- [ ] Export or inspect the netlist for the power path: `BT1+ -> F1 -> SW2 -> +VSW`, then regulator and motor-driver `VM`.
- [ ] Sync schematic to PCB only after the above items are complete.
- [ ] Do an initial unrouted placement sanity check before committing to detailed routing.

## Documentation Cleanup

- [ ] Update the power architecture notes so the PTC is no longer an open decision once F1 is finalized.
- [ ] Update the motor-driver spec with the final connector pinout and orientation after physical verification.
- [ ] Update bring-up notes with the selected PTC and the current-limited test procedure.
