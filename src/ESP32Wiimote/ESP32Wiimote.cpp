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

#define CONFIG_CLASSIC_BT_ENABLED

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"
#include "esp_bt.h"
#include <HardwareSerial.h>

#include "time.h"
#include "sys/time.h"

#include "ESP32Wiimote.h"
#include "TinyWiimote.h"

#define WIIMOTE_VERBOSE 0

#if WIIMOTE_VERBOSE
#define VERBOSE_PRINT(...) Serial.printf(__VA_ARGS__)
#define VERBOSE_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define VERBOSE_PRINT(...) do {} while(0)
#define VERBOSE_PRINTLN(...) do {} while(0)
#endif

#define RX_QUEUE_SIZE 32
#define TX_QUEUE_SIZE 32
xQueueHandle ESP32Wiimote::rxQueue = NULL;
xQueueHandle ESP32Wiimote::txQueue = NULL;

const TwHciInterface ESP32Wiimote::tinywii_hci_interface = {
  ESP32Wiimote::hciHostSendPacket
};

esp_vhci_host_callback_t ESP32Wiimote::vhci_callback;

ESP32Wiimote::ESP32Wiimote(void)
{
    _pNunchukState = &_nunchukStateA;
    _pOldNunchukState = &_nunchukStateB;
    _nunStickThreshold = NUNCHUK_STICK_THRESHOLD * NUNCHUK_STICK_THRESHOLD;
    _filter = FILTER_NONE;
}

void ESP32Wiimote::notifyHostSendAvailable(void) {
  VERBOSE_PRINT("notifyHostSendAvailable\n");
  if(!TinyWiimoteDeviceIsInited()){
    TinyWiimoteResetDevice();
  }
}

void ESP32Wiimote::createQueue(void) {
  txQueue = xQueueCreate(TX_QUEUE_SIZE, sizeof(queuedata_t*));
  if (txQueue == NULL){
    VERBOSE_PRINTLN("xQueueCreate(txQueue) failed");
    return;
  }
  rxQueue = xQueueCreate(RX_QUEUE_SIZE, sizeof(queuedata_t*));
  if (rxQueue == NULL){
    VERBOSE_PRINTLN("xQueueCreate(rxQueue) failed");
    return;
  }
}

void ESP32Wiimote::handleTxQueue(void) {
  if(uxQueueMessagesWaiting(txQueue)){
    bool ok = esp_vhci_host_check_send_available();
    VERBOSE_PRINT("esp_vhci_host_check_send_available=%d", ok);
    if(ok){
      queuedata_t *queuedata = NULL;
      if(xQueueReceive(txQueue, &queuedata, 0) == pdTRUE){
        esp_vhci_host_send_packet(queuedata->data, queuedata->len);
        VERBOSE_PRINT("SEND => %s", format2Hex(queuedata->data, queuedata->len));
        free(queuedata);
      }
    }
  }
}

void ESP32Wiimote::handleRxQueue(void) {
  if(uxQueueMessagesWaiting(rxQueue)){
    queuedata_t *queuedata = NULL;
    if(xQueueReceive(rxQueue, &queuedata, 0) == pdTRUE){
      handleHciData(queuedata->data, queuedata->len);
      free(queuedata);
    }
  }
}

esp_err_t ESP32Wiimote::sendQueueData(xQueueHandle queue, uint8_t *data, size_t len) {
    VERBOSE_PRINTLN("sendQueueData");
    if(!data || !len){
        VERBOSE_PRINTLN("no data");
        return ESP_OK;
    }
    queuedata_t * queuedata = (queuedata_t*)malloc(sizeof(queuedata_t) + len);
    if(!queuedata){
        VERBOSE_PRINTLN("malloc failed");
        return ESP_FAIL;
    }
    queuedata->len = len;
    memcpy(queuedata->data, data, len);
    if (xQueueSend(queue, &queuedata, portMAX_DELAY) != pdPASS) {
        VERBOSE_PRINTLN("xQueueSend failed");
        free(queuedata);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void ESP32Wiimote::hciHostSendPacket(uint8_t *data, size_t len) {
  sendQueueData(txQueue, data, len);
}

int ESP32Wiimote::notifyHostRecv(uint8_t *data, uint16_t len) {
  VERBOSE_PRINT("notifyHostRecv:");
  for (int i = 0; i < len; i++)
  {
    VERBOSE_PRINT(" %02x", data[i]);
  }
  VERBOSE_PRINTLN("");

  if(ESP_OK == sendQueueData(rxQueue, data, len)){
    return ESP_OK;
  }else{
    return ESP_FAIL;
  }
}

void ESP32Wiimote::init(void)
{
    TinyWiimoteInit(tinywii_hci_interface);
    createQueue();
    vhci_callback.notify_host_recv = notifyHostRecv;
    vhci_callback.notify_host_send_available = notifyHostSendAvailable;    

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (!btStart()) {
        Serial.printf( "btStart() failed\n");
        return;
    }

    esp_err_t ret;
    if ((ret = esp_vhci_host_register_callback(&vhci_callback)) != ESP_OK) {
        return;
    }
}

void ESP32Wiimote::task(void)
{
  if(!btStarted()){
    return;
  }
  handleTxQueue();
  handleRxQueue();
}

int ESP32Wiimote::available(void)
{
    int stateIsAvailable = TinyWiimoteAvailable();
    if (stateIsAvailable) {
      NunchukState *pTmpNunchuck;

      _gotData = TinyWiimoteRead();

      // update old button state
      _oldButtonState = _buttonState;

      // update button state
      _buttonState = 0;
      _buttonState = (_gotData.data[TWII_OFFSET_BTNS1] << 8) | _gotData.data[TWII_OFFSET_BTNS2];

      // update old nunchuck state(= exchange nunchuk state area)
      pTmpNunchuck =  _pOldNunchukState;
      _pOldNunchukState =  _pNunchukState;
      _pNunchukState =  pTmpNunchuck;

      // update nunchuk state
      _pNunchukState->xStick = _gotData.data[TWII_OFFSET_EXTCTRL + 0];
      _pNunchukState->yStick = _gotData.data[TWII_OFFSET_EXTCTRL + 1];
      _pNunchukState->xAxis = _gotData.data[TWII_OFFSET_EXTCTRL + 2];
      _pNunchukState->yAxis = _gotData.data[TWII_OFFSET_EXTCTRL + 3];
      _pNunchukState->zAxis = _gotData.data[TWII_OFFSET_EXTCTRL + 4];
      _pNunchukState->cBtn = ((_gotData.data[TWII_OFFSET_EXTCTRL + 5] & 0x02) >> 1) ^ 0x01;
      _pNunchukState->zBtn = (_gotData.data[TWII_OFFSET_EXTCTRL + 5] & 0x01) ^ 0x01;

      // check button change
      int buttonIsChanged = false;
      if (_filter & FILTER_REMOTE_BUTTON) {
        ; // ignore
      }
      else if (_buttonState != _oldButtonState) {
        buttonIsChanged = true;
      }

      // check nunchuk stick change
      int nunchukStickIsChanged = false;
      int nunXStickDelta = (int)(_pNunchukState->xStick) - _pOldNunchukState->xStick;
      int nunYStickDelta = (int)(_pNunchukState->yStick) - _pOldNunchukState->yStick;
      int nunStickDelta = (nunXStickDelta*nunXStickDelta + nunYStickDelta*nunYStickDelta) / 2;
      if (_filter & FILTER_NUNCHUK_STICK) {
        ; // ignore
      }
      else if (nunStickDelta >= _nunStickThreshold) {
        nunchukStickIsChanged = true;
      }

      // check nunchuk button change
      int nunchukButtonIsChanged = false;
      if (_filter & FILTER_NUNCHUK_BUTTON) {
        ; // ignore
      }
      else if (
        (_pNunchukState->cBtn != _pOldNunchukState->cBtn)
        || (_pNunchukState->zBtn != _pOldNunchukState->zBtn)
      ) {
        nunchukButtonIsChanged = true;
      }

      // check accel change
      int accelIsChanged = false;
      if (_filter & FILTER_NUNCHUK_ACCEL) {
        ; // ignore
      }
      else {
        accelIsChanged = true;
      }

      stateIsAvailable = 
        ( buttonIsChanged
        | nunchukStickIsChanged
        | nunchukButtonIsChanged
        | accelIsChanged
        );

    }
    return stateIsAvailable;
}

uint16_t ESP32Wiimote::getButtonState(void)
{
  return _buttonState;
}

NunchukState ESP32Wiimote::getNunchukState(void)
{
  return *_pNunchukState;
}

void ESP32Wiimote::addFilter(int action, int filter) {
  if (action == ACTION_IGNORE) {
    _filter = _filter | filter;
  }
}

