# Firmware

Active firmware work for the STM32 MCU on the Rev A motor-control board.

The firmware is bare-metal C11 for the `STM32F103CBT6`. It uses CMSIS register
definitions but no STM32 HAL, LL framework, RTOS, heap, or generated runtime
code. [`roamer_rev_a.ioc`](roamer_rev_a.ioc) records the planned pin and clock
assignment; it is a design artifact rather than the firmware generator.

## Build

Requirements:

- Arm GNU Toolchain (`arm-none-eabi-gcc`, `objcopy`, and `size`)
- CMake 3.22 or newer
- Ninja

From the repository root:

```sh
cmake -S firmware -B firmware/build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build firmware/build-debug

cmake -S firmware -B firmware/build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build firmware/build-release
```

Each build produces `roamer_rev_a.elf`, `.hex`, `.bin`, and `.map` files and
prints Flash/RAM use.

## Program and debug

SWD is the primary programming and recovery interface. The board has a
1.27 mm MIPI10/Arm Cortex debug header; an STLINK-V3SET includes the matching
cable. The target must supply its own 3.3 V power. Keep the motors disconnected
during initial bring-up.

With `STM32_Programmer_CLI` on `PATH` and an ST-LINK attached:

```sh
STM32_Programmer_CLI -c port=SWD -w firmware/build-release/roamer_rev_a.hex -v -rst
```

The initial image keeps both DRV8838 drivers asleep and both PWM inputs low.
PC13 blinks with a one-second cycle after the 16 MHz HSE starts and the system
switches to 48 MHz. A rapid blink indicates that firmware remained on the
8 MHz internal oscillator because HSE startup failed. USB is enabled only when
the 48 MHz clock is available.

## USB heartbeat bring-up

Rev A is self-powered and uses USB only for data. Power the board from its
battery or a current-limited bench supply, connect it to an already-powered
host, and then look for the CDC ACM serial device. USB alone does not power the
board.

On macOS, find and open the device with:

```sh
ls /dev/cu.usbmodem*
screen /dev/cu.usbmodem* 115200
```

The baud-rate setting is accepted for CDC compatibility but does not control a
physical UART. While the serial port is open, the firmware sends one line per
second:

```text
roamer heartbeat
```

Close `screen` with `Ctrl-A`, then `\`. The PC13 LED continues blinking while
USB is connected. Motor commands and received serial data are not supported by
this bring-up image; received bytes are discarded and both drivers remain
asleep.

The USB identity is development-only: VID `0xCAFE`, PID `0x4001`, manufacturer
`Roamer`, and product `Roamer Rev A`. The serial string comes from the MCU's
96-bit unique ID. Replace the prototype VID/PID with assigned values before any
product distribution.

Rev A has a permanently enabled D+ pullup and no VBUS sensing. Firmware pulses
D+ low during startup to encourage clean re-enumeration after reset, but if the
host does not rediscover the device, unplug and reconnect the USB cable as
documented in the hardware policy.

## Verified Rev A pin contract

| Function | MCU pin | Peripheral/mode |
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
| SWDIO/SWCLK | PA13 / PA14 | Debug; SWO is not connected |

The CubeMX design artifact also assigns I2C2 SCL/SDA to PB10/PB11, but those two MCU pads are explicit no-connects in the Rev A schematic and PCB. There are no I2C pullups or external connection, so I2C2 is not physically usable on the current board. Route it before fabrication or remove the assignment and explicitly descope the Rev A expansion bus.

Initial firmware should support independent left/right motor commands, encoder counting, basic telemetry, battery reporting, and command-timeout safe stop.

Hardware-facing firmware requirements are tracked in the [Rev A hardware specification](../hardware/README.md#mcu-and-firmware-facing-pin-contract).

Accepted behavior and safety policies are tracked in [Rev A firmware design](DESIGN.md), including the six-cell NiMH motor-power baseline and 75% normal-duty ceiling.
