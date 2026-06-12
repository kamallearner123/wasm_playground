#!/bin/bash

set -e

echo "===================================="
echo " Installing Emscripten SDK on RHEL9 "
echo "===================================="

# Install prerequisites
echo "[1/5] Installing prerequisites..."
sudo dnf install -y git python3 cmake make gcc gcc-c++

# Clone EMSDK if not already present
echo "[2/5] Downloading EMSDK..."
if [ ! -d "$HOME/emsdk" ]; then
    git clone https://github.com/emscripten-core/emsdk.git "$HOME/emsdk"
else
    echo "EMSDK already exists. Skipping clone."
fi

cd "$HOME/emsdk"

# Install latest SDK
echo "[3/5] Installing latest Emscripten SDK..."
./emsdk install latest

# Activate SDK
echo "[4/5] Activating SDK..."
./emsdk activate latest

# Add environment setup to .bashrc if not already present
echo "[5/5] Configuring environment..."
grep -qxF 'source ~/emsdk/emsdk_env.sh' ~/.bashrc || \
echo 'source ~/emsdk/emsdk_env.sh' >> ~/.bashrc

# Load environment for current shell
source ./emsdk_env.sh

echo ""
echo "===================================="
echo " Installation Complete"
echo "===================================="
echo ""

echo "Emscripten Version:"
emcc --version

echo ""
echo "Run the following command in new terminals:"
echo "source ~/emsdk/emsdk_env.sh"
