

#define SAFETYCOOK_ROM
#define FLASHER_SOFTWARE_VERSION      "20220421"

#define SAFETYCOOK_SOFTWARE_VERSION   "20220420"           // 1-4
#define SAFETYCOOK_BIN      safetycook_20220420_bin        // 2-4
#define SAFETYCOOK_BIN_SIZE safetycook_20220420_bin_size   // 3-4
// update (add empty line and save) /main/CMakeLists.txt   // 4-4

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
