import serial
import time

PORT = "COM8"
BAUD = 9600

def main():
    ser = serial.Serial(PORT, BAUD, timeout=1)
    time.sleep(2)

    print("连接成功，输入指令 rs [-s] <action> / set <id> <deg>：")

    while True:
        try:
            cmd = input(">>> ").strip()
            if cmd == "exit":
                break
            ser.reset_input_buffer()
            ser.write((cmd + "\n").encode())
            start_time = time.time()
            while True:
                if ser.in_waiting:
                    line = ser.readline().decode().strip()
                    if line:
                        print("[Arduino] " + line)
                        break
                if time.time() - start_time > 2:
                    print("[ERROR] Arduino 无响应")
                    break
        except KeyboardInterrupt:
            break
    ser.close()
    print("连接已断开")

if __name__ == "__main__":
    main()
