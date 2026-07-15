# Rev A PCB Placement Review TODO

Use this checklist to finish component placement before starting detailed routing. It covers placement and routing readiness only; incomplete traces, vias, copper fills, and thermal connections are intentionally outside its scope.

Review basis: the saved `driver-board/driver-board.kicad_pcb` placement, the local component datasheets, the Pololu Romi mechanical arrangement, and the existing Rev A design notes.

## Required Before Routing

- [ ] Move `C3` from the reset-switch/header area to immediately beside `U1` pin 7 (`NRST`).
  - Keep the `NRST` connection and C3 ground return very short.
  - `SW1` and `J1` can remain at the right edge; only the capacitor needs to be local to the MCU.
- [ ] Move `R1` immediately beside `U1` pin 44 (`BOOT0`).
  - Keep the default-state pulldown local to the MCU.
  - Extend any optional boot-control connection outward from that local node.
- [ ] Move `R13` immediately beside `U4` pin 7 (`nSLEEP`).
  - Use the short placement of `R12` beside `U3` as the model.
  - Keep the driver shutdown default local instead of leaving a long safety-net branch.

## Strong Placement Improvements

- [ ] Compact `R16` and `R17` into an end-to-end BATLEV divider close to `U1` pin 18.
  - Put the divided BATLEV junction nearest the MCU.
  - Let the longer conductor be on the battery/source side of the divider, not on the high-impedance ADC node.
- [ ] Resolve the USB `D+`/`D-` line-order crossover before routing.
  - Current ordering presents `D+` on the left and `D-` on the right at the MCU side of `U2`, while `U1` expects `D-` on the left and `D+` on the right.
  - Preferred solution: swap the two interchangeable USBLC6 channel assignments in the schematic, then arrange `R9`/`D-` on the left and `R8`/`D+` on the right.
  - Acceptance check: the differential pair can run straight and parallel from the ESD/series-resistor area into U1 without a crossover.

## Test Access Decisions

- [ ] Decide whether Rev A needs a dedicated `VBAT_SAFE` test point.
- [ ] Decide whether to add accessible left/right motor-control test pads or a compact unpopulated header for:
  - `nSLEEP`
  - `EN/PWM`
  - `PH`
- [ ] Confirm there is an easily probed local ground point near the power and/or motor areas.
  - Connector ground may be sufficient if it is physically accessible with the intended probes.
- [ ] Do not add USB `D+`/`D-` test pads unless there is a compelling measurement requirement; avoid unnecessary stubs.

## Assembly and Footprint Checks

- [ ] Add two or preferably three non-collinear top-side global fiducials before routing.
  - Reserve their copper/mask clearances before nearby routes are committed.
- [ ] Select and verify the actual low-profile socket parts for `J3` and `J4`.
  - Confirm body height, mating orientation, pin numbering, and clearance to the Romi encoder assemblies.
  - Do not rely on the generic KiCad socket 3D model as proof of mechanical compatibility.
- [ ] Resolve the `J2` footprint-specific manufacturing constraints with the intended fabricator:
  - approximately 0.12-0.15 mm pad-to-pad clearance in the connector footprint
  - approximately 0.25 mm shell mounting drill versus the current generic 0.30 mm minimum-drill rule
  - Treat these as footprint/local-rule checks, not reasons to move the connector.
- [ ] Resolve the intentional `SW2` board-edge condition.
  - The present mechanical pad is approximately 0.41 mm from the edge versus the generic 0.50 mm rule.
  - Prefer an approved local edge-rule exception if the fabricator supports the manufacturer geometry.
  - If necessary, move the switch inward only enough to satisfy fabrication requirements while preserving actuator access and overhang.

## Confirmed Placement to Preserve

- [ ] Keep `U1` in its current central location and orientation.
  - Crystal pins face the oscillator area.
  - USB pins face the USB area.
  - SWD pins face `J1`.
  - Motor-control signals escape toward their respective sides.
- [ ] Keep `C4`-`C8` with the current MCU supply-pin grouping.
- [ ] Keep `Y1`, `C1`, and `C2` in the current quiet area above U1 unless making the optional small clock-cluster refinement below.
- [ ] Keep `J2` aligned to the bottom board edge and keep `U2`, `C9`, `R6`, `R7`, `R8`, `R9`, and `R5` in the same USB cluster.
- [ ] Keep the left motor-driver cluster in place: `U3`, `C10`, `C11`, `C12`, `R12`, and `J4`.
- [ ] Keep the right motor-driver cluster in place: `U4`, `C13`, `C14`, `C15`, and `J3`, aside from moving `R13` closer to U4.
- [ ] Keep the battery-protection and switching sequence in place: `F1`, `Q1`, `Q2`, `SW2`, `R22`, and `R23`.
- [ ] Keep the LMR51430 regulator cluster substantially unchanged: `U5`, `L1`, its input/output capacitors, bootstrap capacitor, and `R18`/`R19` feedback divider.
- [ ] Keep `C16` near the `VBAT_SW` entrance/regulator area.
  - It is board-level switched-rail bulk capacitance.
  - Each motor driver already has local 10 uF plus 100 nF supply decoupling.
- [ ] Preserve the verified mounting holes, battery-contact footprints, and motor/encoder connector coordinates.

## Optional Refinements

- [ ] If it can be done without disrupting the surrounding placement, nudge `Y1`, `C1`, and `C2` about 1-2 mm closer to U1 and make the two oscillator connections more symmetric.
  - This is not a routing blocker at 16 MHz.
- [ ] If it improves the bootstrap loop without disturbing the quiet feedback area, nudge or rotate `C19` slightly closer to U5.
  - Do not compromise the `R18`/`R19` feedback placement for this marginal improvement.
- [ ] Leave `J1`, `SW1`, `D2`, and `R21` at the accessible right edge unless later routing proves that a small adjustment is beneficial.

## Placement Completion Gate

Do not begin detailed routing until all of the following are true:

- [ ] `C3`, `R1`, and `R13` have been moved and visually checked at their associated IC pins.
- [ ] The BATLEV divider arrangement is finalized.
- [ ] The USB channel/order decision is reflected in both schematic and placement.
- [ ] Test-access decisions are complete so no late footprints need to interrupt routed areas.
- [ ] Fiducials and final connector bodies/keepouts are present.
- [ ] `J2` and `SW2` local fabrication-rule decisions are documented.
- [ ] A fresh unrouted placement inspection reports no courtyard collisions or unintended board-edge violations.

Once this gate is complete, route in this order: high-current battery/motor paths, regulator loops, USB differential pair, clock/reset/BOOT0, remaining control signals, then low-speed signals and test access.
