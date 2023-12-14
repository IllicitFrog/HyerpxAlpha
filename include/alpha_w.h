#pragma once

#include <cstring>
#include <hidapi/hidapi.h>
#include <iostream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <thread>
#include <wchar.h>

#define VENDOR_ID 0x03f0
#define PRODUCT_ID 0x098d
#define MAX_STR 255

namespace alpha_w {

enum connection_status { DISCONNECTED = 0, CONNECTED = 1, ABSENT = 2 };
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
  headset() : handle(NULL) {
    std::cout << "Initializing HIDAPI" << std::endl;
    if (hid_init() < 0) {
      throw std::runtime_error("hid_init failed");
    };
    std::cout << "Opening device" << std::endl;
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
    t.join();
    hid_close(handle);
    hid_exit();
  }

  void run() {
    t = std::thread(&headset::read_loop, this);
    send_command(commands::CONNECTION_STATE);
    send_command(commands::GET_INDENTIFIER);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (status == connection_status::CONNECTED) {
      send_command(commands::STATUS_REQUEST);
      send_command(commands::PING);
    }
  }

  void send_command(commands cmd) {
    unsigned char bytes[4];
    bytes[0] = ((long)cmd >> 24) & 0xff;
    bytes[1] = ((long)cmd >> 16) & 0xff;
    bytes[2] = ((long)cmd >> 8) & 0xff;
    bytes[3] = (long)cmd & 0xff;
    hid_write(handle, bytes, 4);
  }


private:
  unsigned long identifier;
  std::wstring manufacturer;
  std::wstring product;
  std::wstring serial;
  sleep_time sleep;
  connection_status status;
  unsigned int battery;
  bool muted;
  bool voice;
  bool mic_monitor;
  hid_device *handle;
  std::thread t;

  void read_loop() {
    unsigned char buffer[32] = {0};
    while (1) {
      hid_read(handle, buffer, sizeof(buffer));
      // information received
      if (buffer[0] == 0x21 && buffer[1] == 0xbb) {
        switch (buffer[2]) {
          // status update
        case 0x03:
          if (buffer[3] == 0x00) {
            status = connection_status::ABSENT;
          } else if (buffer[3] == 0x01) {
            status = connection_status::DISCONNECTED;
          } else if (buffer[3] == 0x02) {
            status = connection_status::CONNECTED;
          }
          break;
        case 0x05:
          // same dont know
          break;
        case 0x07:
          if (buffer[3] == 0x14) {
            // dont know what this does
          }
          break;
        case 0x09:
          // don't know
          break;
        case 0x0a:
          // again no idea
          break;
        case 0x0b:
          battery = (unsigned int)buffer[3];
          std::cout << "Battery: " << battery << std::endl;
          break;
        case 0x0c:
          // response to 0c
          break;
        case 0x0d:
          identifier =
              (unsigned long)buffer[3] << 40 | (unsigned long)buffer[4] << 32 |
              (unsigned long)buffer[5] << 24 | (unsigned long)buffer[6] << 16 |
              (unsigned long)buffer[7] << 8 | (unsigned long)buffer[8];
          std::cout << "Identifier: " << identifier << std::endl;
          break;
        case 0x10:
          if (buffer[3] == 0x00 && buffer[4] == 0x01) {
            voice = false;
          } else if (buffer[3] == 0x01 && buffer[4] == 0x01) {
            voice = true;
          }
          break;
        case 0x12:
          if (buffer[3] == 0x0a && buffer[4] == 0x01) {
            sleep = S10;
          } else if (buffer[3] == 0x14 && buffer[4] == 0x01) {
            sleep = S20;
          } else if (buffer[3] == 0x1e && buffer[4] == 0x01) {
            sleep = S30;
          }
          break;
        case 0x13:
          if (buffer[3] == 0x00) {
            voice = false;
          } else if (buffer[3] == 0x01) {
            voice = true;
          }
          break;
        case 0x22:
          if (buffer[3] == 0x00) {
            mic_monitor = false;
          } else if (buffer[3] == 0x01) {
            mic_monitor = true;
          }
          break;
        case 0x23:
          if (buffer[3] == 0x00) {
            muted = false;
          } else if (buffer[3] == 0x01) {
            muted = true;
          }
          break;
        case 0x24:
          if (buffer[3] == 0x01) {
            status = DISCONNECTED;
          } else if (buffer[3] == 0x02) {
            status = CONNECTED;
          }
          break;
        }
      }
      memset(buffer, 0, sizeof(buffer));
    }
  }
};
} // namespace alpha_w
