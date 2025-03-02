#include "ds18b20.h"
#include "ntp.h"
#include "sdcard.h"
#include "sems_def.h"
#include "sqlite.h"
#include "web.h"

static const uint32_t TIMER = SAVE_TIME_INTERVAL;

void app_main(void)
{
    ds18b20_timer_init(TIMER);
    sdcard_init();
    sqlite_init(TIMER);
    websocket_init();
    initialize_sntp();
}