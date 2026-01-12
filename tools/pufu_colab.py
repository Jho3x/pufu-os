
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

import threading

def stream_reader(pipe, name):
    try:
        with pipe:
            for line in iter(pipe.readline, b''):
                print(line.decode().strip())
    except Exception:
        pass

def run_os():
    print("Launching Pufu OS (Background)...")
    # Must pass the bootloader script
    # We use PIPE to capture output and provide input
    proc = subprocess.Popen(
        ["./bin/pufu_os", "src/userspace/boot/bootloader.pufu"],
        stdin=subprocess.PIPE,  # Enable Stdin
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        bufsize=0 # Unbuffered
    )
    
    # Start threads to print output
    t_out = threading.Thread(target=stream_reader, args=(proc.stdout, "OUT"))
    t_out.daemon = True
    t_out.start()
    
    t_err = threading.Thread(target=stream_reader, args=(proc.stderr, "ERR"))
    t_err.daemon = True
    t_err.start()
    
    return proc

def input_loop(proc):
    """
    Reads from Colab input() and sends to Pufu OS stdin.
    This blocks the main thread, but output threads keep running.
    """
    print("\n[Interactive Mode Enabled] Type commands below and press Enter.")
    print("To exit the input loop (but keep Pufu running), type 'CTRL+C' or interrupt kernel.\n")
    try:
        while proc.poll() is None:
            # In Colab, this creates an input box
            try:
                # We use a non-blocking check if possible, but input() is blocking.
                # Just loop input.
                cmd = input() 
                if cmd:
                    # Send input + newline to Pufu
                    msg = cmd + "\n"
                    proc.stdin.write(msg.encode())
                    proc.stdin.flush()
            except EOFError:
                break
    except KeyboardInterrupt:
        print("\n[Input Loop Stopped] Pufu OS continues in background...")

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
    print("\nTimeout waiting for server (Is the port blocked?).")
    return False

def start_ngrok(port, proc, auth_token):
    # Set the token programmatically
    ngrok.set_auth_token(auth_token)

    # Open a TCP tunnel to the web server
    # Note: HTTP tunnel is better for browser viewing, but TCP is fine if using raw socket.
    # Actually for web backend we want http.
    public_url = ngrok.connect(port, "http").public_url
    print(f"\n========================================")
    print(f"PUFU OS PUBLIC CLOUD ACCESS (NGROK)")
    print(f"========================================")
    print(f"Click here to access Pufu OS: {public_url}")
    print(f"========================================\n")

    try:
        # Keep the tunnel alive and monitor process
        while True:
            time.sleep(1)
            if proc.poll() is not None:
                print(f"\n[Monitor] Pufu OS crashed with exit code {proc.returncode}")
                # Print stderr if we can capture it (already streaming to console)
                break
    except KeyboardInterrupt:
        print("Shutting down Ngrok...")
    finally:
        ngrok.kill()

def network_manager(proc, auth_token):
    # Wait for the service to be ready
    if wait_for_server(8081, proc):
        # Use Ngrok with pre-collected token
        start_ngrok(8081, proc, auth_token)
    else:
        print("Error: Pufu OS Web Backend failed to start.")

def main():
    if not os.path.exists("Makefile"):
        print("Error: Run this from the repository root.")
        return
        
    # 1. Ask for Ngrok Token FIRST to avoid input conflicts later
    print("=== Ngrok Configuration ===")
    print("Ngrok requires a free Authtoken to be reliable.")
    print("Get one here: https://dashboard.ngrok.com/get-started/your-authtoken")
    auth_token = input("Enter your Ngrok Authtoken: ").strip()
    if not auth_token:
        print("Error: Token required.")
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
    
    # Start Network Manager in Background (Pass Token)
    t_net = threading.Thread(target=network_manager, args=(proc, auth_token))
    t_net.daemon = True
    t_net.start()
    
    # Run Interactive Input in Main Thread
    input_loop(proc)
    
    # Cleanup
    if proc.poll() is None:
        proc.terminate()

if __name__ == "__main__":
    main()
