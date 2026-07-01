"""
Get them PLLs here.
"""

HSI = 16
M = 8
N = 168
P = 4
Q = 7

sysclk = (HSI / M) * N / P
usbclk = (HSI / M) * N / Q

print("HSI:", HSI)
print("M:", M)
print("N:", N)
print("P:", P)
print(f"sysclk = {sysclk}")
print(f"usbclk = {usbclk}")
