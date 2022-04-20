

#define SAFETYCOOK_ROM

#define FLASHER_SOFTWARE_VERSION      "20220407"


// 
#define SAFETYCOOK_SOFTWARE_VERSION   "20220420"           // 1-4
#define SAFETYCOOK_BIN      safetycook_20220420_bin        // 2-4
#define SAFETYCOOK_BIN_SIZE safetycook_20220420_bin_size   // 3-4
// update (add empty line and save) /main/CMakeLists.txt   // 4-4

#define HIGHER_BAUDRATE 230400
#define BUTTON_1 15

#define SLAVE_RX_TX   35 // yellow // yellow 35
#define SLAVE_IO0     32 // blue   // white  32
#define SLAVE_RESET   33 // orange // red    33
#define SLAVE_TX_RX   25 // green  // white  25


#define RED_LED_PWR   27 // red
#define RED_LED_GND   14 // black


#define GREEN_LED_PWR 23 // red
#define GREEN_LED_GND 22 // black
#define SSD1306_PWR   21 // red
#define SSD1306_GND   19 // black
#define SSD1306_SDA   18 // yellow
#define SSD1306_SCL    5 // white

//#define SSD1306_I2C_ADDRESS    0x3C
//#define SSD1306_WIDTH           128
//#define SSD1306_HEIGHT           64
//#define SSD1306_I2C_RESET_PIN    -1


#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<RED_LED_PWR) | (1ULL<<RED_LED_GND) | (1ULL<<GREEN_LED_PWR)  | (1ULL<<GREEN_LED_GND) )

#define BUF_SIZE (1024)

#define VALIDATE_UART_BAUD_RATE     115200
#define VALIDATE_UART_PORT_NUM      2
#define VALIDATE_TEST_RXD SLAVE_RX_TX
#define VALIDATE_TEST_TXD SLAVE_TX_RX

#define VALIDATE_TEST_RTS (UART_PIN_NO_CHANGE)
#define VALIDATE_TEST_CTS (UART_PIN_NO_CHANGE)

#define VALIDATE_TASK_STACK_SIZE    2048


