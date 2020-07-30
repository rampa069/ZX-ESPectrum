// Copyright (c) 2020 Daiki Yasuda
//
// This is licensed under
// - Creative Commons Attribution-NonCommercial 3.0 Unported
// - https://creativecommons.org/licenses/by-nc/3.0/
// - Or see LICENSE.md
//
// The short of it is...
//   You are free to:
//     Share — copy and redistribute the material in any medium or format
//     Adapt — remix, transform, and build upon the material
//   Under the following terms:
//     NonCommercial — You may not use the material for commercial purposes.

#ifndef __ESP32_WIIMOTE_H__
#define __ESP32_WIIMOTE_H__

#include "esp_bt.h"
#include "TinyWiimote.h"

typedef struct {
        uint8_t xStick;
        uint8_t yStick;
        uint8_t xAxis;
        uint8_t yAxis;
        uint8_t zAxis;
        uint8_t cBtn;
        uint8_t zBtn;
} NunchukState;

enum
{
  FILTER_NONE                = 0x0000,
  FILTER_REMOTE_BUTTON       = 0x0001,
  FILTER_NUNCHUK_BUTTON      = 0x0002,
  FILTER_NUNCHUK_STICK       = 0x0004,
  FILTER_NUNCHUK_ACCEL       = 0x0008,
};

enum
{
  ACTION_IGNORE,
};

class ESP32Wiimote
{
public:
  enum
  {
    BUTTON_LEFT       = 0x0800,
    BUTTON_RIGHT      = 0x0400,
    BUTTON_UP         = 0x0200,
    BUTTON_DOWN       = 0x0100,
    BUTTON_A          = 0x0008,
    BUTTON_B          = 0x0004,
    BUTTON_PLUS       = 0x1000,
    BUTTON_HOME       = 0x0080,
    BUTTON_MINUS      = 0x0010,
    BUTTON_ONE        = 0x0002,
    BUTTON_TWO        = 0x0001
  };
    
  const int NUNCHUK_STICK_THRESHOLD = 2;

  ESP32Wiimote(void);

  void init(void);
  void task(void);
  int available(void);
  uint16_t getButtonState(void);
  NunchukState getNunchukState(void);
  void addFilter(int action, int filter);

private:

  typedef struct {
          size_t len;
          uint8_t data[];
  } queuedata_t;

  TinyWiimoteData _gotData;

  uint16_t _buttonState;
  uint16_t _oldButtonState;

  NunchukState *_pNunchukState;
  NunchukState *_pOldNunchukState;

  NunchukState _nunchukStateA;
  NunchukState _nunchukStateB;

  int _nunStickThreshold;

  int _filter;

  static const TwHciInterface tinywii_hci_interface;
  static esp_vhci_host_callback_t vhci_callback;
  static xQueueHandle txQueue;
  static xQueueHandle rxQueue;

  static void createQueue(void);
  static void handleTxQueue(void);
  static void handleRxQueue(void);
  static esp_err_t sendQueueData(xQueueHandle queue, uint8_t *data, size_t len);
  static void notifyHostSendAvailable(void);
  static int notifyHostRecv(uint8_t *data, uint16_t len);
  static void hciHostSendPacket(uint8_t *data, size_t len);

};

#endif // __ESP32_WIIMOTE_H__
