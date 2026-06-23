# Embedded Systems Self Learning

My motivation to build this repo was to explore on personal technical development and something I keep seeing in my consulting work. The challenge of fundamental grasping analog and industrial systems to a software defined networking stack. 

So my intent is to explore any particular analog/sensor and consume across the low level stacks I'm aware of and have note really gone self taught in at a computer architecture and operating system /dev/ level.



Im using components from the Elegoo Mega Complete Starter Kit as my initial starting point, but first i need to understand the core kernels to the operating system I would analyze operating systems.


## Folder Structure Of System Embedding

```bash
.
├── README.md
├── arduino
│   ├── README.md
│   └── servo_ultrasonic_beacon.ino
└── stm32
    ├── c
    ├── go
    ├── odin
    ├── rust
    └── zig
```


## Circuit Board Research And Specs

| Board | Core Specs | Core Elements / Learning | Design Considerations |
| :----- | :---------- | :----------------------- | :-------------------- |
| Arduino MEGA2560 R3 | **MCU**: ATmega2560, 8-bit AVR. **Clock**: 16 MHz. **Memory**: 256 KB flash, 8 KB SRAM, 4 KB EEPROM. **Logic**: 5 V. **I/O**: 54 digital pins, 15 PWM outputs, 16 analog inputs, 4 UARTs, SPI, I2C. | Good first board for GPIO, PWM, ADC reads, serial protocols, timers, interrupts, and translating sensor/actuator behavior into embedded control loops. The large pin count makes it useful for multi-sensor experiments from the Elegoo kit. | 5 V logic is convenient for starter-kit parts but needs level shifting for many modern 3.3 V modules. Limited SRAM means data buffers, strings, and logging need discipline. No built-in debugger, so learning often starts with serial output and simple test sketches. |
| STM32 Nucleo | **MCU Family**: STM32 ARM Cortex-M boards, commonly Cortex-M0/M3/M4/M7 depending on model. **Example**: NUCLEO-F401RE uses a 32-bit Cortex-M4F at up to 84 MHz with 512 KB flash and 96 KB SRAM. **Logic**: 3.3 V. **I/O**: Arduino Uno R3-style headers, ST morpho headers, ADC, timers, UART, SPI, I2C, and on-board ST-LINK debug/programming. | Good next step for moving from Arduino-style sketches to register-level work, HAL/LL drivers, linker scripts, startup code, interrupts, DMA, clock trees, RTOS concepts, and cross-language embedded experiments in C, Rust, Zig, Odin, or Go. | Pick the exact Nucleo model intentionally because memory, clock speed, peripherals, and pin mappings vary across the family. 3.3 V logic requires checking sensor voltage compatibility. The richer clock, DMA, and interrupt model gives more control but demands stronger attention to datasheets, reference manuals, and debugger-driven validation. |


## Operating Systems


| Operating System | Kernel Type  | Description                                                                                                 | Key Characteristics                                                                                                                                                                                                                    | Architecture And Device I/O                                                                                                                                                                                                                                                                                                                                                                          |
| :---------------- | :------------ | :----------------------------------------------------------------------------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| macOS            | Hybrid (XNU) | Combines elements from the Mach microkernel and components from FreeBSD.                                    | **Kernel**: XNU integrates Mach and FreeBSD components. **Userland**: Unix-like environment with BSD userland tools. **Compatibility**: Supports Intel and Apple silicon architectures.                                                | **Architecture**: Apple silicon uses ARM64/RISC style architecture; older Macs use Intel x86_64/CISC. **Cameras**: Usually accessed through AVFoundation, not stable `/dev/video*` paths. **Drives**: Disk devices appear as `/dev/disk*`; mounted volumes appear under `/Volumes/VolumeName`.                                                                                                       |
| Linux            | Monolithic   | A single large kernel that manages all system resources and services.                                       | **Kernel**: Monolithic, allowing efficient performance and direct hardware access. **Flexibility**: Highly customizable across servers, desktops, and embedded systems. **Community**: Open-source with a large development community. | **Architecture**: Common on ARM/RISC boards and x86/x86_64 CISC machines. **Cameras**: Usually exposed through V4L2 as `/dev/video*`. **Drives**: Common device paths include `/dev/sd*`, `/dev/mmcblk*`, and `/dev/nvme*`; mounts often live under `/mnt`, `/media`, or project-specific paths. **Board I/O**: GPIO, I2C, and SPI can appear as `/dev/gpiochip*`, `/dev/i2c-*`, and `/dev/spidev*`. |
| BSD              | Monolithic   | Similar to Linux, BSD uses a traditional monolithic kernel architecture.                                    | **Kernel**: Monolithic, similar to Linux but with a focus on stability and security. **Licensing**: BSD licenses are permissive and allow proprietary use. **Variants**: Includes FreeBSD, OpenBSD, and NetBSD.                        | **Architecture**: Runs on x86/x86_64 CISC and several ARM/RISC platforms depending on the BSD variant. **Cameras**: Device support varies; FreeBSD can expose cameras through webcamd as `/dev/video*`. **Drives**: Common device paths include `/dev/ada*`, `/dev/da*`, and `/dev/nvd*`; mounts are commonly placed under `/mnt` or configured mount points.                                        |
| Windows          | Hybrid       | Utilizes a hybrid kernel design, incorporating features from both monolithic and microkernel architectures. | **Kernel**: Hybrid, designed to balance performance and modularity. **User Interface**: Graphical interface with a broad software ecosystem. **Market Share**: Dominates the desktop market, particularly in business environments.    | **Architecture**: Most common on x86/x86_64 CISC machines, with ARM64 support on some devices. **Cameras**: Accessed through the Windows driver stack and media APIs rather than simple `/dev` paths. **Drives**: Exposed as drive letters like `C:\` or volume paths; hardware details are handled by the driver and device manager layers.                                                         |
