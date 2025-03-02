#include "sqlite.h"

#define DB_UPDATE_BIT (1 << 0)

static const char *TAG      = "sqlite";
static const char *NAME     = "%s/" DB_NAME;
static esp_timer_handle_t sqlite_timer;
static EventGroupHandle_t event_group;

static void sqlite_timer_callback(void *arg)
{
    xEventGroupSetBits(event_group, DB_UPDATE_BIT);
}

static void sqlite(void *pvParameter)
{
    char db_path[32];
    snprintf(db_path, sizeof(db_path), NAME, get_mount_point());
    ESP_LOGI(TAG, "Database Path: %s", db_path);

    sqlite3 *db;
    sqlite3_initialize();

    int rc = db_open(db_path, &db);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to open database: %s", sqlite3_errmsg(db));
    }
    ESP_LOGI(TAG, "Database opened successfully");

    rc = db_exec(db, "PRAGMA journal_mode=WAL;");
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to set WAL mode: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
    }

    rc = db_exec(db, "CREATE TABLE IF NOT EXISTS temperature ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "timestamp DATETIME, "
                     "temperature FLOAT);");
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to create query: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
    }

    sqlite3_close(db);
    ESP_LOGI(TAG, "db created");

    uint32_t TIMER = (uint32_t)pvParameter;
    while (1)
    {
        EventBits_t bits =
            xEventGroupWaitBits(event_group, DB_UPDATE_BIT, pdTRUE, pdFALSE,
                                pdMS_TO_TICKS(TIMER + 100));

        if ((bits & DB_UPDATE_BIT))
        {
            char timestamp[20];
            get_timestamp(timestamp, sizeof(timestamp));

            sqlite3 *db;

            int rc = db_open(db_path, &db);
            if (rc != SQLITE_OK)
            {
                ESP_LOGE(TAG, "Failed to open database: %s",
                         sqlite3_errmsg(db));
                vTaskDelete(NULL);
            }
            ESP_LOGI(TAG, "open");
            vTaskDelay(pdMS_TO_TICKS(100));

            sqlite3_stmt *stmt;
            const char *sql = "INSERT INTO temperature (timestamp, "
                              "temperature) VALUES (?, ?);";

            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            if (rc != SQLITE_OK)
            {
                ESP_LOGE(TAG, "Failed to prepare statement: %s",
                         sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                vTaskDelete(NULL);
            }
            sqlite3_bind_text(stmt, 1, timestamp, -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 2, ds18b20_get_temp());

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                ESP_LOGE(TAG, "Failed to execute statement: %s",
                         sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                vTaskDelete(NULL);
            }

            sqlite3_finalize(stmt);

            sqlite3_close(db);
        }
    }
}

void sqlite_init(uint32_t interval_ms)
{
    event_group = xEventGroupCreate();
    xTaskCreate(&sqlite, "SQLITE3", 1024 * 6, (void*)interval_ms, 5, NULL);

    const esp_timer_create_args_t timer_args = {
        .callback = &sqlite_timer_callback,
        .name     = "sqlite_timer",
    };
    esp_timer_create(&timer_args, &sqlite_timer);
    esp_timer_start_periodic(sqlite_timer, interval_ms * 1000);
}