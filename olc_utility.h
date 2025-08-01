// Integration for Google's C implementation[1] of Open Location Codes[2][3]
//
// [1]: https://github.com/google/open-location-code/tree/main/c/src
// [2]: https://en.wikipedia.org/wiki/Open_Location_Code
// [3]: https://maps.google.com/pluscodes/

#include "lib/olc.h"

int str_cat_plus_code(string_t *str, double lat, double lon)
{
    char out_buffer[256];
    OLC_LatLon location;
    location.lat = lat;
    location.lon = lon;
    int len = OLC_EncodeDefault(&location, out_buffer, ARRAY_SIZE(out_buffer));

    if (len > 0) {
        str_cat_c (str, out_buffer);
    }

    return len > 0 ? 1 : 0;
}
