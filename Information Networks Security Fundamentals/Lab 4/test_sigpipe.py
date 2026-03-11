import socket
import time

def sigpipe_test():
    s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s1.connect(('127.0.0.1', 8080))
    s1.send(b'Hello\n')
    s1.close()
    print("[+] First connection closed abruptly. Server should handle SIGPIPE.")

    time.sleep(1)

    try:
        s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s2.connect(('127.0.0.1', 8080))
        s2.send(b'ping\n')
        response = s2.recv(1024)
        if response:
            print("[+] Server is alive! Response:", response.decode().strip())
        else:
            print("[-] Server closed the second connection immediately.")
        s2.close()
    except Exception as e:
        print("[-] Failed to connect to server after SIGPIPE test:", e)

if __name__ == "__main__":
    sigpipe_test()