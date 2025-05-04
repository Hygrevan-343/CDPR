import serial
import time
from pynput import keyboard

# Set your COM port and baud rate here
PORT = 'COM3'  # Change this to your Arduino port
BAUD = 9600

# Connect to Arduino
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)  # Wait for connection to stabilize


# Mapping keys to commands
key_map = {
    keyboard.Key.up: '\u001B[A',
    keyboard.Key.down: '\u001B[B',
    keyboard.Key.left: '\u001B[C',
    keyboard.Key.right: '\u001B[D',
    'u': 'U',
    'd': 'D'
}

def send_command(cmd):
    if ser.is_open:
        ser.write((cmd + '\n').encode())
        print(f">> Sent: {cmd}")

def on_press(key):
    try:
        if key.char.lower() == 'u':
            send_command('U')
        elif key.char.lower() == 'd':
            send_command('D')
        elif key.char in '123456789':
            delay = 1100 - (int(key.char) * 100)
            send_command(f"SPD-{delay}")
    except AttributeError:
        if key in key_map:
            send_command(key_map[key])

def on_release(key):
    send_command("")  # Send empty command to stop motion
    if key == keyboard.Key.esc:
        return False  # Stop listener

print("Ready. Use arrow keys, U/D, or 1–9 for speed. Press ESC to quit.")

with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
