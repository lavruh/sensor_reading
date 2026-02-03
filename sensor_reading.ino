#include <M5Unified.h>
#include <M5GFX.h>
#include "field.h"

#define US_KEYBOARD 1
#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

#define DEVICE_NAME "ESP32 Keyboard"

bool isBleConnected = false;
String dataToSend = "";

M5GFX display;

uint16_t lineH = 30;

uint16_t menuIndex = 0;

Field dateField = Field();
Field timeField = Field();
Field batField = Field();
Field bleField = Field();
TitleField titleField = TitleField();
TitleField body1 = TitleField();
TitleField body2 = TitleField();


// Forward declarations
void typeText(const char* text);
void bluetoothTask(void*);
void updateClocks(void);
void checkBat(void);
void checkBLE(void);
void diplayBleStatus(String);
void setTime(void);
void showIMU(void);
void setTimeMenu(void);

int batteryLevel = 100;

struct rect_t
{
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
};

void buttons()
{
  if(M5.BtnA.wasClicked()){
    if(menuIndex == 1) setTime();
    typeText(dataToSend);
  }

  if(M5.BtnB.wasClicked()){
    menuIndex = menuIndex+1;
    if(menuIndex == 2) menuIndex = 0;
    M5.Display.clear();
  }

  if(M5.BtnPWR.wasClicked()){
  }
}





void setup()
{
    auto cfg = M5.config();  
    M5.begin(cfg);  
    M5.Display.setTextSize(1);
    lineH = M5.Display.fontHeight();
    int h = lineH;
    int w = M5.Display.width()/2;

    dateField = Field(M5.Display.width() / 2, 5, h, w);
    timeField = Field(M5.Display.width() / 2, lineH*2, h, w);
    batField = Field(5, 5, h, w);
    bleField = Field(5, lineH*2, h, w);
    titleField = TitleField(0, lineH * 5, lineH, M5.Display.width(), &fonts::Font4);
    body1 = TitleField(0, lineH * 9, lineH, M5.Display.width(), &fonts::Font2);
    body2 = TitleField(0, lineH * 12, lineH, M5.Display.width(), &fonts::Font2);

    M5.Imu.begin();
    xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
}

// loop function is executed repeatedly for as long as it is running.
// loop function acquires values from sensors, rewrites the screen, etc.
void loop()
{
  M5.update();
  // checkBat();
  updateClocks();
  checkBLE();
  buttons();   
  
  if(menuIndex == 0) showIMU();  
  if(menuIndex == 1) setTimeMenu();             
}

// Message (report) sent when a key is pressed or released
struct InputReport {
    uint8_t modifiers;	     // bitmask: CTRL = 1, SHIFT = 2, ALT = 4
    uint8_t reserved;        // must be 0
    uint8_t pressedKeys[6];  // up to six concurrenlty pressed keys
};

// Message (report) received when an LED's state changed
struct OutputReport {
    uint8_t leds;            // bitmask: num lock = 1, caps lock = 2, scroll lock = 4, compose = 8, kana = 16
};


// The report map describes the HID device (a keyboard in this case) and
// the messages (reports in HID terms) sent and received.
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1),      0x01,       // Generic Desktop Controls
    USAGE(1),           0x06,       // Keyboard
    COLLECTION(1),      0x01,       // Application
    REPORT_ID(1),       0x01,       //   Report ID (1)
    USAGE_PAGE(1),      0x07,       //   Keyboard/Keypad
    USAGE_MINIMUM(1),   0xE0,       //   Keyboard Left Control
    USAGE_MAXIMUM(1),   0xE7,       //   Keyboard Right Control
    LOGICAL_MINIMUM(1), 0x00,       //   Each bit is either 0 or 1
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_COUNT(1),    0x08,       //   8 bits for the modifier keys
    REPORT_SIZE(1),     0x01,       
    HIDINPUT(1),        0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   1 byte (unused)
    REPORT_SIZE(1),     0x08,
    HIDINPUT(1),        0x01,       //   Const, Array, Abs
    REPORT_COUNT(1),    0x06,       //   6 bytes (for up to 6 concurrently pressed keys)
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,       //   101 keys
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    HIDINPUT(1),        0x00,       //   Data, Array, Abs
    REPORT_COUNT(1),    0x05,       //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
    REPORT_SIZE(1),     0x01,
    USAGE_PAGE(1),      0x08,       //   LEDs
    USAGE_MINIMUM(1),   0x01,       //   Num Lock
    USAGE_MAXIMUM(1),   0x05,       //   Kana
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    HIDOUTPUT(1),       0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   3 bits (Padding)
    REPORT_SIZE(1),     0x03,
    HIDOUTPUT(1),       0x01,       //   Const, Array, Abs
    END_COLLECTION(0)               // End application collection
};


BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;

const InputReport NO_KEY_PRESSED = { };


/*
 * Callbacks related to BLE connection
 */
class BleKeyboardCallbacks : public BLEServerCallbacks {

    void onConnect(BLEServer* server) {
        isBleConnected = true;

        // Allow notifications for characteristics
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(true);
    }

    void onDisconnect(BLEServer* server) {
        isBleConnected = false;

        // Disallow notifications for characteristics
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(false);
    }
};


/*
 * Called when the client (computer, smart phone) wants to turn on or off
 * the LEDs in the keyboard.
 * 
 * bit 0 - NUM LOCK
 * bit 1 - CAPS LOCK
 * bit 2 - SCROLL LOCK
 */
 class OutputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) {
        OutputReport* report = (OutputReport*) characteristic->getData();
//        Serial.print("LED state: ");
//        Serial.print((int) report->leds);
//        Serial.println();
    }
};


void bluetoothTask(void*) {

    // initialize the device
    BLEDevice::init(DEVICE_NAME);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BleKeyboardCallbacks());

    // create an HID device
    hid = new BLEHIDDevice(server);
    input = hid->inputReport(1); // report ID
    output = hid->outputReport(1); // report ID
    output->setCallbacks(new OutputCallbacks());

    // set manufacturer name
    hid->manufacturer()->setValue("Maker Community");
    // set USB vendor and product ID
    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    // information about HID device: device is not localized, device can be connected
    hid->hidInfo(0x00, 0x02);

    // Security: device requires bonding
    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);

    // set report map
    hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    hid->startServices();

    // set battery level to 100%
    hid->setBatteryLevel(batteryLevel);

    // advertise the services
    BLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->addServiceUUID(hid->deviceInfo()->getUUID());
    advertising->addServiceUUID(hid->batteryService()->getUUID());
    advertising->start();

    // M5.Display.startWrite();
    // M5.Display.setCursor(5, lineH*3);
    // M5.Display.print("BLE ready");
    // M5.Display.endWrite();
    bleField.show("BLE ready");
    delay(portMAX_DELAY);
};


void typeText(String text) {
    if(!isBleConnected) return;
    // int len = strlen(text);
    int len = text.length();
    for (int i = 0; i < len; i++) {

        // translate character to key combination
        uint8_t val = (uint8_t)text[i];
        if (val > KEYMAP_SIZE)
            continue; // character not available on keyboard - skip
        KEYMAP map = keymap[val];

        // create input report
        InputReport report = {
            .modifiers = map.modifier,
            .reserved = 0,
            .pressedKeys = {
                map.usage,
                0, 0, 0, 0, 0
            }
        };

        // send the input report
        input->setValue((uint8_t*)&report, sizeof(report));
        input->notify();

        delay(5);

        // release all keys between two characters; otherwise two identical
        // consecutive characters are treated as just one key press
        input->setValue((uint8_t*)&NO_KEY_PRESSED, sizeof(NO_KEY_PRESSED));
        input->notify();

        delay(5);
    }
}

void checkBLE(){
  if(isBleConnected){
    bleField.show("Conn");
  }else{
    bleField.show("Disc");
  }
}



void diplayWifiStatus(String msg, uint8_t offset=0){
        M5.Display.drawString(msg,M5.Display.width()/4, lineH * (4+offset));
}

 #define WIFI_SSID     "Ap9"
 #define WIFI_PASSWORD "mimicipi1"
 #define NTP_TIMEZONE  "EET-2"
 #define NTP_SERVER1   "0.pool.ntp.org"
 #define NTP_SERVER2   "1.pool.ntp.org"
 #define NTP_SERVER3   "2.pool.ntp.org"

#include <WiFi.h>


// Different versions of the framework have different SNTP header file names and availability.
 #if __has_include (<esp_sntp.h>)
  #include <esp_sntp.h>
  #define SNTP_ENABLED 1
 #elif __has_include (<sntp.h>)
  #include <sntp.h>
  #define SNTP_ENABLED 1
 #endif


#ifndef SNTP_ENABLED
#define SNTP_ENABLED 0
#endif

void setTimeMenu(){
  // M5.Display.setTextSize(2);
  titleField.show("Set Time");
  // M5.Display.drawString("Get NTP", 15, lineH * 6);
  // M5.Display.drawString("time", 15, lineH * 7);
  // M5.Display.setTextSize(1);
}

void setTime(){
/// setup RTC ( NTP auto setting )
  configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
  diplayWifiStatus("WiFi:");

  WiFi.begin( WIFI_SSID, WIFI_PASSWORD );

  for (int i = 20; i && WiFi.status() != WL_CONNECTED; --i)
  {
    diplayWifiStatus(".", 1);
    M5.delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    diplayWifiStatus("\r\nWiFi Connected.");
    diplayWifiStatus("NTP:", 1);
#if SNTP_ENABLED
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
    {
      diplayWifiStatus(".", 2);
      M5.delay(1000);
    }
#else
    M5.delay(1600);
    struct tm timeInfo;
    while (!getLocalTime(&timeInfo, 1000))
    {
      diplayWifiStatus('.', 2);
    };
#endif
    diplayWifiStatus("\r\nNTP Connected.", 1);

    time_t t = time(nullptr)+1; // Advance one second.
    while (t > time(nullptr));  /// Synchronization in seconds
    M5.Rtc.setDateTime( localtime( &t ) );
  }
  else
  {
    diplayWifiStatus("\r\nWiFi none...");
  }

  M5.Display.clear();
}

static constexpr const uint32_t color_tbl[18] = 
{
0xFF0000u, 0xCCCC00u, 0xCC00FFu,
0xFFCC00u, 0x00FF00u, 0x0088FFu,
0xFF00CCu, 0x00FFCCu, 0x0000FFu,
0xFF0000u, 0xCCCC00u, 0xCC00FFu,
0xFFCC00u, 0x00FF00u, 0x0088FFu,
0xFF00CCu, 0x00FFCCu, 0x0000FFu,
};
static constexpr const float coefficient_tbl[3] = { 0.5f, (1.0f / 256.0f), (1.0f / 1024.0f) };
static int prev_xpos[18];

void drawBar(int32_t ox, int32_t oy, int32_t nx, int32_t px, int32_t h, uint32_t color)
{
  uint32_t bgcolor = (color >> 3) & 0x1F1F1Fu;
  if (px && ((nx < 0) != (px < 0)))
  {
    M5.Display.fillRect(ox, oy, px, h, bgcolor);
    px = 0;
  }
  if (px != nx)
  {
    if ((nx > px) != (nx < 0))
    {
      bgcolor = color;
    }
    M5.Display.setColor(bgcolor);
    M5.Display.fillRect(nx + ox, oy, px - nx, h);
  }
}


void drawGraph(const rect_t& r, const m5::imu_data_t& data)
{
  float aw = (128 * r.w) >> 1;
  float gw = (128 * r.w) / 256.0f;
  float mw = (128 * r.w) / 1024.0f;
  int ox = (r.x + r.w)>>1;
  int oy = r.y;
  int h = (r.h / 18);
  int bar_count = 9;

  M5.Display.startWrite();
  for (int index = 0; index < bar_count; ++index)
  {
    float xval;
    if (index < 9)
    {
      auto coe = coefficient_tbl[index / 3] * r.w;
      xval = data.value[index] * coe;
    }
    else
    {
      xval = M5.Imu.getOffsetData(index - 9) * (1.0f / (1 << 19));
    }

    // for Linear scale graph.
    float tmp = xval;

    // The smaller the value, the larger the amount of change in the graph.
//  float tmp = sqrtf(fabsf(xval * 128)) * (signbit(xval) ? -1 : 1);

    int nx = tmp;
    int px = prev_xpos[index];
    if (nx != px)
    prev_xpos[index] = nx;
    drawBar(ox, oy + h * index, nx, px, h - 1, color_tbl[index]);
  }
  M5.Display.endWrite();
}

void showIMU(){
  auto imu_update = M5.Imu.update();
  int dataOffset = M5.Display.width()/2;
  static rect_t rect_graph_area = {M5.Display.width()/2, lineH*20, M5.Display.width()/2, 200 };
  auto data = M5.Imu.getImuData();
  drawGraph(rect_graph_area, data);

  char buffer[20];

  float list = atan2(-data.accel.x, sqrt(data.accel.y * data.accel.y + data.accel.z * data.accel.z)) * 180.0 / PI;
  float trim = atan2(data.accel.y, data.accel.z) * 180.0 / PI;

  titleField.show("Gyro");

  snprintf(buffer, sizeof(buffer), "  List : %.1f  ", list);
  body1.show(buffer);
  snprintf(buffer, sizeof(buffer), "  Trim : %.1f  ", trim);
  body2.show(buffer);

  snprintf(buffer, sizeof(buffer), "L:%.1f T:%.1f", list, trim);
  dataToSend = buffer;
  M5.delay(100);
}
