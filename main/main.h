// #define OTA_ROM
// #define CUSTOM_ROM

#ifdef CUSTOM_ROM
#define OTA_ROM
#define TARGET_NAME     "SafetyCook"
#define CUSTOM_ROM      safetycook_20220905_bin
#define CUSTOM_ROM_SIZE safetycook_20220905_bin_size
#else
#define TARGET_NAME "Esp32Blink"
#endif

#define FLASHER_SOFTWARE_VERSION  "20220907"
#define TARGET_SOFTWARE_VERSION   "20220906"

// Tip -> update (add empty line and save) /main/CMakeLists.txt to rebuild the .bin files into 'build/binaries.c'

#define HIGHER_BAUDRATE 230400

#define TARGET_RX_TX  35 // yellow wire
#define TARGET_IO0    32 // blue wire
#define TARGET_RESET  33 // orange wire
#define TARGET_TX_RX  25 // green wire
#define RED_LED_PWR   27 // red wire
#define RED_LED_GND   14 // black wire

#define GREEN_LED_PWR 23 // red wire
#define GREEN_LED_GND 22 // black wire
#define SSD1306_PWR   21 // red wire
#define SSD1306_GND   19 // black wire
#define SSD1306_SDA   18 // yellow wire
#define SSD1306_SCL   05 // white wire
#define BUTTON_1      15 // yellow wire

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<RED_LED_PWR) | (1ULL<<RED_LED_GND) | (1ULL<<GREEN_LED_PWR)  | (1ULL<<GREEN_LED_GND) )

#define BUF_SIZE (1024)
#define VALIDATE_UART_BAUD_RATE     115200
#define VALIDATE_UART_PORT_NUM      2
#define VALIDATE_TEST_RXD TARGET_RX_TX
#define VALIDATE_TEST_TXD TARGET_TX_RX
#define VALIDATE_TEST_RTS (UART_PIN_NO_CHANGE)
#define VALIDATE_TEST_CTS (UART_PIN_NO_CHANGE)
#define VALIDATE_TASK_STACK_SIZE    2048

#define U8G2_ESP32_HAL_DEFAULT_2  {        \
    (gpio_num_t) U8G2_ESP32_HAL_UNDEFINED, \
    (gpio_num_t) U8G2_ESP32_HAL_UNDEFINED, \
    (gpio_num_t) U8G2_ESP32_HAL_UNDEFINED, \
    (gpio_num_t) U8G2_ESP32_HAL_UNDEFINED, \
    (gpio_num_t) U8G2_ESP32_HAL_UNDEFINED, \
    (gpio_num_t) U8G2_ESP32_HAL_UNDEFINED, \
    (gpio_num_t) U8G2_ESP32_HAL_UNDEFINED, \
    I2C_MASTER_NUM,                        \
    I2C_MASTER_FREQ_HZ   }


enum cookerFirmwareVersion {
  cfvVALIDATED,
  cfvAMISS,
  cfvUNKNOWN
};

enum icon {
    completed
   ,failed
   ,progress_1_3
   ,progress_2_3
   ,progress_3_3
   ,open
};
