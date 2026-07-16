# Rev A Schematic Wrap-Up TODO

Use this as the gate before starting PCB routing/layout in earnest, and as the checklist for layout-stage verification before fabrication.

Current ERC status: clean as of the latest KiCad CLI run, with `0 violations`.

## Must Finish Before Layout

- [x] Lock F1 part metadata to the selected JLC part: `MINISMDC260F/16-2`, `C16490`, Littelfuse, `1812`, `16 V`, `2.6 A hold`, `5 A trip`.
- [x] Add or verify F1 datasheet/MPN fields so BOM review does not rely only on a generic PTC value.
- [ ] Confirm every schematic symbol has the intended footprint, especially motor drivers, motor/encoder connectors, battery lugs, power switch, USB connector, crystal, bulk capacitors, and test points.
  - Footprint audit status: motor drivers, motor/encoder connectors, USB connector, crystal, bulk capacitors, PTC, fiducials, LEDs, passives, test points, and battery-contact placeholders have footprints assigned.
  - `BT1`/`BT2` use first-pass custom footprint `CustomParts:Romi_Chassis_BatteryContactPair_2Slot_10_44mm_FirstPass` with 1.02 mm x 4.06 mm plated slots on 10.44 mm pitch from the Pololu power-distribution-board dimension callouts.
  - Before routing, verify the battery-contact footprint against the actual Romi chassis tabs or imported Pololu DXF, including polarity, slot pitch, board-side orientation, and whether `BT1`/`BT2` should remain separate two-slot footprints or become one four-contact chassis footprint.
  - `SW2` uses custom footprint `CustomParts:SW_SPDT_Angled_YuenFung_MT-O-102-C003-N003-RS` for JLCPCB `C1788495`; place it at the PCB edge using the footprint's `Dwgs.User` guide/keepout, then verify pad ordering and actuator direction against the physical switch before routing.
- [ ] Confirm all JLCPCB/LCSC fields for assembled parts are populated and point to in-stock parts.
- [ ] Verify battery-lug polarity against the actual Romi chassis solderable battery contacts before routing the battery entry path.
- [ ] Verify left and right motor/encoder connector pin order against the physical motor plus encoder-board assembly.
- [ ] Verify left/right connector mirroring and header orientation so the PCB mates correctly with both encoder boards.
- [ ] Confirm encoder board electrical assumptions: encoder VCC requirement, open-drain A/B outputs, and pullups to `+3V3`.
- [ ] Confirm DRV8838 `SLEEP_N` default pulldowns keep both drivers asleep through MCU reset, boot, and debugger attach.
- [ ] Confirm motor PWM/EN and PH pins default to inactive states in hardware and firmware pin planning.
- [ ] Confirm each DRV8838 has local `VM` and logic-supply bypass capacitors with appropriate footprints.
- [ ] Confirm shared `+VSW`/motor-rail bulk capacitance is present, sized, and has a physical footprint suitable for motor transients.
- [x] Rev A decision: omit the BATLEV filtering capacitor; precise battery measurement is not important for this design. The 100 kΩ/33 kΩ divider remains safely below the MCU ADC limit at maximum pack voltage.
- [ ] Verify USB policy in schematic: USB is telemetry/data only and does not power the board or motors.
- [ ] Confirm power LED, heartbeat LED, motor-enable/status LED, and fault/status visibility are represented in the schematic.
- [ ] Confirm enough test points exist for bring-up: `VBAT`, `+VSW`, `+3V3`, `GND`, SWD, UART/USB, motor outputs, `SLEEP_N`, PWM/EN, PH, fault, encoder A/B, and battery ADC.

## Mechanical Checks Before Layout

- [ ] Check PCB outline, mounting holes, and keepouts against the Romi chassis.
- [ ] Check clearance around battery contacts, battery bay, motor clips, encoder boards, wheels, chassis ribs, and caster area.
- [ ] Check access to the main power switch, USB connector, SWD header, reset/boot controls, and probe points.
- [ ] Confirm the `SW2` toggle body/lever overhang clears the Romi chassis and is reachable at the board edge.
- [ ] Decide whether any bench-power input is included; if yes, re-evaluate reverse-polarity protection for that input.

## Verify During Placement And Routing

- [ ] Start with mechanical placement: board outline, mounting holes, battery contacts, motor/encoder connectors, `SW2`, USB, SWD, reset/boot controls, and required keepouts.
- [ ] Place `BT1`/`BT2` so the physical Romi battery contacts really form the intended series chain: `BT1+ -> VBAT`, `BT1- -> BT2+`, `BT2- -> GND`.
- [ ] Verify `SW2` pad numbering against the actual toggle switch footprint before routing copper: common pin to `VBAT_SAFE`, selected throw to `VBAT_SW`, unused throw no-connect.
- [ ] Place input protection in order and physically close together: battery positive -> F1 PTC -> Q1 reverse-polarity MOSFET -> `VBAT_SAFE` -> `SW2`.
- [ ] Route high-current battery and motor paths first with short, wide traces/pours: battery entry, F1, Q1, `SW2`, `VBAT_SW`, DRV8838 `VM`, motor outputs, and motor-current returns.
- [ ] Keep motor-current loops compact and away from encoder A/B, USB, SWD, crystal, reset, and BATLEV ADC routing.
- [ ] Place DRV8838 `VM` and logic bypass capacitors tight to their IC pins; place the shared `VBAT_SW` bulk capacitor near the motor-driver supply entry/cluster.
- [ ] Place the LMR51430 buck loop tightly: input cap, regulator, bootstrap cap, inductor, output caps, feedback divider, and ground return per datasheet layout guidance.
- [ ] Keep L2 between the main `+3V3` feed and the `+3V3A` island; place C22/C23 immediately on the filtered side beside U1 VDDA/VSSA with no alternate copper path around L2.
- [ ] Keep the buck switch node small and away from encoder, USB, crystal, SWD, reset, and ADC traces.
- [ ] Route BATLEV as a reasonably quiet ADC node after the divider; no dedicated filter capacitor is required for Rev A.
- [ ] Route encoder VCC from `VBAT_SW` with awareness that it is a noisy battery rail; keep encoder A/B traces referenced to quiet ground and pulled up to `+3V3`.
- [ ] Verify left/right motor and encoder connector orientation in the PCB view so motor polarity, encoder VCC, encoder A/B, and GND are not mirrored or swapped.
- [ ] Put bring-up test points where probes can physically reach them: `VBAT`, `VBAT_SAFE`, `VBAT_SW`, `+3V3`, `GND`, BATLEV, motor outputs, `SLEEP_N`, PWM/EN, PH, encoder A/B.

## Verify After Routing Before Fabrication

- [ ] Run DRC with the selected JLCPCB design rules and resolve every real clearance, unconnected, courtyard, and silkscreen issue.
- [ ] Re-run ERC after any schematic edits made during layout and keep the report at `0 violations`.
- [ ] Inspect the final routed power path in PCB/net-highlight view: `BT1+ -> F1 -> Q1 -> VBAT_SAFE -> SW2 -> VBAT_SW`.
- [ ] Inspect the final routed battery series path: `BT1- -> BT2+`, and `BT2- -> GND`.
- [ ] Inspect reverse-polarity MOSFET routing against the AO3401A pinout and footprint: drain to fused battery input, source to protected rail, gate to GND.
- [ ] Inspect `SW2` in 3D or footprint view: actuator direction, edge access, pad numbering, unused throw isolation, and chassis clearance.
- [ ] Inspect motor-driver routing for adequate trace width, thermal copper, exposed-pad grounding, and compact bypass placement.
- [ ] Inspect the buck regulator layout against the LMR51430 datasheet recommendations before ordering.
- [ ] Inspect USB routing and power policy: VBUS remains local to USB/protection/sense and does not power `+3V3`, `VBAT_SW`, or motors.
- [ ] Inspect encoder connector routes against the physical encoder boards: VCC from `VBAT_SW`, A/B pulled to `+3V3`, ground continuity, left/right orientation.
- [ ] Review final BOM/CPL upload preview in JLCPCB: rotations, side, footprints, part numbers, Basic/Extended classification, and hand-solder/DNP items.
- [ ] Print or overlay the PCB at 1:1 scale against the Romi chassis before ordering.

## Final KiCad Release Checks

- [ ] Re-run annotation after final schematic edits.
- [ ] Run ERC and keep the report at `0 violations`.
- [ ] Generate a fresh BOM and review all high-risk rows by hand: MCU, regulator, motor drivers, PTC, connectors, switch, crystal, USB, and bulk capacitors.
- [ ] Export or inspect the netlist for the power path: `BT1+ -> F1 -> SW2 -> +VSW`, then regulator and motor-driver `VM`.
- [ ] Sync schematic to PCB only after the above items are complete.
- [ ] Do an initial unrouted placement sanity check before committing to detailed routing.

## Documentation Cleanup

- [x] Update the power architecture notes so the PTC is no longer an open decision once F1 is finalized.
- [ ] Update the motor-driver spec with the final connector pinout and orientation after physical verification.
- [ ] Update bring-up notes with the selected PTC and the current-limited test procedure.
