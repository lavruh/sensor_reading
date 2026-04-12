// #include <M5Unified.h>
// #include "menu.h"
// #include "field.h"


//  #define WIFI_SSID     "Ap9"
//  #define WIFI_PASSWORD "mimicipi1"
//  #define NTP_TIMEZONE  "EET-2"
//  #define NTP_SERVER1   "0.pool.ntp.org"
//  #define NTP_SERVER2   "1.pool.ntp.org"
//  #define NTP_SERVER3   "2.pool.ntp.org"

// #include <WiFi.h>


// // Different versions of the framework have different SNTP header file names and availability.
//  #if __has_include (<esp_sntp.h>)
//   #include <esp_sntp.h>
//   #define SNTP_ENABLED 1
//  #elif __has_include (<sntp.h>)
//   #include <sntp.h>
//   #define SNTP_ENABLED 1
//  #endif


// #ifndef SNTP_ENABLED
// #define SNTP_ENABLED 0
// #endif


// // Forward declarations from sensor_reading.ino
// extern TitleField titleField;
// extern Field body1;
// extern Field body2;

// // SetTimeMenuStrategy implementation
// void SetTimeMenuStrategy::loopTask() {
//   titleField.show("Set Time");
// }

// void SetTimeMenuStrategy::onButtonA() {
//   // Trigger time setting on button A
  
// /// setup RTC ( NTP auto setting )
//   configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
//   body1.show("WiFi:");

//   WiFi.begin( WIFI_SSID, WIFI_PASSWORD );

//   for (int i = 20; i && WiFi.status() != WL_CONNECTED; --i)
//   {
//     body2.show(".");
//     M5.delay(500);
//   }
//   if (WiFi.status() == WL_CONNECTED) {
//     body1.show("WiFi Connected.");
//     body2.show("NTP:");
// #if SNTP_ENABLED
//     while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
//     {
//       body2.show(".");
//       M5.delay(1000);
//     }
// #else
//     M5.delay(1600);
//     struct tm timeInfo;
//     while (!getLocalTime(&timeInfo, 1000))
//     {
//       body2.show('.', 2);
//     };
// #endif
//     body2.show("\r\nNTP Connected.");

//     time_t t = time(nullptr)+1; // Advance one second.
//     while (t > time(nullptr));  /// Synchronization in seconds
//     M5.Rtc.setDateTime( localtime( &t ) );
//   }
//   else
//   {
//     body1.show("\r\nWiFi none...");
//   }
// }
