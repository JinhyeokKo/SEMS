#include "ds18b20.h"

#define DS18B20_SKIP_ROM           0xCC
#define DS18B20_CONVERT_T          0x44
#define DS18B20_READ_SCRATCHPAD    0xBE

static const gpio_num_t DS18B20_GPIO = HW_DS18B20_GPIO;
static double latest_temp = -999.0;
static esp_timer_handle_t ds18b20_timer;

static void write_bit(int bit)
{
    gpio_set_direction(DS18B20_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_GPIO, 0);
    ets_delay_us(5);
    
    if (bit) {
        gpio_set_level(DS18B20_GPIO, 1);
    }
    ets_delay_us(60);
    gpio_set_level(DS18B20_GPIO, 1);
    ets_delay_us(1);
}

static int read_bit(void)
{
    int bit;
    gpio_set_direction(DS18B20_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_GPIO, 0);
    ets_delay_us(2);
    
    gpio_set_direction(DS18B20_GPIO, GPIO_MODE_INPUT);
    ets_delay_us(10);
    bit = gpio_get_level(DS18B20_GPIO);
    ets_delay_us(50);
    
    return bit;
}

static void write_byte(uint8_t data)
{
    for (int i = 0; i < 8; i++) {
        write_bit(data & 1);
        data >>= 1;
    }
}

static uint8_t read_byte(void)
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        data >>= 1;
        if (read_bit()) {
            data |= 0x80;
        }
    }
    return data;
}

static esp_err_t reset(void)
{
    gpio_set_direction(DS18B20_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_GPIO, 0);
    ets_delay_us(480);
    
    gpio_set_direction(DS18B20_GPIO, GPIO_MODE_INPUT);
    ets_delay_us(70);
    
    int presence = gpio_get_level(DS18B20_GPIO);
    ets_delay_us(410);
    
    return presence ? ESP_FAIL : ESP_OK;
}

static void ds18b20_read_temp(void *arg)
{
    if (reset() == ESP_OK) {
        write_byte(DS18B20_SKIP_ROM);
        write_byte(DS18B20_CONVERT_T);
    }

    vTaskDelay(pdMS_TO_TICKS(750));

    if (reset() != ESP_OK) {
        latest_temp = -999.0;
        return;
    }

    write_byte(DS18B20_SKIP_ROM);
    write_byte(DS18B20_READ_SCRATCHPAD);
    
    uint8_t temp_l = read_byte();
    uint8_t temp_h = read_byte();
    
    int16_t temp = (temp_h << 8) | temp_l;
    latest_temp = temp * 0.0625;
    ESP_LOGI("DS18B20", "Temp: %.2f C", latest_temp);
}

void ds18b20_timer_init(uint32_t interval_ms)
{
    const esp_timer_create_args_t timer_args = {
        .callback = &ds18b20_read_temp,
        .name = "ds18b20_timer"
    };
    esp_timer_create(&timer_args, &ds18b20_timer);
    esp_timer_start_periodic(ds18b20_timer, interval_ms * 1000);
}

double ds18b20_get_temp(void)
{
    return latest_temp;
}