/* 
    name: Esp32 serial flasher
    from: M.Mastenbroek (github.com/Machiel80)
*/

#include <sys/param.h>
#include <string.h>
#include "main.h"
#include "esp_err.h"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG // ESP_LOG_VERBOSE
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_loader.h"

#include <u8g2.h>
extern "C" {
    #include "esp32_port.h"
    #include "firmware_target.h"
    #include "u8g2_esp32_hal.h"
    #include "button.h"
    #include "icons.h"
}

/* extern "C" {
	void app_main(void);
} */

volatile bool _toggle_validate_slave_software_task_enabled   = true;
volatile bool _toggle_result_validate_slave_software_changed = true;
u8g2_t _u8g2;  // a structure which will contain all the data for one display
enum cookerFirmwareVersion _firmware_version = cfvUNKNOWN;

static void change_request_validate_software(const char* note, enum cookerFirmwareVersion firmware_version_state) {
    const char* TAG = "software changed";
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGD(TAG,"request '%s'",note);

    if(firmware_version_state != _firmware_version ) {
        _firmware_version = firmware_version_state;
        _toggle_result_validate_slave_software_changed = true;
        if(_firmware_version == cfvVALIDATED)
            ESP_LOGD(TAG,"state changed '%s' to VALIDATED",note);
        else if(_firmware_version == cfvAMISS)
            ESP_LOGD(TAG,"state changed '%s' to AMISS",note);
        else
            ESP_LOGD(TAG,"state changed '%s' to UNKNOWN",note);
    }
}

static void slave_software_validate_task(void *arg) {
    static const char* TAG = "validate software";
    static bool task_restart = false;
    esp_log_level_set(TAG, ESP_LOG_INFO);
    
    unsigned long REQUEST_FOR_VERSION_INTERVAL = 1000; // ms example: 1000 = 1 sec.
    unsigned long RESPONSE_FOR_VERSION_TIMEOUT =  200; // ms example: 1000 = 1 sec.
	unsigned long lastVersionRequestTime = 0;
    unsigned long lastVersionResponseTime = 0;

    bool version_request_sent = false;
    bool timeout_timer_active = false;

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = VALIDATE_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(VALIDATE_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(VALIDATE_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(VALIDATE_UART_PORT_NUM, VALIDATE_TEST_TXD, VALIDATE_TEST_RXD, VALIDATE_TEST_RTS, VALIDATE_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    ESP_LOGI(TAG,"(I) task started");
    ESP_LOGD(TAG,"(D) task started");

    while (true) {
        if(_toggle_validate_slave_software_task_enabled == true) {
            if(task_restart == true) {
                task_restart = false;
                ESP_LOGD(TAG,"task restarted");
                ESP_ERROR_CHECK(uart_driver_install(VALIDATE_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
                ESP_ERROR_CHECK(uart_param_config(VALIDATE_UART_PORT_NUM, &uart_config));
                ESP_ERROR_CHECK(uart_set_pin(VALIDATE_UART_PORT_NUM, VALIDATE_TEST_TXD, VALIDATE_TEST_RXD, VALIDATE_TEST_RTS, VALIDATE_TEST_CTS));

                ESP_LOGI(TAG, "reset slave");
                esp_loader_reset_target();
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }

            // Read data from the UART
            int len = uart_read_bytes(VALIDATE_UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);

            if(len > 0) {
                if(version_request_sent == true) {
                    version_request_sent = false;
                    if (memcmp ( data, SAFETYCOOK_SOFTWARE_VERSION, strlen(SAFETYCOOK_SOFTWARE_VERSION) ) == 0) {
                        change_request_validate_software("equal versions", cfvVALIDATED);
                    }else {
                        change_request_validate_software("unknown version", cfvAMISS);
                    }
                    timeout_timer_active = false;
                } else if(len > 5) { // slave is sending UART information, ignore input and wait until 
                    lastVersionRequestTime = esp_timer_get_time() / 1000ULL; // restart timer
                    change_request_validate_software("noise, version unknown", cfvUNKNOWN);
                }
                // ESP_LOGI(TAG,"%.*s",len, data);
                // printf("%.*s",len, data);
            } 

            if((esp_timer_get_time() / 1000ULL) - lastVersionRequestTime >= REQUEST_FOR_VERSION_INTERVAL) {
                lastVersionRequestTime += REQUEST_FOR_VERSION_INTERVAL; // reset timer
    
                if(version_request_sent == false) {
                    version_request_sent = true;
                    ESP_LOGD(TAG,"send UART request for version information");
                    // Write data back to the UART
                    uart_write_bytes(VALIDATE_UART_PORT_NUM, "version\n",strlen("version\n"));
                    lastVersionResponseTime = esp_timer_get_time() / 1000ULL; // restart timer
                    timeout_timer_active = true;
                }else {
                    change_request_validate_software("time-out (4 seconds)", cfvAMISS);
                    timeout_timer_active = false;
                    version_request_sent = false;
                }
            } // timer, request for version 

            if(timeout_timer_active == true && (esp_timer_get_time() / 1000ULL) - lastVersionResponseTime >= RESPONSE_FOR_VERSION_TIMEOUT) {
                timeout_timer_active = false;
                change_request_validate_software("time-out", cfvAMISS);
            } // timer, response for version 
        }else {
            
            if(task_restart == false) {
                task_restart = true;
                ESP_LOGD(TAG,"task pauze, UART driver turned-off");
                uart_driver_delete(VALIDATE_UART_PORT_NUM);
                change_request_validate_software("task turned off",cfvUNKNOWN);
            }
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
    } // endless loop
}

void set_icon(enum icon icon_name) {
    u8g2_ClearBuffer(&_u8g2);
    switch (icon_name) {
      case completed:
        u8g2_DrawXBM(&_u8g2, 32, 0, 64, 64, icon_completed);
        break;
      case failed:
        u8g2_DrawXBM(&_u8g2, 32, 0, 64, 64, icon_failed);
        break;
      case progress_1_3:
        u8g2_DrawXBM(&_u8g2, 32, 0, 64, 64, icon_progress_1_3);
        break;
      case progress_2_3:
        u8g2_DrawXBM(&_u8g2, 32, 0, 64, 64, icon_progress_2_3);
        break;
      case progress_3_3:
        u8g2_DrawXBM(&_u8g2, 32, 0, 64, 64, icon_progress_3_3);
        break;
      case open:
        u8g2_DrawXBM(&_u8g2, 32, 0, 64, 64, icon_open);
        u8g2_SetFont(&_u8g2, u8g2_font_6x13_tf);
        u8g2_DrawStr(&_u8g2, 40, 36, SAFETYCOOK_SOFTWARE_VERSION);
        break;    
    }
    u8g2_SendBuffer(&_u8g2);
}

extern "C" void app_main(void) {
    printf("Start ...\n" );
    static const char* TAG = "main";
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    gpio_config_t io_conf; // Fix issue with GPIO 14 (RED LED) didn't work well
    io_conf.intr_type = GPIO_INTR_DISABLE; // disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;       // set as output mode
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL; // bit mask of the pins that you want to set, e.g.GPIO14/27
    io_conf.pull_down_en = (gpio_pulldown_t) 0; // disable pull-down mode
    io_conf.pull_up_en = (gpio_pullup_t)0;      // disable pull-up mode
    gpio_config(&io_conf);   // configure GPIO with the given settings

    gpio_set_direction((gpio_num_t) SSD1306_PWR, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t) SSD1306_GND, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t) RED_LED_PWR, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t) RED_LED_GND, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t) GREEN_LED_PWR, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t) GREEN_LED_GND, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t) SSD1306_PWR, 1); // +3.3V
    gpio_set_level((gpio_num_t) SSD1306_GND, 0); // GND
    gpio_set_level((gpio_num_t) RED_LED_PWR, 0);
    gpio_set_level((gpio_num_t) RED_LED_GND, 0);
    gpio_set_level((gpio_num_t) GREEN_LED_PWR, 0);
    gpio_set_level((gpio_num_t) GREEN_LED_GND, 0); // GND

    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT_2;

    
    u8g2_esp32_hal.sda = (gpio_num_t) SSD1306_SDA;;  // data for I²C
    u8g2_esp32_hal.scl = (gpio_num_t) SSD1306_SCL;;  // clock for I²C

    // u8g2_esp32_hal.clk = (gpio_num_t) (-1);
    // u8g2_esp32_hal.mosi = (gpio_num_t) (-1);
    // u8g2_esp32_hal.cs = (gpio_num_t) (-1);
    // u8g2_esp32_hal.reset = (gpio_num_t) (-1);
    // u8g2_esp32_hal.dc = (gpio_num_t) (-1);
    // u8g2_esp32_hal.i2c_num = (gpio_num_t) (-1);
    // u8g2_esp32_hal.i2c_clk_speed = (gpio_num_t) (-1);

    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
          &_u8g2
        , U8G2_R0
        , u8g2_esp32_i2c_byte_cb
        , u8g2_esp32_gpio_and_delay_cb);  // init u8g2 structure
    
    // Note that address '0x78' is the I2C address already shifted left to include the read/write flag. 
    // Instead of supplying the address 0x3C which would be 0011 1100 supply 0x78 which would be 0111 1000.
    u8x8_SetI2CAddress(&_u8g2.u8x8, 0x78); 

    ESP_LOGI(TAG, "u8g2 InitDisplay");
    u8g2_InitDisplay(&_u8g2);  // send init sequence to the display, display is in, sleep mode after this
    ESP_LOGI(TAG, "u8g2 SetPowerSave");
    u8g2_SetPowerSave(&_u8g2, 0);  // wake up display
  
    u8g2_ClearBuffer(&_u8g2);
    u8g2_SetFont(&_u8g2, u8g2_font_tallpixelextended_tf);
    u8g2_DrawStr(&_u8g2, 31, 11, "SafetyCook");
    u8g2_SetFont(&_u8g2, u8g2_font_fub20_tn);
    u8g2_DrawStr(&_u8g2, 2, 41, SAFETYCOOK_SOFTWARE_VERSION);
    u8g2_SetFont(&_u8g2, u8g2_font_6x13_tf);
    u8g2_DrawStr(&_u8g2, 2, 63, "FlashBox:");
    u8g2_DrawStr(&_u8g2, 60, 63, FLASHER_SOFTWARE_VERSION);

    u8g2_SendBuffer(&_u8g2);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    target_binaries_t bin; // flash binary configuration

    const loader_esp32_config_t config = {
        .baud_rate = VALIDATE_UART_BAUD_RATE, // 115200
        .uart_port = UART_NUM_1,
        .uart_rx_pin =  TARGET_RX_TX, 
        .uart_tx_pin =  TARGET_TX_RX, 
        .reset_trigger_pin =  TARGET_RESET, 
        .gpio0_trigger_pin =  TARGET_IO0,
     // .rx_buffer_size = 0,    /*!< Set to zero for default RX buffer size */
     // .tx_buffer_size = 0,    /*!< Set to zero for default TX buffer size */        
     // .queue_size = 0,        /*!< Set to zero for default UART queue size */
     // .uart_queue = NULL      /*!< Set to NULL, if UART queue handle is not necessary. Otherwise, it will be assigned here */
    };

    button_event_t ev;
    QueueHandle_t button_events = button_init(PIN_BIT(BUTTON_1));
    xTaskCreate(slave_software_validate_task, "software validate task", VALIDATE_TASK_STACK_SIZE, NULL, 10, NULL);

    while (true) { // endless loop

        if(_toggle_result_validate_slave_software_changed == true) {
            _toggle_result_validate_slave_software_changed = false;

            ESP_LOGI(TAG, "validate slave software, state changed");
            if(_firmware_version == cfvVALIDATED) {
                gpio_set_level((gpio_num_t) GREEN_LED_PWR, 1);
                set_icon(completed);
                ESP_LOGI(TAG, "slave software validated");
            } else if(_firmware_version == cfvAMISS) {
                gpio_set_level((gpio_num_t) GREEN_LED_PWR, 0);
                set_icon(open);
                ESP_LOGI(TAG, "slave software amiss");
            } else { // cfvUNKNOWN
                gpio_set_level((gpio_num_t) GREEN_LED_PWR, 0);
                // set_icon(open);
                ESP_LOGI(TAG, "slave software unknown");
            }
        }

        if (xQueueReceive(button_events, &ev, 1000/portTICK_PERIOD_MS)) {
            if ((ev.pin == BUTTON_1) && (ev.event == BUTTON_DOWN)) {

                ESP_LOGI(TAG, "button pressed");
                set_icon(progress_1_3);
                gpio_set_level((gpio_num_t) RED_LED_PWR, 1);
                gpio_set_level((gpio_num_t) GREEN_LED_PWR, 0);
                _toggle_validate_slave_software_task_enabled = false;
                vTaskDelay(100 / portTICK_PERIOD_MS);

                if (loader_port_esp32_init(&config) != ESP_LOADER_SUCCESS) {
                    ESP_LOGE(TAG, "serial initialization failed");
                    set_icon(failed);
                } else {
                    ESP_LOGI(TAG, "serial initialization completed");
                    if (connect_to_target(HIGHER_BAUDRATE) != ESP_LOADER_SUCCESS) {
                        ESP_LOGE(TAG, "connect to target failed");
                        set_icon(failed);
                    } else {
                        ESP_LOGI(TAG, "connect to target completed");
                        get_binaries(esp_loader_get_target(), &bin);
                        if (flash_binary(bin.boot.data, bin.boot.size, bin.boot.addr) != ESP_LOADER_SUCCESS) {
                            ESP_LOGE(TAG, "flash binary 'boot' failed");
                            set_icon(failed);
                        } else {
                            ESP_LOGI(TAG, "flash binary 'boot' completed");
                            
                            if(flash_binary(bin.part.data, bin.part.size, bin.part.addr) != ESP_LOADER_SUCCESS) {
                                ESP_LOGE(TAG, "flash binary 'part' failed");
                                set_icon(failed);
                            } else {
                                ESP_LOGI(TAG, "flash binary 'part' completed");
                                    set_icon(progress_2_3);
                                if(flash_binary(bin.app.data,  bin.app.size,  bin.app.addr) != ESP_LOADER_SUCCESS) {
                                    ESP_LOGE(TAG, "flash binary 'app' failed");
                                    set_icon(failed);
                                } else {
                                    ESP_LOGI(TAG, "flash binary 'app' completed");
                                    set_icon(progress_3_3);
#ifdef SAFETYCOOK_ROM
                                    if(flash_binary(bin.ota.data,  bin.ota.size,  bin.ota.addr) != ESP_LOADER_SUCCESS) {
                                        ESP_LOGE(TAG, "flash binary 'ota' failed");
                                        set_icon(failed);
                                    } else {    
                                        ESP_LOGI(TAG, "flash binary 'ota' completed");                    
                                    } // ota completed
#endif
                                } // app completed
                            } // part completed
                        } // boot completed
                    } // connect to slave completed
                } // serial initialization completed

                ESP_LOGI(TAG, "uart driver delete");
                uart_driver_delete(config.uart_port);
                xQueueReset(button_events); // remove button presses 
                gpio_set_level((gpio_num_t) RED_LED_PWR, 0);
                _toggle_validate_slave_software_task_enabled = true;
                
            } // button 1 pressed
        } // some button events
    } // endless loop
} // app main