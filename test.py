import serial

ser = serial.Serial("COM3", 115200, timeout=1)

print("수신 시작")

while True:
    line = ser.readline().decode("utf-8").strip()
    if line:
        print(line)
