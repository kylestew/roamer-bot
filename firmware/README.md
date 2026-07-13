# Firmware

Active firmware work for the STM32 MCU on the Rev A motor-control board.

Initial firmware should support independent left/right motor commands, encoder counting, basic telemetry, battery reporting, and command-timeout safe stop.

Hardware-facing firmware requirements are tracked in [Rev A STM32 MCU requirements](../hardware/rev-a-stm32-mcu-requirements.md).

Accepted behavior and safety policies are tracked in [Rev A firmware design](DESIGN.md), including the six-cell NiMH motor-power baseline and 75% normal-duty ceiling.
