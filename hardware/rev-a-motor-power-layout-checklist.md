# Rev A Motor-Power PCB Layout Checklist

Use this checklist while placing and routing the DRV8838 motor-driver power path. It records the intended roles of the motor-rail capacitors and the checks needed before fabrication.

## Capacitor Roles

| Driver | VM high-frequency bypass | VM local storage | VCC bypass |
| --- | --- | --- | --- |
| U3, left motor | C11, 100 nF | C10, 10 uF | C12, 100 nF |
| U4, right motor | C14, 100 nF | C13, 10 uF | C15, 100 nF |

C16 is the shared `VBAT_SW` bulk capacitor: 220 uF, 25 V, low-ESR aluminum electrolytic. It supports both drivers and matches the 220 uF VSW bulk value used by the Pololu Romi 32U4 reference design.

These capacitors do not replace the battery as the sustained current source:

- C11/C14 supply the fastest H-bridge switching edges.
- C10/C13 support local VM current over short PWM transients.
- C16 buffers the shared motor rail against wiring/copper inductance, motor-current steps, and returned motor energy.
- The NiMH pack and high-current copper path supply sustained acceleration and stall current.

For a first-order droop estimate, use `delta V = I * delta t / C`. For example, an ideal 220 uF capacitor supplying 2 A for 100 us drops about 0.9 V. This calculation intentionally ignores help from the battery and losses from capacitor ESR, trace resistance, and inductance; final sizing requires measurement.

## Schematic And BOM Checks

- [ ] Keep C10 and C13 as local 10 uF ceramics; do not replace the 100 nF bypass capacitors with them.
- [ ] Confirm C11 and C14 connect from each DRV8838 `VM` pin to GND.
- [ ] Confirm C12 and C15 connect from each DRV8838 `VCC` pin to GND.
- [ ] Confirm C16 connects directly across `VBAT_SW` and GND with correct polarity.
- [ ] Confirm C16 is at least 220 uF and has voltage, ESR, ripple-current, temperature, and lifetime ratings suitable for the motor rail.
- [ ] Retain comfortable voltage margin above the 8.4 V freshly charged six-cell NiMH pack and possible regenerative spikes; the selected C16 is rated for 25 V.
- [ ] Correct the C10/C13 description: JLCPCB `C15850`, Samsung `CL21A106KAYNNNE`, is X5R rather than X7R.
- [ ] Use the Samsung DC-bias curve when estimating the effective C10/C13 capacitance at `VBAT_SW`; do not assume the full nominal 10 uF remains under bias.
- [ ] Recheck JLCPCB availability and manufacturer part numbers before release.
- [ ] Consider an optional DNP footprint for a second 220-470 uF bulk capacitor if board space permits.

## Placement Checks

- [ ] Place C11 immediately beside U3's `VM` pin. Minimize the complete `VM -> capacitor -> GND` loop, not just the positive trace.
- [ ] Place C14 immediately beside U4's `VM` pin with the same short-loop treatment.
- [ ] Place C12 and C15 immediately beside their corresponding `VCC` pins.
- [ ] Place C10 close behind C11 and C13 close behind C14; the 100 nF parts get first priority at the IC pins.
- [ ] Place C16 beside the motor-driver `VBAT_SW` supply entry or between clustered drivers, with short access to both VM branches.
- [ ] Keep every capacitor fully inside `Edge.Cuts` and clear of courtyards, chassis features, and connector bodies.
- [ ] If U3 and U4 are placed far apart, reconsider one remote shared bulk capacitor. Options are a central 220 uF capacitor plus local ceramics, or separate 100-220 uF bulk capacitors near the two drivers.
- [ ] Keep each driver close enough to its motor connector to avoid a long high-current output loop, unless a central driver cluster produces a demonstrably better power/ground layout.
- [ ] Keep the motor-power group away from USB, the crystal, reset, BATLEV, encoder A/B, and other noise-sensitive routing.

## Routing And Ground Checks

- [ ] Route battery contacts through F1, Q1, Q2, and `VBAT_SW` to the motor-driver supply using short, wide copper or pours sized for simultaneous motor current.
- [ ] Give C16 a wide connection to `VBAT_SW` and a short, wide return to the motor ground path; do not connect it through a narrow branch trace.
- [ ] Route each VM branch through its local capacitor area before reaching the driver pin so the bypass capacitor is electrically local.
- [ ] Use a solid, low-impedance ground return under and around U3/U4 and their capacitors.
- [ ] Connect each DRV8838 exposed pad to GND with appropriate thermal copper and vias.
- [ ] Keep left- and right-motor current returns out of MCU, USB, encoder, and ADC return paths.
- [ ] Minimize motor-output loop area and use adequate copper width for start and stall current.
- [ ] Provide accessible `VBAT_SW` and GND probe points at the driver cluster; remote test points do not show the full local transient.

## Bulk-Capacitance Decision

The initial Rev A population is one shared 220 uF C16 plus 10 uF and 100 nF local capacitors per driver. This is a reasonable starting point, not a proof that the rail is adequately controlled.

- [ ] Do not increase capacitance solely from the motor stall-current number; sustained current must come from the battery and copper path.
- [ ] Increase or distribute bulk capacitance if testing shows excessive local droop, ringing, or regenerative rise.
- [ ] If increasing C16 to 330-470 uF or fitting a second capacitor, recheck power-on inrush through the battery contacts, F1, Q1, and Q2.
- [ ] Remember that bulk capacitance cannot absorb unlimited braking energy. Add or revise a clamp strategy if regenerative peaks approach the DRV8838 operating limit.

## Bring-Up Measurements

Use a short probe ground spring at the driver-cluster `VBAT_SW` and GND points. Long oscilloscope ground leads can invent ringing that is not present on the PCB.

- [ ] Start with a current-limited bench supply and one motor at low duty cycle.
- [ ] Capture local `VBAT_SW` during one-motor start and stop.
- [ ] Repeat with both motors starting simultaneously.
- [ ] Capture direction reversal, commanded braking/decay behavior, stall release, and power-off.
- [ ] Repeat at approximately 7.2 V nominal and 8.4 V freshly charged NiMH voltage.
- [ ] Verify local VM droop does not cause erratic motor operation, encoder-power loss, buck-regulator dropout, or MCU reset.
- [ ] Verify positive spikes remain below the DRV8838 recommended operating limit with margin; do not use the absolute-maximum rating as a normal target.
- [ ] Check C16 temperature and signs of excessive ripple stress during extended two-motor operation.
- [ ] Decide from measured waveforms whether to populate the optional bulk capacitor, increase C16, distribute bulk capacitance, or add a rail clamp in a later revision.

## References

- [TI DRV8838 datasheet](https://www.ti.com/lit/ds/symlink/drv8838.pdf), especially bulk capacitance and layout guidance.
- [Pololu Romi 32U4 schematic](https://www.pololu.com/file/0J1258/romi-32u4-control-board-schematic-diagram.pdf), including the 220 uF VSW reference.
- [Pololu #1520 motor](https://www.pololu.com/product/1520), including the 1.25 A stall-current reference at 4.5 V.
- [Samsung CL21A106KAYNNNE product data](https://product.samsungsem.com/mlcc/CL21A106KAYNNN.do), including X5R and DC-bias characteristics.
- [Rev A motor-driver spec](rev-a-motor-driver-spec.md).
- [Rev A power spec](rev-a-power-spec.md).
