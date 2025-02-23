#include "main.h"

static const char *TAG = "main";

void sqlite(void *pvParameter)
{
    while (1)
    {
        double temp = ds18b20_get_temp();
        
        char db_path[32];
        snprintf(db_path, sizeof(db_path), "%s/temp.db", get_mount_point());
        ESP_LOGI(TAG, "Database Path: %s", db_path);

        sqlite3 *db;
        sqlite3_initialize();

        int rc = db_open(db_path, &db);
        if (rc != SQLITE_OK)
        {
            ESP_LOGE(TAG, "Failed to open database");
            vTaskDelete(NULL);
        }
        ESP_LOGI(TAG, "open");
        vTaskDelay(pdMS_TO_TICKS(100));

        rc = db_exec(db, "CREATE TABLE IF NOT EXISTS temperature ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                         "temperature FLOAT);");
        if (rc != SQLITE_OK)
        {
            ESP_LOGE(TAG, "Failed to create query");
            vTaskDelete(NULL);
        }

        sqlite3_stmt *stmt;
        const char *sql = "INSERT INTO temperature (temperature) VALUES (?);";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            ESP_LOGE(TAG, "Failed to prepare statement");
            vTaskDelete(NULL);
        }

        sqlite3_bind_double(stmt, 1, temp);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ESP_LOGE(TAG, "Failed to execute statement");
            vTaskDelete(NULL);
        }

        sqlite3_finalize(stmt);

        sqlite3_close(db);
        
        vTaskDelay(pdMS_TO_TICKS(300000));
    }
}

void app_init(void)
{
    ds18b20_init();
    sdcard_init();
    initialize_sntp();
}

void app_main(void)
{
    app_init();
    xTaskCreate(&sqlite, "SQLITE3", 1024 * 6, NULL, 5, NULL);
    websocket_main();
}