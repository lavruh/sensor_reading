#include <M5Unified.h>

// `lineH` is defined in sensor_reading.ino
extern uint16_t lineH;

void updateClocks()
{
   if (M5.Rtc.isEnabled())
      {
        char buf[32];

        /// Get the date and time from the RTC and display it.
        m5::rtc_datetime_t dt;
        if (M5.Rtc.getDateTime(&dt))
        {
          M5.Display.startWrite();
          snprintf( buf, 30, "%04d-%02d-%02d"
                  , dt.date.year
                  , dt.date.month
                  , dt.date.date
                  );
          M5.Display.drawString(buf, M5.Display.width() / 2 - 3, 5);
          snprintf( buf, 30, "%02d:%02d:%02d"
                  , dt.time.hours
                  , dt.time.minutes
                  , dt.time.seconds
                  );
          M5.Display.drawString(buf, M5.Display.width() / 2 - 3, lineH+5);
          M5.Display.endWrite();
        }
        else
        {
          M5.Display.drawString("RTC error", M5.Display.width() / 2, lineH>>1);
        }
      }
}
