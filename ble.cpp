#define US_KEYBOARD 1
#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include "device_name.h"
#include "ble.h"
#include "field.h"
#include "BluefruitService.h"

#define UART_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"

bool isBleConnected = false;
extern String dataToSend;
extern Field bleField;


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
BLEServer* keyboardServer = nullptr;
static TaskHandle_t bluetoothTaskHandle = nullptr;

namespace {
struct BleConnection {
    bool active;
    esp_gatt_if_t gattsIf;
    uint16_t connectionId;
};

constexpr size_t MAX_TRACKED_CONNECTIONS = 4;
BleConnection trackedConnections[MAX_TRACKED_CONNECTIONS] = {};
portMUX_TYPE connectionMux = portMUX_INITIALIZER_UNLOCKED;

void trackGattConnection(
    esp_gatts_cb_event_t event,
    esp_gatt_if_t gattsIf,
    esp_ble_gatts_cb_param_t* param) {
    bool connected = false;
    bool connectionChanged = false;
    portENTER_CRITICAL(&connectionMux);
    if (event == ESP_GATTS_CONNECT_EVT) {
        for (BleConnection& connection : trackedConnections) {
            if (!connection.active) {
                connection = {true, gattsIf, param->connect.conn_id};
                connectionChanged = true;
                break;
            }
        }
        connected = true;
    } else if (event == ESP_GATTS_DISCONNECT_EVT) {
        for (BleConnection& connection : trackedConnections) {
            if (connection.active &&
                connection.connectionId == param->disconnect.conn_id) {
                connection.active = false;
                connectionChanged = true;
            }
        }
    }
    portEXIT_CRITICAL(&connectionMux);

    if (connectionChanged) {
        isBleConnected = connected;
        BluefruitService::getInstance().setConnected(connected);
        if (input != nullptr) {
            BLE2902* descriptor = static_cast<BLE2902*>(
                input->getDescriptorByUUID(BLEUUID(static_cast<uint16_t>(0x2902))));
            if (descriptor != nullptr) {
                descriptor->setNotifications(connected);
            }
        }
        if (!connected && BLEDevice::getInitialized()) {
            BLEDevice::startAdvertising();
        }
    }
}
}

const InputReport NO_KEY_PRESSED = { };


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


bool startBleKeyboardOnServer(BLEServer* server, bool combinedMode) {
    if (server == nullptr) {
        return false;
    }
    if (combinedMode) {
        BLEDevice::stopAdvertising();
    }
    keyboardServer = server;
    // Initialize Bluefruit Service (UART)
    BluefruitService::getInstance().init(keyboardServer, !combinedMode);

    // create an HID device
    hid = new BLEHIDDevice(keyboardServer);
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
    // hid->setBatteryLevel(40);

    // advertise the services
    BLEAdvertising* advertising = keyboardServer->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);

    if (combinedMode) {
        BLEAdvertisementData scanResponseData;
        scanResponseData.setCompleteServices(BLEUUID(hid->hidService()->getUUID()));
        scanResponseData.setCompleteServices(BLEUUID(UART_SERVICE_UUID));
        scanResponseData.setShortName("Sensor");
        if (!advertising->setScanResponseData(scanResponseData)) {
            return false;
        }
    } else {
        BLEAdvertisementData advertisementData;
        advertisementData.setFlags(0x06);
        advertisementData.setAppearance(HID_KEYBOARD);
        advertisementData.setCompleteServices(BLEUUID(hid->hidService()->getUUID()));
        advertisementData.setName(DEVICE_NAME);
        if (!advertising->setAdvertisementData(advertisementData)) {
            return false;
        }

        BLEAdvertisementData scanResponseData;
        scanResponseData.setCompleteServices(BLEUUID(UART_SERVICE_UUID));
        scanResponseData.setServiceData(BLEUUID(hid->batteryService()->getUUID()), "\x64");
        if (!advertising->setScanResponseData(scanResponseData)) {
            return false;
        }
    }

    return advertising->start();
}

void bluetoothTask(void*) {
    BLEDevice::init(DEVICE_NAME);
    trackBleConnections();
    keyboardServer = BLEDevice::createServer();
    if (!startBleKeyboardOnServer(keyboardServer)) {
        Serial.println("Failed to initialize BLE keyboard services");
        bluetoothTaskHandle = nullptr;
        vTaskDelete(nullptr);
        return;
    }

    bleField.show("  R");
    delay(portMAX_DELAY);
};

bool startBluetoothTask() {
    if (bluetoothTaskHandle != nullptr) {
        return true;
    }

    return xTaskCreate(
               bluetoothTask, "bluetooth", 20000, nullptr, 5,
               &bluetoothTaskHandle) == pdPASS;
}

void stopBluetoothTask() {
    if (bluetoothTaskHandle == nullptr) {
        return;
    }

    vTaskDelete(bluetoothTaskHandle);
    bluetoothTaskHandle = nullptr;
}

void trackBleConnections() {
    portENTER_CRITICAL(&connectionMux);
    memset(trackedConnections, 0, sizeof(trackedConnections));
    portEXIT_CRITICAL(&connectionMux);
    BLEDevice::setCustomGattsHandler(trackGattConnection);
}

void resetBleKeyboardState() {
    keyboardServer = nullptr;
    hid = nullptr;
    input = nullptr;
    output = nullptr;
    isBleConnected = false;
    BluefruitService::getInstance().setConnected(false);
}

bool disconnectBleClients(uint32_t timeoutMs) {
    if (!BLEDevice::getInitialized()) {
        return true;
    }

    const unsigned long startedAt = millis();
    for (;;) {
        BLEDevice::stopAdvertising();
        delay(50);

        BleConnection connections[MAX_TRACKED_CONNECTIONS];
        portENTER_CRITICAL(&connectionMux);
        memcpy(connections, trackedConnections, sizeof(connections));
        portEXIT_CRITICAL(&connectionMux);

        bool anyConnected = false;
        for (const BleConnection& connection : connections) {
            if (connection.active) {
                anyConnected = true;
                const esp_err_t result = esp_ble_gatts_close(
                    connection.gattsIf, connection.connectionId);
                if (result != ESP_OK) {
                    Serial.printf("BLE disconnect request failed: %d\n", result);
                }
            }
        }

        if (!anyConnected) {
            delay(100);
            portENTER_CRITICAL(&connectionMux);
            for (const BleConnection& connection : trackedConnections) {
                anyConnected = anyConnected || connection.active;
            }
            portEXIT_CRITICAL(&connectionMux);
            if (!anyConnected) {
                BLEDevice::stopAdvertising();
                return true;
            }
        }

        if (millis() - startedAt >= timeoutMs) {
            Serial.println("Timed out waiting for BLE clients to disconnect");
            return false;
        }
        delay(10);
    }
}

bool disableBleKeyboard() {
    if (!disconnectBleClients()) {
        return false;
    }

    if (BLEDevice::getInitialized()) {
        BLEDevice::deinit(false);
    }
    resetBleKeyboardState();
    return true;
}


void typeText(String t) {
    if(!isBleConnected) return;
    // int len = strlen(text);
    String text = t+"\n";
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
    bleField.show("  C");
  }else{
    bleField.show("  D");
  }
}