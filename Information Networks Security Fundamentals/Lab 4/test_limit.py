import socket
import threading
import time
import signal
import sys

connections = []
lock = threading.Lock()
stop = False
total = 1100

def worker(i):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('127.0.0.1', 8080))
        with lock:
            connections.append(s)
            print(f"Connected {i+1}, total {len(connections)}")
        while not stop:
            time.sleep(1)
    except Exception as e:
        with lock:
            print(f"Failed {i+1}: {e}")

def signal_handler(sig, frame):
    global stop
    print("\nStopping...")
    stop = True

signal.signal(signal.SIGINT, signal_handler)

print(f"Starting {total} connections...")
threads = []
for i in range(total):
    t = threading.Thread(target=worker, args=(i,))
    t.start()
    threads.append(t)
    time.sleep(0.01) 

print("All threads started. Press Ctrl+C to release connections.")
while not stop:
    time.sleep(1)

for s in connections:
    s.close()
for t in threads:
    t.join()
print("Done.")