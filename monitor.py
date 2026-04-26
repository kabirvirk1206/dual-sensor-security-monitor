# Runs on flipper zero - shows live data from esp32
# buttons let you change thresholds and arm/disarm the system without touching the esp32

import machine
import time


uart = machine.UART(1, baudrate=115200, timeout=100)


btn_up    = machine.Pin(7,  machine.Pin.IN, machine.Pin.PULL_UP)
btn_down  = machine.Pin(8,  machine.Pin.IN, machine.Pin.PULL_UP)
btn_left  = machine.Pin(5,  machine.Pin.IN, machine.Pin.PULL_UP)
btn_right = machine.Pin(6,  machine.Pin.IN, machine.Pin.PULL_UP)
btn_ok    = machine.Pin(44, machine.Pin.IN, machine.Pin.PULL_UP)

armed = True

def send(cmd):
   
    uart.write((cmd + "\n").encode())

print("up/down = distance +/-5cm")
print("left/right = sound +/-50")
print("ok = arm or disarm")

while True:
    # tp check if any button is pressed and send the right command accordingly 
    if not btn_up.value():
        send("D+"); time.sleep_ms(300)
    elif not btn_down.value():
        send("D-"); time.sleep_ms(300)
    elif not btn_right.value():
        send("S+"); time.sleep_ms(300)
    elif not btn_left.value():
        send("S-"); time.sleep_ms(300)
    elif not btn_ok.value():
        send("ARM" if not armed else "DIS")
        time.sleep_ms(400)

    # get incoming data (Used Claude for this portion)
    if uart.any():
        raw = uart.readline()
        if raw:
            try:
                p = raw.decode().strip().split(',')
                if len(p) == 7:
                    armed = p[4] == '1'
                    alert = "!! ALERT !!" if p[3] == '1' else "clear"
                    arm   = "armed" if armed else "disarmed"
                    # print it all on one line so it fits the flipper screen
                    print("[{}] {} d:{}<{}cm s:{}<{}".format(
                          arm, alert, p[0], p[5], p[1], p[6]))
            except:
                pass

    time.sleep_ms(50)
