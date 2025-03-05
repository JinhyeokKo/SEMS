#include "wifi_prov.h"
#include "ble_prov.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "mdns.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

#ifndef CONFIG_WIFI
#include "ble_prov.h"
#endif

static const char *TAG = "wifi";

MessageBufferHandle_t xMessageBufferToClient;
static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
#ifndef CONFIG_WIFI
        ESP_ERROR_CHECK(ble_prov_stop());
#endif
    }
}

esp_err_t wifi_initialize(void)
{
    const int max_retries = 3;
    int retry             = 0;
    esp_err_t ret;

    while (retry < max_retries)
    {
        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
            ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_LOGW(TAG, "NVS 파티션에 빈 페이지가 없거나 새로운 버전을 "
                          "발견했습니다. 초기화를 시도합니다.");
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        
        if (ret == ESP_OK)
        {
            retry = 0;
            break;
        }else{
            retry++;
            ESP_LOGI(TAG, "NVS 초기화 대기중... (%d/%d)", retry, max_retries);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS 초기화 실패: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    s_wifi_event_group = xEventGroupCreate();
    if (!s_wifi_event_group)
    {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

#ifdef CUSTOM_IP
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));

    static const int ip[]      = IP4_IP_CUSTOM;
    static const int gateway[] = IP4_GATEWAY_CUSTOM;
    static const int netmask[] = IP4_NETMASK_CUSTOM;
    static const int dns[]     = IP4_DNS_CUSTOM;

    esp_netif_ip_info_t ip_info = {0};
    IP4_ADDR(&ip_info.ip, ip[0], ip[1], ip[2], ip[3]);
    IP4_ADDR(&ip_info.gw, gateway[0], gateway[1], gateway[2], gateway[3]);
    IP4_ADDR(&ip_info.netmask, netmask[0], netmask[1], netmask[2], netmask[3]);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));

    esp_netif_dns_info_t dns_info = {0};
    esp_ip4_addr_t dns_ip;
    IP4_ADDR(&dns_ip, dns[0], dns[1], dns[2], dns[3]);
    dns_info.ip.u_addr.ip4 = dns_ip;
    dns_info.ip.type       = IPADDR_TYPE_V4;
    ESP_ERROR_CHECK(
        esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns_info));
#else
    esp_netif_create_default_wifi_sta();
#endif

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,
        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL,
        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta =
            {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                .pmf_cfg            = {.capable = true, .required = false},
            },
    };

#ifndef CONFIG_WIFI
    ESP_ERROR_CHECK(ble_prov_init());
    ESP_ERROR_CHECK(ble_prov_start());

    wifi_credentials_t credentials;
    ESP_ERROR_CHECK(ble_prov_get_credentials(&credentials));

    strcpy((char *)wifi_config.sta.ssid, credentials.ssid);
    strcpy((char *)wifi_config.sta.password, credentials.password);
#endif

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-F-Fi 초기화 완료");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s", wifi_config.sta.ssid);
        return ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void mdns_initialise(void)
{
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(CONFIG_MDNS_HOSTNAME));
    ESP_LOGI(TAG, "mdns hostname set to: [%s]", CONFIG_MDNS_HOSTNAME);
    ESP_ERROR_CHECK(mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0));
}

void wifi_init(void)
{
    wifi_initialize();
    mdns_initialise();

    xMessageBufferToClient = xMessageBufferCreate(1024);
    configASSERT(xMessageBufferToClient);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void wifi_get_ip(char *buffer)
{
    esp_netif_ip_info_t ip_info;
    ESP_ERROR_CHECK(esp_netif_get_ip_info(
        esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info));
    sprintf(buffer, IPSTR, IP2STR(&ip_info.ip));
}