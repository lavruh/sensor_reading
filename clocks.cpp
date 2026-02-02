#include <M5Unified.h>
#include "field.h"

// `lineH` is defined in sensor_reading.ino
extern uint16_t lineH;
extern Field dateField;
extern Field timeField;

void updateClocks()
{
   if (M5.Rtc.isEnabled())
      {
        char buf[32];

        m5::rtc_datetime_t dt;
        if (M5.Rtc.getDateTime(&dt))
        {
        snprintf( buf, 30, "%04d-%02d-%02d"
                  , dt.date.year
                  , dt.date.month
                  , dt.date.date
                  );

        dateField.show( String(buf) );
        snprintf( buf, 30, "%02d:%02d:%02d"
                  , dt.time.hours
                  , dt.time.minutes
                  , dt.time.seconds
                  );
        timeField.show( String(buf));
        }
        else
        {
          dateField.show("RTC error");
        }
      }
}
