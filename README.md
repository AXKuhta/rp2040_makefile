## rp2040 usb network card

This had initially started as a project to build pico-sdk using Make instead of cmake but somehow evolved into a project to build a usb network adapter instead.

Key parts of it:
- TinyUSB RNDIS driver
- FreeRTOS-Plus-TCP
- Interface driver derived from the ESP32 driver (which may be the simplest interface driver in FreeRTOS-Plus-TCP)

Here's how to build it:

```
# Source dependencies
git clone https://github.com/raspberrypi/pico-sdk --branch=1.5.1 --depth=1
git clone https://github.com/hathach/tinyusb --branch=0.15.0 --depth=1
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel --branch=V10.6.1 --depth=1
git clone https://github.com/FreeRTOS/FreeRTOS-Plus-TCP --branch=V4.0.0 --depth=1

# Picolibc
# We can get away with not building it from source!
wget https://github.com/picolibc/picolibc/releases/download/1.8.5/picolibc-1.8.5-12.3.rel1.zip
unzip -p picolibc-1.8.5-12.3.rel1.zip arm-none-eabi/picolibc/arm-none-eabi/lib/release/thumb/v6-m/nofp/libc.a > libc.a
unzip -p picolibc-1.8.5-12.3.rel1.zip lib/gcc/arm-none-eabi/12.3.1/picolibc.specs > picolibc.specs

make --file=Makefile_elf2uf2 # Build uf2 tool once
make --file=Makefile_bs2 # Build bootloader once
make -j4
```
