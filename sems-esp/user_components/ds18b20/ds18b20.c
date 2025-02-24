#include "ds18b20.h"

#define DS18B20_SKIP_ROM           0xCC
#define DS18B20_CONVERT_T          0x44
#define DS18B20_READ_SCRATCHPAD    0xBE

static const gpio_num_t DS18B20_GPIO = HW_DS18B20_GPIO;

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

esp_err_t ds18b20_init(void)
{
    gpio_reset_pin(DS18B20_GPIO);
    return ESP_OK;
}

double ds18b20_get_temp(void)
{
    if (reset() != ESP_OK) {
        return -999.0f;
    }

    write_byte(DS18B20_SKIP_ROM);
    write_byte(DS18B20_CONVERT_T);
    
    ets_delay_us(750000);
    
    if (reset() != ESP_OK) {
        return -999.0f;
    }
    
    write_byte(DS18B20_SKIP_ROM);
    write_byte(DS18B20_READ_SCRATCHPAD);
    
    uint8_t temp_l = read_byte();
    uint8_t temp_h = read_byte();
    
    int16_t temp = (temp_h << 8) | temp_l;
    return temp * 0.0625;  // 온도 변환 (섭씨)
}