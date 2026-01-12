
"""
Pufu OS - Google Colab Launcher
Running this script in a Colab cell will:
1. Compile the OS.
2. Launch the Web Backend.
3. Expose the Viewport via LocalTunnel (Public URL).
"""

import os
import subprocess
import time
import sys
import socket

def install_deps():
    print("Installing dependencies...")

def compile_os():
    print("Compiling Pufu OS...")
    ret = subprocess.run(["make", "clean"])
    ret = subprocess.run(["make", "all"])
    if ret.returncode != 0:
        print("Compilation Failed!")
        sys.exit(1)
    print("Compilation Success.")

def run_os():
    print("Launching Pufu OS (Background)...")
    # Must pass the bootloader script
    return subprocess.Popen(["./bin/pufu_os", "src/userspace/boot/bootloader.pufu"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def wait_for_server(port, timeout=30):
    print(f"Waiting for Pufu OS Web Backend on port {port}...")
    start_time = time.time()
    while time.time() - start_time < timeout:
        try:
            with socket.create_connection(("localhost", port), timeout=1):
                print("Server Ready!")
                return True
        except (socket.timeout, ConnectionRefusedError):
            time.sleep(0.5)
    print("\nTimeout waiting for server.")
    return False

def start_localtunnel(port):
    print("\n" + "="*40)
    print("PUFU OS PUBLIC CLOUD ACCESS")
    print("="*40)
    
    # 1. Install localtunnel
    if subprocess.run(["which", "lt"], stdout=subprocess.DEVNULL).returncode != 0:
        print("Installing LocalTunnel (npm)...")
        # Ensure npm is installed (Colab usually has it)
        subprocess.run(["npm", "install", "-g", "localtunnel"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    # 2. Get Public IP for Password (Tunnel verification)
    try:
        ip = subprocess.check_output(["curl", "-s", "ipv4.icanhazip.com"]).decode().strip()
        print(f"** IMPORTANT **: The Password is: {ip}")
    except:
        print("Could not fetch IP (Password).")

    # 3. Start Tunnel
    print("Starting Tunnel...")
    # lt runs in foreground, so we background it
    lt_proc = subprocess.Popen(["lt", "--port", str(port)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    # Read the URL from stdout (it prints "your url is: ...")
    import re
    while True:
        line = lt_proc.stdout.readline().decode()
        if "your url is" in line:
            url = line.split("is:")[1].strip()
            print(f"Click here to access Pufu OS: {url}")
            print(f"(Don't forget the password: {ip})")
            print("="*40 + "\n")
            break
        if not line and lt_proc.poll() is not None:
            print("LocalTunnel failed to start.")
            break

def main():
    if not os.path.exists("Makefile"):
        print("Error: Run this from the repository root.")
        return

    # Cleanup previous instances
    print("Cleaning up old processes...")
    subprocess.run(["pkill", "-9", "pufu_os"])
    time.sleep(1)

    compile_os()
    
    # Force GUI mode for Cloud
    print("Configuring for Cloud (GUI Mode)...")
    subprocess.run(["sed", "-i", "s/mode: cli/mode: gui/g", "src/userspace/boot/user_config.pufu"])
    
    proc = run_os()
    
    # Wait for the service to be ready
    if not wait_for_server(8080):
        print("Error: Pufu OS Web Backend failed to start.")
        if proc.stderr:
            print(proc.stderr.read().decode())
        proc.terminate()
        return

    # Use LocalTunnel instead of Colab Proxy
    start_localtunnel(8080)
    
    try:
        while True:
            time.sleep(1)
            if proc.poll() is not None:
                print("Pufu OS stopped.")
                break
    except KeyboardInterrupt:
        print("Stopping...")
        proc.terminate()

if __name__ == "__main__":
    main()
