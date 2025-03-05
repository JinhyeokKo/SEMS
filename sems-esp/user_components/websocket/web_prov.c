#include "web_prov.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdcard.h"
#include "websocket_server.h"
#include "wifi_prov.h"

void client_task(void *pvParameters);
void server_task(void *pvParameters);

void web_prov(void)
{
    ws_server_start();

    char ipinfo[64];
    wifi_get_ip(ipinfo);

    xTaskCreate(&server_task, "server_task", 1024 * 8, (void *)ipinfo, 5, NULL);

    xTaskCreate(&client_task, "client_task", 1024 * 6,
                (void *)get_mount_point(), 5, NULL);

    vTaskDelay(100);
}