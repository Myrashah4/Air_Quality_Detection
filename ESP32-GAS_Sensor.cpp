extern "C" {
    #include "freertos/FreeRTOS.h"//For delays and multitasking.
    #include "freertos/task.h"
    #include "driver/i2c.h"//I2C driver for communication with OLED
    #include "esp_log.h"//For logging debug messages
    #include "esp_adc/adc_oneshot.h"//For reading analog values
}

#include <cstring>  // For strlen function
#include "ssd1306.h" //Library to control the 128x64 OLED display.
#include "font8x8_basic.h"// Character font used for text display.

#define TAG "GAS"
#define GAS_SENSOR_CHANNEL ADC_CHANNEL_6  //The gas sensor is connected to GPIO34, which maps to ADC1 channel 6.

static SSD1306_t dev;//internal state for the OLED display

extern "C" void app_main(void) {
    // OLED I2C init (SDA = 21, SCL = 22)
    i2c_master_init(&dev, GPIO_NUM_21, GPIO_NUM_22, -1);
    
    ssd1306_init(&dev, 128, 64);  // OLED with 128x64 resolution.
    
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xFF);
    ssd1306_display_text(&dev, 0, "Initializing...", 13, false);//message on line 0
    ssd1306_show_buffer(&dev);

    // ADC Setup
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);//Configures the channel connected to the gas sensor (GPIO34).

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc1_handle, GAS_SENSOR_CHANNEL, &chan_config);

    vTaskDelay(pdMS_TO_TICKS(2000));

    while (true) {
        int raw = 0;//Reads raw analog value from the gas sensor.
        adc_oneshot_read(adc1_handle, GAS_SENSOR_CHANNEL, &raw);
        ESP_LOGI(TAG, "Gas ADC raw: %d", raw);

        // Clear screen and display information
        ssd1306_clear_screen(&dev, false);
        
        // Display gas detection status on line 0
        if (raw > 3000) {
            ssd1306_display_text(&dev, 0, "GAS Detected!", 13, false);
        } else {
            ssd1306_display_text(&dev, 0, "No Gas", 6, false);
        }
        
        // Display raw ADC value on line 1
        char value_str[20];
        snprintf(value_str, sizeof(value_str), "Value: %d", raw);
        ssd1306_display_text(&dev, 1, value_str, strlen(value_str), false);
        
        // Display voltage equivalent on line 2 (assuming 3.3V reference)
        float voltage = (raw / 4095.0) * 3.3;
        char voltage_str[20];
        snprintf(voltage_str, sizeof(voltage_str), "Voltage: %.2fV", voltage);
        ssd1306_display_text(&dev, 2, voltage_str, strlen(voltage_str), false);
        
        ssd1306_show_buffer(&dev);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}