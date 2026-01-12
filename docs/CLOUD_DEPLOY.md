# Pufu OS Cloud Deployment Guide

This guide explains how to run Pufu OS inside Google Colab using the new Web Backend.

## Quick Start (Google Colab)

1. **Open a new Notebook** in Google Colab.
2. **Clone the Repo** (Replace with your repo URL):
   ```python
   !git clone https://github.com/Jho3x/pufu-os.git
   %cd pufu-os
   ```
3. **Run the Launcher**:
   ```python
   !python3 tools/pufu_colab.py
   ```
4. **Click the Link**: The script will output a `proxyPort` URL. Click it to open the Pufu OS Desktop in your browser.

## How it Works
1. **Compilation**: The script compiles the OS on the Colab VM.
2. **Web Backend**: Pufu OS detects it's headless (or configured for GUI) and starts the HTTP Server on port 8080.
3. **Proxying**: Jupyter/Colab proxy forwarding exposes port 8080 as a public-facing URL (secured by your Google login).

## "Can the Agent see it?"
Yes, IF you use a public tunnel like `ngrok`.
1. Sign up for ngrok and get a token.
2. Run this in Colab instead of the proxy:
   ```python
   !pip install pyngrok
   from pyngrok import ngrok
   ngrok.set_auth_token("YOUR_TOKEN")
   public_url = ngrok.connect(8080)
   print(f"Agent Viewable URL: {public_url}")
   ```
3. Give that `public_url` to the Agent.
