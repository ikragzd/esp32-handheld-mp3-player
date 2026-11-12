# DIY Handheld MP3 Player

A simple, self-contained handheld MP3 player built as a personal embedded electronics hobby project. It plays audio files from an SD card while rendering 64x64 pixel-art animations and time-synced lyrics on a miniature OLED screen.

## Hardware Used
* **Microcontroller:** ESP32-C3 Super Mini
* **Audio Module:** DFPlayer Mini MP3 decoder and 3.5mm headphone jack
* **Display:** 0.96-inch SSD1306 OLED display over I2C
* **Controls:** Tactile pushbuttons for playback navigation

## Features
* Standard MP3 playback controlled via UART commands
* Custom 64x64 pixel-art animations rendered using U8g2lib
* Synchronized lyric display timed with audio playback
