#include "ntp.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

static const char *TAG = "NTP_SYNC";

static const char *TIMEZONE   = TIMEZONE_SET;
static const char *NTP_SERVER = NTP_SERVER_SET;

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "NTP 시간 동기화 완료");

    setenv("TZ", TIMEZONE, 1);
    tzset();
    ESP_LOGI(TAG, "TZ 환경 변수 설정 : %s", getenv("TZ"));

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "현재 시간: %d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void ntp_init(void)
{
    ESP_LOGI(TAG, "NTP 초기화 중...");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTP_SERVER);
    config.sync_cb           = time_sync_notification_cb;
    esp_netif_sntp_init(&config);

    int retry             = 0;
    const int max_retries = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET &&
           retry < max_retries)
    {
        retry++;
        ESP_LOGI(TAG, "NTP 동기화 대기 중... (%d/%d)", retry, max_retries);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
    {
        ESP_LOGE(TAG, "RTC 시간 설정 완료");
    }
    else
    {
        ESP_LOGI(TAG, "NTP 동기화 실패");
    }
}

void get_timestamp(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &timeinfo);
}