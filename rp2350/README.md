
```
git clone https://github.com/raspberrypi/pico-sdk --branch=2.2.0 --depth=1
git clone https://github.com/hathach/tinyusb --branch=0.18.0 --depth=1
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel --branch=V11.2.0 --depth=1

# Picolibc
# We can get away with not building it from source!
wget https://github.com/picolibc/picolibc/releases/download/1.8.5/picolibc-1.8.5-12.3.rel1.zip
unzip -p picolibc-1.8.5-12.3.rel1.zip arm-none-eabi/picolibc/arm-none-eabi/lib/release/thumb/v6-m/nofp/libc.a > libc.a
unzip -p picolibc-1.8.5-12.3.rel1.zip lib/gcc/arm-none-eabi/12.3.1/picolibc.specs > picolibc.specs
```
