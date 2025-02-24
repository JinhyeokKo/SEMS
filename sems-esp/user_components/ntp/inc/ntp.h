#ifndef NTP_H
#define NTP_H

#include "sems_def.h"

#define TIMEZONE_SET   TIMEZONE_NTP
#define NTP_SERVER_SET SERVER_NTP

void initialize_sntp(void);
void get_timestamp(char *buffer, size_t size);

#endif