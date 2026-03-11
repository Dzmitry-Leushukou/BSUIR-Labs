import socket

def big_request():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('127.0.0.1', 8080))
    big_data = 'A' * 5000 + '\n'
    s.send(big_data.encode())
    response = s.recv(1024)
    print("[+] Server response:", response.decode())
    s.close()

if __name__ == "__main__":
    big_request()