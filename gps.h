typedef struct
{
    int year;
    byte month;
    byte day;
    byte hour;
    byte minute;
    byte second;
} s_date;

typedef struct
{
    float longitude;
    float latitude;
} s_coords;

bool readGPSDateTime(s_date *date);
bool readGPS(s_coords *coords, s_date *date);
void smartdelay(unsigned long ms);