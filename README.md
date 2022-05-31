# Raspberry Pi RP2040 MCU Vult DSP template

This C++ boilerplate template allows for creating Synthesizers, Effects and other Audio DSP related projects based on the cheap and easily available RP2040 MCU. It targets the "RP2040 Eurorack DSP Demo Board", which I have made.

The DSP board contains:

* RP2040 Dual-Core MCU (running at 270MHz)
* MIDI input (5-pin DIN)
* Stereo 3,5mm Audio Output (PCM5102 Audio Codec @44100Hz 32-bit)
* 4 Eurorack-Compatible Trigger Inputs
* 2 CV Inputs
* USB Type-C for MCU USB
* USB Type-C for UART Debug Bridges
* Mount for I2C 128x32 SSD1306 OLED
* Mount for PEC12R Encoder with Button
* User/Trigger Button

## Prerequisites

1. Install the  vult compiler: `sudo npm install vult -g`

2. If you have the pico-sdk already installed you can point to it by setting `PICO_SDK_PATH`. Otherwise CMake will download it for you.


## Building 
```
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make
cp rp2040-vult.uf2 /media/xyz/RP2_BOOT/
```
