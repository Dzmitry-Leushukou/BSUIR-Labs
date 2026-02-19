import os
import base64
import json
import time
import threading
import logging
from typing import Dict, Optional
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox

import requests
from cryptography.hazmat.primitives.ciphers.aead import AESGCM

def encrypt_aes_gcm(key: bytes, plaintext: bytes) -> bytes:
    aesgcm = AESGCM(key)
    nonce = os.urandom(12)
    ciphertext = aesgcm.encrypt(nonce, plaintext, None)
    return nonce + ciphertext

def decrypt_aes_gcm(key: bytes, encrypted_data: bytes) -> bytes:
    if len(encrypted_data) < 12:
        raise ValueError("Too short encrypted data")
    nonce = encrypted_data[:12]
    ciphertext = encrypted_data[12:]
    aesgcm = AESGCM(key)
    return aesgcm.decrypt(nonce, ciphertext, None)

def dict_to_bytes(d: Dict) -> bytes:
    return json.dumps(d).encode('utf-8')

def bytes_to_dict(b: bytes) -> Dict:
    return json.loads(b.decode('utf-8'))

def b64_encode(data: bytes) -> str:
    return base64.b64encode(data).decode('ascii')

def b64_decode(data: str) -> bytes:
    return base64.b64decode(data)

class KerberosClient:
    def __init__(self, log_callback):
        self.log_callback = log_callback
        self.kss_url = "http://localhost:8000"
        self.as_url = "http://localhost:8001"
        self.tgs_url = "http://localhost:8002"
        self.service_urls = {
            "fileserver": "http://localhost:8003",
            "printserver": "http://localhost:8004"
        }

        self.user_id: Optional[str] = None
        self.user_key: Optional[bytes] = None
        self.tgt: Optional[str] = None
        self.sk_tgs: Optional[bytes] = None
        self.service_ticket: Optional[str] = None
        self.sk_service: Optional[bytes] = None
        self.current_service: Optional[str] = None 

        self.key_storage_dir = "client_keys"
        os.makedirs(self.key_storage_dir, exist_ok=True)

    def log(self, message):
        self.log_callback(message)

    def _key_file_path(self, user_id: str) -> str:
        return os.path.join(self.key_storage_dir, f"{user_id}.key")

    def save_key(self, user_id: str, key_bytes: bytes):
        filepath = self._key_file_path(user_id)
        key_b64 = b64_encode(key_bytes)
        with open(filepath, 'w') as f:
            f.write(key_b64)
        self.log(f"Key saved to {filepath}")

    def load_key(self, user_id: str) -> Optional[bytes]:
        filepath = self._key_file_path(user_id)
        try:
            with open(filepath, 'r') as f:
                key_b64 = f.read().strip()
                key_bytes = b64_decode(key_b64)
                self.log(f"Key loaded from {filepath}")
                return key_bytes
        except FileNotFoundError:
            return None
        except Exception as e:
            self.log(f"Error loading key: {e}")
            return None

    def register(self, user_id: str):
        self.user_id = user_id
        self.log(f"Registering user {user_id}...")
        try:
            resp = requests.post(f"{self.kss_url}/register",
                                 json={"entity_id": user_id},
                                 timeout=5)
        except Exception as e:
            self.log(f"KSS connection error: {e}")
            return False

        if resp.status_code == 409:
            self.log("User already exists. Fetching existing key...")
            key = self.fetch_key(user_id)
            if key:
                self.user_key = key
                self.save_key(user_id, key)
                return True
            else:
                return False
        elif resp.status_code != 201:
            self.log(f"Registration failed: {resp.status_code} {resp.text}")
            return False

        data = resp.json()
        key_b64 = data.get("key")
        if not key_b64:
            self.log("Invalid response from KSS (no key)")
            return False
        self.user_key = b64_decode(key_b64)
        self.save_key(user_id, self.user_key)
        self.log(f"Registration successful. Key saved.")
        return True

    def fetch_key(self, user_id: str) -> Optional[bytes]:
        try:
            resp = requests.get(f"{self.kss_url}/keys/{user_id}", timeout=5)
        except Exception as e:
            self.log(f"KSS connection error: {e}")
            return None
        if resp.status_code != 200:
            self.log(f"Failed to fetch key: {resp.status_code}")
            return None
        data = resp.json()
        key_b64 = data.get("key")
        if not key_b64:
            self.log("Invalid response (no key)")
            return None
        return b64_decode(key_b64)

    def authenticate(self):
        if not self.user_id:
            self.log("User ID not set. Enter user_id first.")
            return False

        if not self.user_key:
            self.user_key = self.load_key(self.user_id)
            if not self.user_key:
                self.log("No local key found. Please register first.")
                return False

        self.log("Authenticating with AS...")
        try:
            resp = requests.post(f"{self.as_url}/auth",
                                 json={"user_id": self.user_id},
                                 timeout=5)
        except Exception as e:
            self.log(f"AS connection error: {e}")
            return False

        if resp.status_code != 200:
            self.log(f"Auth failed: {resp.status_code} {resp.text}")
            return False

        data = resp.json()
        enc_block_b64 = data.get("encrypted_block")
        tgt_b64 = data.get("tgt")
        if not enc_block_b64 or not tgt_b64:
            self.log("Invalid AS response")
            return False

        try:
            enc_block = b64_decode(enc_block_b64)
            block_bytes = decrypt_aes_gcm(self.user_key, enc_block)
            block = bytes_to_dict(block_bytes)
        except Exception as e:
            self.log(f"Failed to decrypt AS response: {e}")
            return False

        sk_tgs_b64 = block.get("session_key")
        if not sk_tgs_b64:
            self.log("AS response missing session_key")
            return False

        self.sk_tgs = b64_decode(sk_tgs_b64)
        self.tgt = tgt_b64
        self.log("Authentication successful. TGT and session key obtained.")
        return True

    def get_service_ticket(self, service_id: str):
        if not self.tgt or not self.sk_tgs:
            self.log("No TGT or session key. Authenticate first.")
            return False

        self.current_service = service_id
        self.log(f"Requesting service ticket for {service_id}...")
        auth_data = {
            "user_id": self.user_id,
            "timestamp": int(time.time())
        }
        auth_bytes = dict_to_bytes(auth_data)
        auth_enc = encrypt_aes_gcm(self.sk_tgs, auth_bytes)
        auth_b64 = b64_encode(auth_enc)

        payload = {
            "tgt": self.tgt,
            "service_id": service_id,
            "authenticator": auth_b64
        }

        try:
            resp = requests.post(f"{self.tgs_url}/ticket", json=payload, timeout=5)
        except Exception as e:
            self.log(f"TGS connection error: {e}")
            return False

        if resp.status_code != 200:
            self.log(f"TGS error: {resp.status_code} {resp.text}")
            return False

        data = resp.json()
        enc_block_b64 = data.get("encrypted_block")
        service_ticket_b64 = data.get("service_ticket")
        if not enc_block_b64 or not service_ticket_b64:
            self.log("Invalid TGS response")
            return False

        try:
            enc_block = b64_decode(enc_block_b64)
            block_bytes = decrypt_aes_gcm(self.sk_tgs, enc_block)
            block = bytes_to_dict(block_bytes)
        except Exception as e:
            self.log(f"Failed to decrypt TGS response: {e}")
            return False

        sk_service_b64 = block.get("session_key")
        if not sk_service_b64:
            self.log("TGS response missing session_key")
            return False

        self.sk_service = b64_decode(sk_service_b64)
        self.service_ticket = service_ticket_b64
        self.log("Service ticket obtained.")
        return True

    def access_resource(self, request_data: str):
        if not self.service_ticket or not self.sk_service:
            self.log("No service ticket. Get ticket first.")
            return False

        if not self.current_service:
            self.log("No service selected.")
            return False

        service_url = self.service_urls.get(self.current_service)
        if not service_url:
            self.log(f"Unknown service: {self.current_service}")
            return False

        self.log(f"Accessing resource at {service_url}...")
        auth_data = {
            "user_id": self.user_id,
            "timestamp": int(time.time())
        }
        auth_bytes = dict_to_bytes(auth_data)
        auth_enc = encrypt_aes_gcm(self.sk_service, auth_bytes)
        auth_b64 = b64_encode(auth_enc)

        payload = {
            "ticket": self.service_ticket,
            "authenticator": auth_b64,
            "request_data": request_data
        }

        try:
            resp = requests.post(f"{service_url}/resource", json=payload, timeout=5)
        except Exception as e:
            self.log(f"SS connection error: {e}")
            return False

        if resp.status_code != 200:
            self.log(f"SS error: {resp.status_code} {resp.text}")
            return False

        data = resp.json()
        response_text = data.get("response", "No response")
        self.log(f"Server response: {response_text}")
        return True


class KerberosGUI:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Kerberos Client")
        self.root.geometry("700x600")

        self.client = KerberosClient(self.log)

        self.user_id_var = tk.StringVar()
        self.service_var = tk.StringVar(value="fileserver")
        self.request_var = tk.StringVar(value="Hello, server!")

        self.create_widgets()

    def create_widgets(self):
        frame_top = ttk.LabelFrame(self.root, text="User", padding=10)
        frame_top.pack(fill=tk.X, padx=10, pady=5)

        ttk.Label(frame_top, text="User ID:").grid(row=0, column=0, sticky=tk.W)
        ttk.Entry(frame_top, textvariable=self.user_id_var, width=20).grid(row=0, column=1, padx=5)
        ttk.Button(frame_top, text="Register", command=self.register).grid(row=0, column=2, padx=5)
        ttk.Button(frame_top, text="Authenticate", command=self.authenticate).grid(row=0, column=3, padx=5)

        frame_service = ttk.LabelFrame(self.root, text="Service Ticket", padding=10)
        frame_service.pack(fill=tk.X, padx=10, pady=5)

        ttk.Label(frame_service, text="Service ID:").grid(row=0, column=0, sticky=tk.W)
        ttk.Combobox(frame_service, textvariable=self.service_var,
                     values=["fileserver", "printserver"], width=15).grid(row=0, column=1, padx=5)
        ttk.Button(frame_service, text="Get Ticket", command=self.get_ticket).grid(row=0, column=2, padx=5)

        frame_access = ttk.LabelFrame(self.root, text="Access Resource", padding=10)
        frame_access.pack(fill=tk.X, padx=10, pady=5)

        ttk.Label(frame_access, text="Request data:").grid(row=0, column=0, sticky=tk.W)
        ttk.Entry(frame_access, textvariable=self.request_var, width=30).grid(row=0, column=1, padx=5)
        ttk.Button(frame_access, text="Access", command=self.access_resource).grid(row=0, column=2, padx=5)

        frame_log = ttk.LabelFrame(self.root, text="Log", padding=10)
        frame_log.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.log_text = scrolledtext.ScrolledText(frame_log, height=15, state=tk.DISABLED)
        self.log_text.pack(fill=tk.BOTH, expand=True)

        self.status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(self.root, textvariable=self.status_var, relief=tk.SUNKEN)
        status_bar.pack(side=tk.BOTTOM, fill=tk.X)

    def log(self, message):
        def update():
            self.log_text.config(state=tk.NORMAL)
            self.log_text.insert(tk.END, message + "\n")
            self.log_text.see(tk.END)
            self.log_text.config(state=tk.DISABLED)
        self.root.after(0, update)

    def set_status(self, text):
        self.root.after(0, lambda: self.status_var.set(text))

    def run_async(self, target, *args):
        thread = threading.Thread(target=target, args=args, daemon=True)
        thread.start()

    def register(self):
        user_id = self.user_id_var.get().strip()
        if not user_id:
            messagebox.showerror("Error", "Enter user ID")
            return
        self.set_status("Registering...")
        self.run_async(self._register_thread, user_id)

    def _register_thread(self, user_id):
        success = self.client.register(user_id)
        if success:
            self.log(f"User {user_id} ready.")
        self.set_status("Ready")

    def authenticate(self):
        if not self.client.user_id:
            self.client.user_id = self.user_id_var.get().strip()
        if not self.client.user_id:
            messagebox.showerror("Error", "Enter user ID")
            return
        self.set_status("Authenticating...")
        self.run_async(self._auth_thread)

    def _auth_thread(self):
        success = self.client.authenticate()
        if success:
            self.log("Authentication completed.")
        self.set_status("Ready")

    def get_ticket(self):
        if not self.client.user_id:
            messagebox.showerror("Error", "Authenticate first")
            return
        service = self.service_var.get().strip()
        if not service:
            messagebox.showerror("Error", "Enter service ID")
            return
        self.set_status("Getting ticket...")
        self.run_async(self._ticket_thread, service)

    def _ticket_thread(self, service):
        success = self.client.get_service_ticket(service)
        if success:
            self.log(f"Ticket for {service} obtained.")
        self.set_status("Ready")

    def access_resource(self):
        if not self.client.service_ticket:
            messagebox.showerror("Error", "Get service ticket first")
            return
        req_data = self.request_var.get().strip()
        self.set_status("Accessing resource...")
        self.run_async(self._access_thread, req_data)

    def _access_thread(self, req_data):
        success = self.client.access_resource(req_data)
        if success:
            self.log("Resource access completed.")
        self.set_status("Ready")

    def run(self):
        self.root.mainloop()


if __name__ == "__main__":
    gui = KerberosGUI()
    gui.run()