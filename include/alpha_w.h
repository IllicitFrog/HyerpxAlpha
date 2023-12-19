#ifndef __ALPHA_W_H
#define __ALPHA_W_H

#include <cstring>
#include <hidapi/hidapi.h>
#include <iostream>
#include <stdexcept>
#include <wchar.h>
#include <array>

#define VENDOR_ID 0x03f0
#define PRODUCT_ID 0x098d
#define MAX_STR 255

enum connection_status { DISCONNECTED = 0, CONNECTED = 1 };
enum sleep_time { S10 = 10, S20 = 20, S30 = 30 };
enum class commands : int {
  STATUS_REQUEST = 0x21bb0b00,
  PING = 0x21bb0c00,
  GET_INDENTIFIER = 0x21bb0d00,
  CONNECTION_STATE = 0x21bb0300,
  MUTE_MICROPHONE = 0x21bb0200,
  UNMUTE_MICROPHONE = 0x21bb0201,
  VOICE_PROMPTS = 0x21bb1300,
  VOICE_PROMPTS_OFF = 0x21bb1310,
  MICROPHONE_MONITOR = 0x21bb1001,
  MICROPHONE_MONITOR_OFF = 0x21bb1002,
  SLEEP_TIMER_10 = 0x21bb120a,
  SLEEP_TIMER_20 = 0x21bb1214,
  SLEEP_TIMER_30 = 0x21bb121e
};

class headset {
public:
  headset() {
    if (hid_init() < 0) {
      throw std::runtime_error("Failed to initialize HIDAPI");
    };
    std::cout << "HIDAPI version:" << hid_version_str() << std::endl;
    handle = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
    if (handle == NULL) {
      throw std::runtime_error("Unable to open device");
    }
    int res = 0;
    wchar_t man[MAX_STR], prod[MAX_STR], ser[MAX_STR];
    res += hid_get_manufacturer_string(handle, man, MAX_STR);
    res += hid_get_product_string(handle, prod, MAX_STR);
    res += hid_get_serial_number_string(handle, ser, MAX_STR);
    if (res < 0) {
      throw std::runtime_error("Unable to read device strings");
    }
    manufacturer = std::wstring(man);
    product = std::wstring(prod);
    serial = std::wstring(ser);
    std::wcout << "Manufacturer: " << manufacturer << std::endl;
    std::wcout << "Product: " << product << std::endl;
    std::wcout << "Serial: " << serial << std::endl;
  }

  ~headset() {
    hid_close(handle);
    hid_exit();
  }

  void read(unsigned char* buffer) { 
    hid_read(handle, buffer, 32); 
  }

  void send_command(commands cmd) {
    unsigned char bytes[4];
    bytes[0] = ((long)cmd >> 24) & 0xff;
    bytes[1] = ((long)cmd >> 16) & 0xff;
    bytes[2] = ((long)cmd >> 8) & 0xff;
    bytes[3] = (long)cmd & 0xff;
    hid_write(handle, bytes, 4);
  }

  std::wstring print_manufacturer() { return manufacturer; }
  std::wstring print_product() { return product; }
  std::wstring print_serial() { return serial; }

private:
  std::wstring manufacturer;
  std::wstring product;
  std::wstring serial;
  hid_device *handle;
};
#endif
