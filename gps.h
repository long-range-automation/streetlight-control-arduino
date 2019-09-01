#include "datetime.h"

#define GPS_SERIAL Serial1

typedef struct
{
    float longitude;
    float latitude;
} s_coords;

void gps_setup();
bool gps_read_date(s_date *date);
bool gps_read_time(s_time *time);
bool gps_read_coords(s_coords *coords);
void gps_delay(unsigned long ms);
void gps_debug_loop();