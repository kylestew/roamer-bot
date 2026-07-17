# CMSIS subset

Only the headers required by the STM32F103CBT6 firmware are vendored here.

- CMSIS-Core headers come from STM32CubeF1 `v1.8.7`.
- STM32F1 device headers come from `cmsis_device_f1` commit
  `c8e9a4a4f16b6d2cb2a2083cbe5161025280fb22`, the revision pinned by
  STM32CubeF1 `v1.8.7`.
- No STM32 HAL or LL sources are included.

The upstream Apache-2.0 license texts are retained alongside the files.
