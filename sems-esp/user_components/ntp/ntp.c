#include "ntp.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "sdkconfig.h"

static const char *TAG = "NTP_SYNC";

// NTP 동기화 완료 콜백
void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "NTP 시간 동기화 완료");

    // 동기화된 시간 출력
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "현재 시간: %d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

// NTP 초기화 및 RTC 동기화
void initialize_sntp(void) {
    ESP_LOGI(TAG, "NTP 초기화 중...");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("kr.pool.ntp.org");
    config.sync_cb = time_sync_notification_cb; // 동기화 완료 콜백
    esp_netif_sntp_init(&config);

    // NTP 시간 동기화 대기
    int retry = 0;
    const int max_retries = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < max_retries) {
        ESP_LOGI(TAG, "NTP 동기화 대기 중... (%d/%d)", retry, max_retries);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        retry++;
    }

    // RTC 설정 완료
    ESP_LOGI(TAG, "RTC 시간 설정 완료");
}