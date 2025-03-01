#include "ds18b20.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ntp.h"
#include "sdcard.h"
#include "sems_def.h"
#include "web.h"

static const char *TAG      = "main";
static const char *NAME     = "%s/" DB_NAME;
static const uint32_t TIMER = SAVE_TIME_INTERVAL;

void sqlite(void *pvParameter)
{
    double temp = 0;
    char timestamp[20];
    char db_path[32];
    snprintf(db_path, sizeof(db_path), NAME, get_mount_point());
    ESP_LOGI(TAG, "Database Path: %s", db_path);

    sqlite3 *db;
    sqlite3_initialize();

    int rc = db_open(db_path, &db);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to open database: %s", sqlite3_errmsg(db));
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG, "open");
    vTaskDelay(pdMS_TO_TICKS(100));

    rc = db_exec(db, "PRAGMA journal_mode=WAL;");
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to set WAL mode: %s", sqlite3_errmsg(db));
    }

    rc = db_exec(db, "CREATE TABLE IF NOT EXISTS temperature ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "timestamp DATETIME, "
                     "temperature FLOAT);");
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to create query: %s", sqlite3_errmsg(db));
        vTaskDelete(NULL);
    }

    sqlite3_close(db);

    while (1)
    {
        temp = ds18b20_get_temp();
        get_timestamp(timestamp, sizeof(timestamp));

        sqlite3 *db;
        sqlite3_initialize();

        int rc = db_open(db_path, &db);
        if (rc != SQLITE_OK)
        {
            ESP_LOGE(TAG, "Failed to open database: %s", sqlite3_errmsg(db));
            vTaskDelete(NULL);
        }
        ESP_LOGI(TAG, "open");
        vTaskDelay(pdMS_TO_TICKS(100));

        sqlite3_stmt *stmt;
        const char *sql =
            "INSERT INTO temperature (timestamp, temperature) VALUES (?, ?);";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            ESP_LOGE(TAG, "Failed to prepare statement: %s",
                     sqlite3_errmsg(db));
            vTaskDelete(NULL);
        }

        sqlite3_bind_text(stmt, 1, timestamp, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, temp);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ESP_LOGE(TAG, "Failed to execute statement: %s",
                     sqlite3_errmsg(db));
            vTaskDelete(NULL);
        }

        sqlite3_finalize(stmt);

        sqlite3_close(db);

        vTaskDelay(pdMS_TO_TICKS(TIMER));
    }
}

void app_main(void)
{
    ds18b20_timer_init();
    ds18b20_start_timer(SAVE_TIME_INTERVAL);
    sdcard_init();
    websocket_init();
    initialize_sntp();

    xTaskCreate(&sqlite, "SQLITE3", 1024 * 6, NULL, 5, NULL);
}