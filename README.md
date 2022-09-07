# Esp32 serial flasher (FlashBox)

![flashbox overview](https://github.com/Machiel80/esp32-serial-flasher/blob/main/flashbox%20design/images/overview%20filled.jpg?raw=true)<br/>

## Overview
The FlashBox is an Esp32 (host) to Esp32 (target) serial flasher. 

This concept is especially useful when you need to produce a small quantity of PCB with embedded ESP32 software. The process of designing 
(for example with EasyEDA.com) and producing (for example from jlcpcb.com) PCB’s is relative straightforward. But in small quantities 
(less than 1000 pcs) it’s made without your embedded software on the PCB. It's up to you how to get your software into the ESP32 chip.<br/>
<br/>
This FashBox project is meant to making the flashing process straightforward as well. It makes the flash job quick, easy to execute (press the button), 
reliable (green LED will burn when software is verified) and finally the process scalable. Scalable, a FlashBox can operate independently from a computer and is cheap to build, a rough 
estimation of the out-of-pocket cost is around 30 dollars [^1]. That makes the FlashBox scalable, build more of them, when you want to outsource the flashing process.

## Technical
The FlashBox software is build on the 'Serial flasher' component from espressif. This component is a software library for flashing Espressif SoCs from other host microcontroller. 
Espressif SoCs are normally programmed via serial interface (UART). Port layer for given host microcontroller has to be implemented, if not available.

Your own ESP32 binary must be placed in the `firmware_target` directory and will be converted into C-array during the build process.

Following steps are performed in order to re-program target's memory:

1. Filesystem is initialized and mounted.
2. UART1 through which new binary will be transfered is initialized.
3. Host puts target device into boot mode tries to connect by calling `esp_loader_connect()`.
4. Binary file is opened and its size is acquired, as it has to be known before flashing.
5. Then `esp_loader_flash_start()` is called to enter flashing mode and erase amount of memory to be flashed.
6. `esp_loader_flash_write()` function is called repeatedly until the whole binary image is transfered.

[^1]: See 'flashbox design/components/flashbox component list.xlsx'

## Target firmware preparation in outline

1. Add a UART 'version' command to the target firmware that returns the current version.<br/>
   See 'sourcecode example target/blink_example.zip' for an implementation example.
2. Add the same version number from the previous step and change the TARGET_SOFTWARE_VERSION in 'main/main.h'
3. Compile the target firmware and place the .bin files in the 'firmware_target/bin' directory
4. Update the filenames in the 'firmware_target/firmware_target.c' file.<br/>
   The partition, boot, ota and application filenames should match with the names of your bin files ('firmware_target/bin' directory).<br/>
   Tip: look into the 'build/binaries.c' for the correct spelling of the names.<br/>
   Tip: nevert start a bin filename with a number
5. Compile and flash the FlashBox software

## Hardware connection
Table below shows connection between the FlashBox host and thet target ESP32 devices.

| ESP32 (host) | ESP32 (target)|
|:------------:|:-------------:|
|    IO35      |      TX0      |
|    IO32      |      IO0      |
|    IO33      |     RESET     |
|    IO25      |      RX0      |


## Installation history, dont's execute this is just for the record
mkdir components<br/>
cd components<br/>
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

## Hardware wire diagram
![flashbox wire diagram](https://github.com/Machiel80/esp32-serial-flasher/blob/main/flashbox%20design/doc/flashbox%20wire%20diagram%20with%20target.png?raw=true)<br/>
<br/>
![flashbox wire diagram with dev board](https://github.com/Machiel80/esp32-serial-flasher/blob/main/flashbox%20design/doc/flashbox%20wire%20diagram.png?raw=true)<br/>
<br/>
