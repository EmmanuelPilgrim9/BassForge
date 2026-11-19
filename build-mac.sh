#!/bin/bash
# Compila BassForge (VST3 + AU) en macOS. Requiere Xcode, CMake y Git.
set -e
cd "$(dirname "$0")"
cmake -B build -G Xcode
cmake --build build --config Release --target BassForge_VST3 BassForge_AU
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R build/BassForge_artefacts/Release/VST3/BassForge.vst3 ~/Library/Audio/Plug-Ins/VST3/
echo "Listo: BassForge.vst3 copiado a ~/Library/Audio/Plug-Ins/VST3/"
