# Esp32 serial flasher

## Overview

Esp32 (host) to Esp32 (target) serial flasher, how to flash the ESP32 target from this (host) MCU/ESP32 using esp_serial_flash component API.
Binaries to be flashed from host MCU to the target Espressif SoC can be found in `firmware_target` folder and are converted into C-array during build process.

Following steps are performed in order to re-program target's memory:

1. Filesystem is initialized and mounted.
2. UART1 through which new binary will be transfered is initialized.
3. Host puts target device into boot mode tries to connect by calling `esp_loader_connect()`.
4. Binary file is opened and its size is acquired, as it has to be known before flashing.
5. Then `esp_loader_flash_start()` is called to enter flashing mode and erase amount of memory to be flashed.
6. `esp_loader_flash_write()` function is called repeatedly until the whole binary image is transfered.

## Hardware connection

Table below shows connection between two ESP32 devices.

| ESP32 (host) | ESP32 (targer) |
|:------------:|:-------------:|
|    IO35      |      TX0      |
|    IO32      |      IO0      |
|    IO33      |     RESET     |
|    IO25      |      RX0      |


## Install
cd components
git clone https://github.com/espressif/esp-serial-flasher.git<br/>
git clone https://github.com/craftmetrics/esp32-button.git<br/>
git clone https://github.com/olikraus/u8g2.git<br/>
git clone https://github.com/mdvorak/esp-u8g2-hal.git<br/>
<br/>
change (/components/esp32-button/src/button.c) line 132 to 'debounce[idx].inverted = false;'<br/>
change (/components/esp-u8g2-hal/include/u8g2_esp32_hal.h) line 27 to 'I2C_MASTER_FREQ_HZ 250000'<br/>

## Inspiration

https://eprotutorials.com/esp32-gpio-tutorial/<br/>
https://github.com/espressif/esp-serial-flasher<br/>
http://www.lucadentella.it/en/2017/10/30/esp32-25-display-oled-con-u8g2<br/>
https://github.com/lucadentella/esp32-tutorial/tree/master/18_u8g2<br/>
https://github.com/mkfrey/u8g2-hal-esp-idf<br/>
https://github.com/mdvorak/esp-u8g2-hal<br/>
https://www.mischianti.org/images-to-byte-array-online-converter-cpp-arduino<br/>
https://convertio.co/bmp-xbm/<br/>
https://www.esp32.com/viewtopic.php?t=18656<br/>
https://www.youtube.com/watch?v=eLmpKKaQL54<br/>
https://randomnerdtutorials.com/solved-failed-to-connect-to-esp32-timed-out-waiting-for-packet-header<br/>

## IDF menuconfig

Partition Table -> CONFIG_PARTITION_TABLE_SINGLE_APP_LARGE=y

## Hardware

![flashbox pin layout](https://github.com/Machiel80/esp32-serial-flasher/blob/main/flashbox%20design/doc/flashbox%20pin-layout.png?raw=true)<br/>
<br/>
![flashbox wire diagram](https://github.com/Machiel80/esp32-serial-flasher/blob/main/flashbox%20design/doc/flashbox%20wire%20diagram.png?raw=true)<br/>
<br/>






