## rp2040 usb network card

This has initially started as a project to build pico-sdk using Make instead of cmake but somehow evolved into a project to build a usb network adapter instead.

Key parts of it:
- TinyUSB RNDIS driver
- FreeRTOS-Plus-TCP
- Interface driver derived from the ESP32 driver (which may be the simplest interface driver in FreeRTOS-Plus-TCP)