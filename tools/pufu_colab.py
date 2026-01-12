
"""
Pufu OS - Google Colab Launcher
Running this script in a Colab cell will:
1. Compile the OS.
2. Launch the Web Backend.
3. Expose the Viewport via Colab Proxy.
"""

import os
import subprocess
import time
import sys
from threading import Thread

def install_deps():
    print("Installing dependencies...")
    # Colab usually has build-essential. We might need X11 headers for legacy files, 
    # but we removed them from Makefile. 
    # Just in case:
    # subprocess.run(["apt-get", "update"])
    # subprocess.run(["apt-get", "install", "-y", "build-essential"])

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

def bridge_port():
    try:
        from google.colab.output import eval_js
        print("\n" + "="*40)
        print("PUFU OS CLOUD VIEWPORT")
        print("="*40)
        proxy_url = eval_js("google.colab.kernel.proxyPort(8080)")
        print(f"Click here to view Pufu OS: {proxy_url}")
        print("="*40 + "\n")
    except ImportError:
        print("Not running in Google Colab? View at http://localhost:8080/")

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
    time.sleep(2) # Wait for boot
    
    if proc.poll() is not None:
        print("Pufu OS exited prematurely.")
        if proc.stderr:
            print(proc.stderr.read().decode())
        return

    bridge_port()
    
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
