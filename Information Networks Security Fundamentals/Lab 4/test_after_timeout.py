#!/usr/bin/env python3
import socket
import time

def idle_timeout():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('127.0.0.1', 8080))
    print("[+] Connected, now waiting >60 seconds...")
    time.sleep(65)  
    try:
        s.send(b'ping\n')
        print("[+] Data sent, waiting for response...")
        response = s.recv(1024)
        if response:
            print("[+] Received:", response.decode())
        else:
            print("[-] Server closed the connection (recv returned 0)")
    except Exception as e:
        print("[-] Connection error:", e)
    finally:
        s.close()

if __name__ == "__main__":
    idle_timeout()