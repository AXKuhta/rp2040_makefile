from time import time
import usb1

import matplotlib.pyplot as plt
import numpy as np

#
# We operate with a baseband of:
# 25 MHz / 4 / 40 = 156.25 kHz
#
# Passband is ±0.4
# So, ±62.5 kHz
#

# Radiate for 5 seconds
t = np.linspace(0, 5, 156250 * 5)

# Seem to be limited to ±8 kHz in reality
# Add more subcarriers as you wish,
# but be sure to scale accordingly
x = np.exp(1j * 2 * np.pi * t * -8*1000)

#plt.plot(x.real)
#plt.plot(x.imag)
#plt.show()

# Format as offset binary, bit depth of 4
s = 8*x + 8 + 8j
u = np.uint16(s.real)
v = np.uint16(s.imag)

u = np.clip(u, 0, 15)
v = np.clip(v, 0, 15)

#plt.step(t, u)
#plt.step(t, v)
#plt.show()

sequence = bytes(k + (l<<4) for k, l in zip(u, v))

ctx = usb1.USBContext()
handle = None

print ("VID  PID")

for dev in ctx.getDeviceList():
	vid = dev.getVendorID()
	pid = dev.getProductID()
	print(f"{vid:04x} {pid:04x}")
	if vid == 0xCAFE and pid == 0x4011:
		print("Device found")
		handle = dev.open()

handle.claimInterface(2)

EP = 0x03

start = time()

handle.bulkWrite(EP, sequence, timeout=5100)

elapsed = time() - start
count = len(sequence)
rate = count/elapsed
kibi = rate/1024

print(f"{kibi:.1f}kB/s")
