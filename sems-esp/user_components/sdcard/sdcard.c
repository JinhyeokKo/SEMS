#include "sdcard.h"

static const char *mount_point = MOUNT_POINT;
static const char *TAG         = "SDCARD";
static sdmmc_card_t *card      = NULL;
static bool is_mounted         = false;

esp_err_t sdcard_init(void)
{
    if (is_mounted)
    {
        ESP_LOGI(TAG, "SD카드가 이미 마운트되어 있습니다");
        return ESP_OK;
    }

    esp_err_t ret;
    const int max_retries = 3;
    int retry             = 0;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = PIN_NUM_MOSI,
        .miso_io_num     = PIN_NUM_MISO,
        .sclk_io_num     = PIN_NUM_CLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 4000,
    };

    ESP_LOGI(TAG, "SD카드 초기화");

    while (retry < max_retries)
    {
        ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
        if (ret == ESP_OK)
        {
            retry = 0;
            break;
        }
        retry++;
        ESP_LOGI(TAG, "SPI 버스 초기화 대기중... (%d/%d)", retry,
                 max_retries);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    ESP_LOGI(TAG, "SD카드 마운트 시작");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files              = 5,
        .allocation_unit_size   = 16 * 1024};

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs               = PIN_NUM_CS;
    slot_config.host_id               = host.slot;

    while (retry < max_retries)
    {
        ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config,
                                      &mount_config, &card);

        if (ret == ESP_OK)
        {
            is_mounted = true;
            retry      = 0;
            ESP_LOGI(TAG, "SD카드 마운트 성공");
            sdmmc_card_print_info(stdout, card);
            break;
        }
        else if (ret == ESP_FAIL || ret == ESP_ERR_TIMEOUT)
        {
            retry++;
            ESP_LOGI(TAG, "SD카드 마운트 대기중... (%d/%d)", retry,
                     max_retries);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
            ESP_LOGE(TAG,
                     "SD카드 초기화 실패 (%s). "
                     "SD카드 라인에 풀업 저항이 있는지 확인하세요.",
                     esp_err_to_name(ret));
            spi_bus_free(host.slot);
            return ret;
        }
    }

    if (retry >= max_retries)
    {
        ESP_LOGE(TAG, "최대 재시도 횟수 초과, SD카드 초기화 실패");
        spi_bus_free(host.slot);
        return ESP_FAIL;
    }
    return ESP_OK;
}

const char *get_mount_point(void)
{
    return mount_point;
}