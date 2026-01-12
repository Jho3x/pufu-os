
"""
Pufu OS - Google Colab Launcher
Running this script in a Colab cell will:
1. Compile the OS.
2. Launch the Web Backend.
3. Expose the Viewport via NGROK (Reliable).
"""

import os
import subprocess
import time
import sys
import socket
import getpass

def install_deps():
    print("Installing dependencies...")
    subprocess.run(["pip", "install", "pyngrok"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

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
    # We remove stdout/stderr PIPE to allow logs to print to console directly
    # and prevent buffer deadlocks.
    return subprocess.Popen(["./bin/pufu_os", "src/userspace/boot/bootloader.pufu"])

def wait_for_server(port, proc, timeout=30):
    print(f"Waiting for Pufu OS Web Backend on port {port}...")
    start_time = time.time()
    while time.time() - start_time < timeout:
        # Check if process died
        if proc.poll() is not None:
            print(f"\n[!] Process died early with Exit Code: {proc.returncode}")
            return False
            
        try:
            with socket.create_connection(("localhost", port), timeout=1):
                print("Server Ready!")
                return True
        except (socket.timeout, ConnectionRefusedError):
            time.sleep(0.5)
    print("\nTimeout waiting for server.")
    return False

def start_ngrok(port, proc):
    print("\n" + "="*40)
    print("PUFU OS PUBLIC CLOUD ACCESS (NGROK)")
    print("="*40)
    
    # 1. Ask for Token
    print("Ngrok requires a free Authtoken to be reliable.")
    print("Get one here: https://dashboard.ngrok.com/get-started/your-authtoken")
    token = getpass.getpass("Enter your Ngrok Authtoken: ")
    
    if not token:
        print("No token provided. Cannot start tunnel.")
        return

    # 2. Setup Ngrok
    from pyngrok import ngrok, conf
    conf.get_default().auth_token = token
    
    # 3. Connect
    print("Starting Tunnel...")
    try:
        public_url = ngrok.connect(port).public_url
        print(f"\nClick here to access Pufu OS: {public_url}")
        print("="*40 + "\n")
        
        # Keep alive & Monitor Process
        while True:
            time.sleep(1)
            if proc.poll() is not None:
                print("\n[!] Pufu OS has stopped unexpectedly!")
                print("Exit Code:", proc.returncode)
                if proc.stderr:
                    print("--- STDERR ---")
                    print(proc.stderr.read().decode())
                    print("--------------")
                break
    except Exception as e:
        print(f"Ngrok Error: {e}")
    finally:
        ngrok.kill()

def main():
    if not os.path.exists("Makefile"):
        print("Error: Run this from the repository root.")
        return

    install_deps()

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
    if not wait_for_server(8081, proc):
        print("Error: Pufu OS Web Backend failed to start (Timeout or Crash).")
        proc.terminate()
        return

    # Use Ngrok
    start_ngrok(8081, proc)
    
    # Cleanup (Redundant if loop breaks, but safe)
    if proc.poll() is None:
        proc.terminate()

if __name__ == "__main__":
    main()
