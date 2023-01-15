# Usage

```
cd rp2040_makefile
git clone https://github.com/raspberrypi/pico-sdk/
git clone https://github.com/hathach/tinyusb
nano app.c # Add something that has main() --- blink.c or hello_usb.c from pico-examples work here
make -j8
```

Devs of rp2040 [discourage using make](https://forums.raspberrypi.com/viewtopic.php?t=316775) but I have an excuse :joy_cat: --- pico-sdk cmake scripts are somewhat broken on MSYS2.
