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

#include "time.h"
#include "sys/time.h"

#include "TinyWiimote.h"

#define WIIMOTE_VERBOSE 0

#if WIIMOTE_VERBOSE
#include <HardwareSerial.h> // for Arduino
#define VERBOSE_PRINT(...) Serial.printf(__VA_ARGS__)
#define VERBOSE_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define VERBOSE_PRINT(...) do {} while(0)
#define VERBOSE_PRINTLN(...) do {} while(0)
#endif

#define HCI_H4_CMD_PREAMBLE_SIZE           (4)
#define HCI_H4_ACL_PREAMBLE_SIZE           (5)

#define BD_ADDR_LEN     (6)
struct bd_addr_t {
  uint8_t addr[BD_ADDR_LEN];
};

#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (uint8_t)(u8);}
#define BDADDR_TO_STREAM(p, a)   {int ijk; for (ijk = 0; ijk < BD_ADDR_LEN;  ijk++) *(p)++ = (uint8_t) a[BD_ADDR_LEN - 1 - ijk];}
#define STREAM_TO_BDADDR(a, p)   {int ijk; for (ijk = 0; ijk < BD_ADDR_LEN;  ijk++) a[BD_ADDR_LEN - 1 - ijk] = (p)[ijk];}
#define ARRAY_TO_STREAM(p, a, len) {int ijk; for (ijk = 0; ijk < len;        ijk++) *(p)++ = (uint8_t) a[ijk];}

enum {
    H4_TYPE_COMMAND = 1,
    H4_TYPE_ACL     = 2,
    H4_TYPE_SCO     = 3,
    H4_TYPE_EVENT   = 4
};

// L2CAP
#define L2CAP_CONNECT_RES			0x03
#define L2CAP_CONFIG_RES			0x05
#define L2CAP_CONFIG_REQ			0x04

// BTCODE
#define BTCODE_HID			0xA1

// HCI Events
#define HCI_INQUIRY_COMP_EVT            0x01
#define HCI_INQUIRY_RESULT_EVT          0x02
#define HCI_CONNECTION_COMP_EVT         0x03
#define HCI_DISCONNECTION_COMP_EVT      0x05
#define HCI_RMT_NAME_REQUEST_COMP_EVT   0x07
#define HCI_QOS_SETUP_COMP_EVT          0x0D
#define HCI_COMMAND_COMPLETE_EVT        0x0E
#define HCI_COMMAND_STATUS_EVT          0x0F
#define HCI_NUM_COMPL_DATA_PKTS_EVT     0x13

// HCI Command opcode group field(OGF) & Opcode Command Field (OCF)
// refer : http://software-dl.ti.com/lprf/simplelink_cc26x2_sdk-1.60/docs/ble5stack/vendor_specific_guide/BLE_Vendor_Specific_HCI_Guide/hci_interface.html

// Opcode Group Field (OGF) codes
#define HCI_OGF_LINK_CONTROL                 0x01  // Link control group
#define HCI_OGF_CONTROL_BASEBAND             0x03  // Host Controller & Baseband group
#define HCI_OGF_INFORMATIONAL_PARAMETERS     0x04  // Information parameters group

// Host controller & baseband commands
#define HCI_OCF_RESET                        0x0003
#define HCI_OCF_CHANGE_LOCAL_NAME            0x0013
#define HCI_OCF_WRITE_CLASS_OF_DEVICE        0x0024
#define HCI_OCF_WRITE_SCAN_ENABLE            0x001A

// Informational parameter commands
#define HCI_OCF_READ_BD_ADDR                 0x0009

// Link control commands
#define HCI_OCF_INQUIRY                      0x0001
#define HCI_OCF_INQUIRY_CANCEL               0x0002
#define HCI_OCF_CREATE_CONNECTION            0x0005
#define HCI_OCF_REMOTE_NAME_REQUEST          0x0019

// HCI Command opcodes(OGF + OCF)
#define HCI_OPCODE_RESET                          (HCI_OCF_RESET | (HCI_OGF_CONTROL_BASEBAND << 10))
#define HCI_OPCODE_WRITE_LOCAL_NAME               (HCI_OCF_CHANGE_LOCAL_NAME | (HCI_OGF_CONTROL_BASEBAND << 10))
#define HCI_OPCODE_WRITE_CLASS_OF_DEVICE          (HCI_OCF_WRITE_CLASS_OF_DEVICE | (HCI_OGF_CONTROL_BASEBAND << 10))
#define HCI_OPCODE_WRITE_SCAN_ENABLE              (HCI_OCF_WRITE_SCAN_ENABLE | (HCI_OGF_CONTROL_BASEBAND << 10))
#define HCI_OPCODE_READ_BD_ADDR                   (HCI_OCF_READ_BD_ADDR | (HCI_OGF_INFORMATIONAL_PARAMETERS << 10))
#define HCI_OPCODE_INQUIRY                        (HCI_OCF_INQUIRY | (HCI_OGF_LINK_CONTROL << 10))
#define HCI_OPCODE_INQUIRY_CANCEL                 (HCI_OCF_INQUIRY_CANCEL | (HCI_OGF_LINK_CONTROL << 10))
#define HCI_OPCODE_CREATE_CONNECTION              (HCI_OCF_CREATE_CONNECTION | (HCI_OGF_LINK_CONTROL << 10))
#define HCI_OPCODE_REMOTE_NAME_REQUEST            (HCI_OCF_REMOTE_NAME_REQUEST | (HCI_OGF_LINK_CONTROL << 10))

#define HCIC_PARAM_SIZE_WRITE_LOCAL_NAME (248)
#define HCIC_PARAM_SIZE_WRITE_CLASS_OF_DEVICE (3)
#define HCIC_PARAM_SIZE_WRITE_SCAN_ENABLE (1)
#define HCIC_PARAM_SIZE_CREATE_CONNECTION (13)
#define HCIC_PARAM_SIZE_REMOTE_NAME_REQUEST (10)
#define HCIC_PARAM_SIZE_WRITE_INQUIRY_CANCEL (0)
#define HCIC_PARAM_SIZE_WRITE_INQUIRY (5)

static bool deviceInited = false;
static bool wiimoteConnected = false;

/**
 * Command Maker
 */
static uint16_t make_cmd_reset(uint8_t *buf)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_RESET);
    UINT8_TO_STREAM (buf, 0);
    return HCI_H4_CMD_PREAMBLE_SIZE;
}

static uint16_t make_cmd_read_bd_addr(uint8_t *buf)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_READ_BD_ADDR);
    UINT8_TO_STREAM (buf, 0);
    return HCI_H4_CMD_PREAMBLE_SIZE;
}

static uint16_t make_cmd_write_local_name(uint8_t *buf, uint8_t* name, uint8_t len)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_WRITE_LOCAL_NAME);
    UINT8_TO_STREAM (buf, HCIC_PARAM_SIZE_WRITE_LOCAL_NAME);
    ARRAY_TO_STREAM(buf, name, len);
    for(uint8_t i=len; i<HCIC_PARAM_SIZE_WRITE_LOCAL_NAME; i++){
      UINT8_TO_STREAM (buf, 0);
    }
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_WRITE_LOCAL_NAME;
}

static uint16_t make_cmd_write_class_of_device(uint8_t *buf, uint8_t* cod)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_WRITE_CLASS_OF_DEVICE);
    UINT8_TO_STREAM (buf, HCIC_PARAM_SIZE_WRITE_CLASS_OF_DEVICE);
    for(uint8_t i=0; i<HCIC_PARAM_SIZE_WRITE_CLASS_OF_DEVICE; i++){
      UINT8_TO_STREAM (buf, cod[i]);
    }
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_WRITE_CLASS_OF_DEVICE;
}

static uint16_t make_cmd_write_scan_enable(uint8_t *buf, uint8_t mode)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_WRITE_SCAN_ENABLE);
    UINT8_TO_STREAM (buf, HCIC_PARAM_SIZE_WRITE_SCAN_ENABLE);

    UINT8_TO_STREAM (buf, mode);
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_WRITE_SCAN_ENABLE;
}

static uint16_t make_cmd_inquiry(uint8_t *buf, uint32_t lap, uint8_t len, uint8_t num)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_INQUIRY);
    UINT8_TO_STREAM (buf, HCIC_PARAM_SIZE_WRITE_INQUIRY);

    UINT8_TO_STREAM (buf, (uint8_t)( lap      & 0xFF)); // lap 0x33 <- 0x9E8B33
    UINT8_TO_STREAM (buf, (uint8_t)((lap>> 8) & 0xFF)); // lap 0x8B
    UINT8_TO_STREAM (buf, (uint8_t)((lap>>16) & 0xFF)); // lap 0x9E
    UINT8_TO_STREAM (buf, len);
    UINT8_TO_STREAM (buf, num);
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_WRITE_INQUIRY;
}

static uint16_t make_cmd_inquiry_cancel(uint8_t *buf)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_INQUIRY_CANCEL);
    UINT8_TO_STREAM (buf, HCIC_PARAM_SIZE_WRITE_INQUIRY_CANCEL);

    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_WRITE_INQUIRY_CANCEL;
}

static uint16_t make_cmd_remote_name_request(uint8_t *buf, struct bd_addr_t bdAddr, uint8_t psrm, uint16_t clkofs)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_REMOTE_NAME_REQUEST);
    UINT8_TO_STREAM (buf, HCIC_PARAM_SIZE_REMOTE_NAME_REQUEST);

    BDADDR_TO_STREAM (buf, bdAddr.addr);
    UINT8_TO_STREAM (buf, psrm);    // Page Scan Repetition Mode
    UINT8_TO_STREAM (buf, 0);       // Reserved
    UINT16_TO_STREAM (buf, clkofs); // Clock Offset
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_REMOTE_NAME_REQUEST;
}

static uint16_t make_cmd_create_connection(uint8_t *buf, struct bd_addr_t bdAddr, uint16_t pt, uint8_t psrm, uint16_t clkofs, uint8_t ars)
{
    UINT8_TO_STREAM (buf, H4_TYPE_COMMAND);
    UINT16_TO_STREAM (buf, HCI_OPCODE_CREATE_CONNECTION);
    UINT8_TO_STREAM (buf, HCIC_PARAM_SIZE_CREATE_CONNECTION);

    BDADDR_TO_STREAM (buf, bdAddr.addr);
    UINT16_TO_STREAM (buf, pt);     // Packet Type
    UINT8_TO_STREAM (buf, psrm);    // Page Scan Repetition Mode
    UINT8_TO_STREAM (buf, 0);       // Reserved
    UINT16_TO_STREAM (buf, clkofs); // Clock Offset
    UINT8_TO_STREAM (buf, ars);     // Allow Role Switch
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_CREATE_CONNECTION;
}

#define L2CAP_HEADER_LEN (4) //Length + Channel ID

static uint16_t make_l2cap_packet(uint8_t *buf, uint16_t channelID, uint8_t *data, uint16_t len) {
    UINT16_TO_STREAM (buf, len);
    UINT16_TO_STREAM (buf, channelID); // 0x0001=Signaling channel
    ARRAY_TO_STREAM (buf, data, len);
    return L2CAP_HEADER_LEN + len;
}

static uint16_t make_acl_l2cap_packet(uint8_t *buf, uint16_t ch, uint8_t pbf, uint8_t bf, uint16_t channelID, uint8_t *data, uint8_t len) {
  uint8_t* l2cap_buf = buf + HCI_H4_ACL_PREAMBLE_SIZE;
  uint16_t l2capLen = make_l2cap_packet(l2cap_buf, channelID, data, len);

  UINT8_TO_STREAM (buf, H4_TYPE_ACL);
  UINT8_TO_STREAM (buf, ch & 0xFF);
  UINT8_TO_STREAM (buf, ((ch >> 8) & 0x0F) | pbf << 4 | bf << 6);
  UINT16_TO_STREAM (buf, l2capLen);

  return HCI_H4_ACL_PREAMBLE_SIZE + l2capLen;
}

TwHciInterface _hciInterface;

static void sendHciPacket(uint8_t *data, size_t len) {
    VERBOSE_PRINTLN("sendHciPacket");
    _hciInterface.hci_send_packet(data, len);
}

static int findItemsInArray(uint8_t* array, size_t arraySize, size_t itemLength, uint8_t* data, size_t dataLength, size_t alignment) {
  for(int i=0; i<arraySize; i++){
    if(memcmp(array + (itemLength*i) + alignment, data, dataLength) == 0){
      return i;
    }
  }
  return -1;
}

#define FORMAT_HEX_MAX_BYTES 30
static char formatHexBuffer[FORMAT_HEX_MAX_BYTES*3 + 4];

char* format2Hex(uint8_t* data, uint16_t len) {
    for(uint16_t i=0; i<len && i<FORMAT_HEX_MAX_BYTES; i++){
        sprintf(formatHexBuffer+3*i, "%02X ", data[i]);
        formatHexBuffer[3*i+3] = '\0';
    }
    if(FORMAT_HEX_MAX_BYTES<len){
        sprintf(formatHexBuffer+3*FORMAT_HEX_MAX_BYTES, "...");
        formatHexBuffer[3*FORMAT_HEX_MAX_BYTES+3] = '\0';
    }
    return formatHexBuffer;
}

/**
 * Connected device list
 */
 struct connected_device_t {
  bd_addr_t bdAddr;
  uint8_t psrm;
  uint16_t clkofs;
};
static int connectedDeviceListNum = 0;
#define SCANNED_DEVICE_LIST_SIZE 16
static connected_device_t connectedDeviceList[SCANNED_DEVICE_LIST_SIZE];
static int findConnectedDevice(struct bd_addr_t bdAddr) {
  return findItemsInArray((uint8_t*)connectedDeviceList, connectedDeviceListNum, sizeof(connected_device_t), (uint8_t*)&bdAddr.addr, BD_ADDR_LEN, 0);
}
static int connected_device_add(struct connected_device_t connected_device) {
  if(SCANNED_DEVICE_LIST_SIZE == connectedDeviceListNum){
    return -1;
  }
  connectedDeviceList[connectedDeviceListNum++] = connected_device;
  return connectedDeviceListNum;
}
static void connected_device_clear(void) {
  connectedDeviceListNum = 0;
}

/**
 * L2CAP
 */
struct l2cap_connection_t {
  uint16_t ch;
  uint16_t remoteCID;
};
static int l2capConnectionSize = 0;
#define L2CAP_CONNECTION_LIST_SIZE 8
static l2cap_connection_t l2capConnectionList[L2CAP_CONNECTION_LIST_SIZE];
static int l2capFindConnection(uint16_t ch) {
  return findItemsInArray((uint8_t*)l2capConnectionList, l2capConnectionSize, sizeof(l2cap_connection_t), (uint8_t*)&ch, sizeof(uint16_t), 0);
}
static int l2capAddConnection(struct l2cap_connection_t connection) {
  if(L2CAP_CONNECTION_LIST_SIZE == l2capConnectionSize){
    return -1;
  }
  l2capConnectionList[l2capConnectionSize++] = connection;
  return l2capConnectionSize;
}
static void l2capClearConnection(void) {
  l2capConnectionSize = 0;
}

static uint8_t tmpQueueData[256];

static void resetDevice(void) {
  VERBOSE_PRINTLN("resetDevice");
  connected_device_clear();
  l2capClearConnection();
  uint16_t len = make_cmd_reset(tmpQueueData);
  sendHciPacket(tmpQueueData, len);
}

void TinyWiimoteResetDevice(void) {
  resetDevice();
  deviceInited = true;
}

/**
 * HCI Event Handler
 */
static void handleCommandCompleteEvent(uint8_t len, uint8_t* data) {
    VERBOSE_PRINTLN("handleCommandCompleteEvent");
    uint16_t cmdOpcode = (uint16_t)data[1] | ((uint16_t)data[2] << 8);

    switch(cmdOpcode){
      case HCI_OPCODE_RESET:
        if(data[3]==0x00){ // OK
          VERBOSE_PRINTLN("reset succeeded");
          uint16_t len = make_cmd_read_bd_addr(tmpQueueData);
          sendHciPacket(tmpQueueData, len);
          VERBOSE_PRINTLN("queued read_bd_addr");
        }else{
          VERBOSE_PRINTLN("reset failed");
        }
        break;
      case HCI_OPCODE_READ_BD_ADDR:
        if(data[3] == 0x00){ // OK
          VERBOSE_PRINT("read_bd_addr succeeded(BD_ADDR=%s)", format2Hex(data+4, 6));
          char name[] = "ESP32-BT-L2CAP";
          VERBOSE_PRINT("sizeof(name)=%d", sizeof(name));
          uint16_t len = make_cmd_write_local_name(tmpQueueData, (uint8_t*)name, sizeof(name));
          sendHciPacket(tmpQueueData, len);
          VERBOSE_PRINTLN("queued write_local_name");
        }else{
          VERBOSE_PRINTLN("read_bd_addr failed");
        }
        break;
      case HCI_OPCODE_WRITE_LOCAL_NAME:
        if(data[3] == 0x00){ // OK
          VERBOSE_PRINTLN("write_local_name succeeded");
          uint8_t cod[3] = {0x04, 0x05, 0x00};
          uint16_t len = make_cmd_write_class_of_device(tmpQueueData, cod);
          sendHciPacket(tmpQueueData, len);
          VERBOSE_PRINTLN("queued write_class_of_device");
        }else{
          VERBOSE_PRINTLN("write_local_name failed");
        }
        break;
      case HCI_OPCODE_WRITE_CLASS_OF_DEVICE:
        if(data[3] == 0x00){ // OK
          VERBOSE_PRINTLN("write_class_of_device succeeded");
          uint16_t len = make_cmd_write_scan_enable(tmpQueueData, 3);
          sendHciPacket(tmpQueueData, len);
          VERBOSE_PRINTLN("queued write_scan_enable");
        }else{
          VERBOSE_PRINTLN("write_class_of_device failed");
        }
        break;
      case HCI_OPCODE_WRITE_SCAN_ENABLE:
        if(data[3] == 0x00){ // OK
          VERBOSE_PRINTLN("write_scan_enable succeeded");

          connected_device_clear();
          uint16_t len = make_cmd_inquiry(tmpQueueData, 0x9E8B33, 0x05/*0x30*/, 0x00);
          sendHciPacket(tmpQueueData, len);
          VERBOSE_PRINTLN("queued inquiry");
        }else{
          VERBOSE_PRINTLN("write_scan_enable failed.");
        }
        break;
      case HCI_OPCODE_INQUIRY_CANCEL:
        if(data[3] == 0x00){ // OK
          VERBOSE_PRINTLN("inquiry_cancel succeeded");
        }else{
          VERBOSE_PRINTLN("inquiry_cancel failed");
        }
        break;
      default:
        VERBOSE_PRINTLN("UNKNOWN COMMAND EVENT");
        break;
    }
}

static void handleCommandStatusEvent(uint8_t len, uint8_t* data) {
    VERBOSE_PRINTLN("handleCommandStatusEvent");
    uint16_t cmdOpcode = (uint16_t)data[2] | ((uint16_t)data[3] << 8);

    switch(cmdOpcode){
      case HCI_OPCODE_INQUIRY:
        if(data[0] == 0x00){
          VERBOSE_PRINTLN("pending HCI_OPCODE_INQUIRY");
        }else{
          VERBOSE_PRINT("failed HCI_OPCODE_INQUIRY (error=%02X)", data[0]);
        }
        break;
      case HCI_OPCODE_REMOTE_NAME_REQUEST:
        if(data[0] == 0x00){
          VERBOSE_PRINTLN("pending HCI_OPCODE_REMOTE_NAME_REQUEST");
        }else{
          VERBOSE_PRINT("failed HCI_OPCODE_REMOTE_NAME_REQUEST (error=%02X)", data[0]);
        }
        break;
      case HCI_OPCODE_CREATE_CONNECTION:
        if(data[0] == 0x00){
          VERBOSE_PRINTLN("pending HCI_OPCODE_CREATE_CONNECTION");
        }else{
          VERBOSE_PRINT("failed HCI_OPCODE_CREATE_CONNECTION(error=%02X)", data[0]);
        }
        break;
      default:
        VERBOSE_PRINTLN("UNKNOWN STATUS EVENT");
        break;
    }

}

static void handleInquiryCompleteEvent(uint8_t len, uint8_t* data) {
    uint8_t status = data[0];
    VERBOSE_PRINT("inquiry_complete status=%02X", status);
    resetDevice();
}

static void handleInquiryResultEvent(uint8_t len, uint8_t* data) {
    uint8_t num = data[0];
    VERBOSE_PRINTLN("inquiry_result");

    for(int i=0; i<num; i++){
      int pos = 1 + (6+1+2+3+2) * i;

      struct bd_addr_t bdAddr;
      STREAM_TO_BDADDR(bdAddr.addr, data+pos);

      VERBOSE_PRINT("BD_ADDR(%d/%d) : %s    ", i, num, format2Hex((uint8_t*)&bdAddr.addr, BD_ADDR_LEN));

      int idx = findConnectedDevice(bdAddr);
      if(idx == -1){
        VERBOSE_PRINT("Page_Scan_Repetition_Mode = %02X    ", data[pos+6]);
        // data[pos+7] data[pos+8] // Reserved
        VERBOSE_PRINT("Class_of_Device = %02X %02X %02X    ", data[pos+9], data[pos+10], data[pos+11]);
        VERBOSE_PRINT("Clock_Offset = %02X %02X    ", data[pos+12], data[pos+13]);

        struct connected_device_t connected_device;
        connected_device.bdAddr = bdAddr;
        connected_device.psrm    = data[pos+6];
        connected_device.clkofs  = ((0x80 | data[pos+12]) << 8) | (data[pos+13]);

        idx = connected_device_add(connected_device);
        if(0<=idx){
            if(data[pos+9]==0x04 && data[pos+10]==0x25 && data[pos+11]==0x00){ // Filter for Wiimote [04 25 00] 
                uint16_t len = make_cmd_remote_name_request(tmpQueueData, connected_device.bdAddr, connected_device.psrm, connected_device.clkofs);
                sendHciPacket(tmpQueueData, len);
                //log_d("    connected_list_add n=%d", n);
                VERBOSE_PRINTLN("queued remote_name_request");
            }else{
                VERBOSE_PRINTLN("skiped to remote_name_request (!= Wiimote COD)");
            }
        }else{
            VERBOSE_PRINTLN("failed to connected_list_add");
        }
      }else{
        VERBOSE_PRINT(" (dup idx:%d)", idx);
      }
    }
}

static void handleRemoteNameRequestCompleteEvent(uint8_t len, uint8_t* data) {
    uint8_t status = data[0];
    VERBOSE_PRINT("remote_name_request_complete status=%02X", status);
    struct bd_addr_t bdAddr;
    STREAM_TO_BDADDR(bdAddr.addr, data+1);
    VERBOSE_PRINT("  BD_ADDR = %s", format2Hex((uint8_t*)&bdAddr.addr, BD_ADDR_LEN));

    char* name = (char*)(data+7);
    VERBOSE_PRINT("  REMOTE_NAME = %s", name);

    int idx = findConnectedDevice(bdAddr);
    if(0<=idx && strcmp("Nintendo RVL-CNT-01", name)==0){
        {
            uint16_t len = make_cmd_inquiry_cancel(tmpQueueData);
            sendHciPacket(tmpQueueData, len);
            VERBOSE_PRINTLN("queued inquiry_cancel");
        }

        struct connected_device_t connected_device = connectedDeviceList[idx];

        uint16_t pt = 0x0008;
        uint8_t ars = 0x00;
        uint16_t len = make_cmd_create_connection(tmpQueueData, connected_device.bdAddr, pt, connected_device.psrm, connected_device.clkofs, ars);
        sendHciPacket(tmpQueueData, len);
        VERBOSE_PRINTLN("queued create_connection");
    }
}

#define L2CAP_PAYLOAD_MAX_LEN (64)
uint8_t payload[L2CAP_PAYLOAD_MAX_LEN];

static void l2capConnect(uint16_t ch, uint16_t psm, uint16_t cid) {
    uint8_t  pbf = 0b10; // Packet Boundary Flag
    uint8_t  bf = 0b00; // Broadcast Flag
    uint16_t channelID           = 0x0001;

    // create command of 'Control frame'
    uint8_t  posi = 0;
    // Command Header
    payload[posi++] = 0x02;  // CODE:CONNECTION REQUEST
    payload[posi++] = 0x01;  // Identifier
    payload[posi++] = 0x04;  // Length:     0x0008
    payload[posi++] = 0x00;
    // PSM: HID_Control=0x0011, HID_Interrupt=0x0013
    payload[posi++] = (uint8_t)(psm & 0xFF);
    payload[posi++] = (uint8_t)(psm >> 8);
    // Source CID: 0x0040+
    payload[posi++] = (uint8_t)(cid & 0xFF);
    payload[posi++] = (uint8_t)(cid >> 8);
    uint16_t dataLen = posi;
    uint16_t len = make_acl_l2cap_packet(tmpQueueData, ch, pbf,  bf, channelID, payload, dataLen);
    sendHciPacket(tmpQueueData, len);
    VERBOSE_PRINTLN("queued acl_l2cap_single_packet(CONNECTION REQUEST)");
}

static void handleConnectionCompleteEvent(uint8_t len, uint8_t* data) {
    uint8_t status = data[0];
    VERBOSE_PRINT("connection_complete status=%02X", status);

    uint16_t ch = data[2] << 8 | data[1]; // Connection Handle
    struct bd_addr_t bdAddr;
    STREAM_TO_BDADDR(bdAddr.addr, data+3);
    uint8_t lt = data[9];  // Link Type
    uint8_t ee = data[10]; // Encryption Enabled

    VERBOSE_PRINT("  Connection_Handle  = 0x%04X", ch);
    VERBOSE_PRINT("  BD_ADDR            = %s", format2Hex((uint8_t*)&bdAddr.addr, BD_ADDR_LEN));
    VERBOSE_PRINT("  Link_Type          = %02X", lt);
    VERBOSE_PRINT("  Encryption_Enabled = %02X", ee);

    l2capConnect(ch, 0x0013, 0x0045);
}

static void handleDisconnectionCompleteEvent(uint8_t len, uint8_t* data) {
    uint8_t status = data[0];
    VERBOSE_PRINT("disconnection_complete status=%02X  ", status);

    uint16_t ch = data[2] << 8 | data[1]; //Connection Handle
    uint8_t reason = data[3];  // Reason

    VERBOSE_PRINT("Connection_Handle  = 0x%04X  ", ch);
    VERBOSE_PRINT("Reason             = %02X", reason);

    wiimoteConnected = false;
    resetDevice();
}

void handleHciEvent(uint8_t event_code, uint8_t len, uint8_t* data) {
    VERBOSE_PRINTLN("handleHciEvent");
    if(event_code != HCI_INQUIRY_RESULT_EVT){ // suppress HCI_INQUIRY_RESULT_EVT
      VERBOSE_PRINT("EVENT code=%02X len=%d data=%s", event_code, len, format2Hex(data, len));
    }

    switch(event_code){
      case HCI_INQUIRY_COMP_EVT:
        handleInquiryCompleteEvent(len, data);;
        break;
      case HCI_INQUIRY_RESULT_EVT:
        handleInquiryResultEvent(len, data);;
        break;
      case HCI_CONNECTION_COMP_EVT:
        handleConnectionCompleteEvent(len, data);;
        break;
      case HCI_DISCONNECTION_COMP_EVT:
        handleDisconnectionCompleteEvent(len, data);;
        break;
      case HCI_RMT_NAME_REQUEST_COMP_EVT:
        handleRemoteNameRequestCompleteEvent(len, data);;
        break;
      case HCI_COMMAND_COMPLETE_EVT:
        handleCommandCompleteEvent(len, data);;
        break;
      case HCI_COMMAND_STATUS_EVT:
        handleCommandStatusEvent(len, data);;
        break;
      default:
        // handleHciEvent no impl
        break;
    }

}


static void handleL2capConnectionResponse(uint16_t ch, uint8_t* data) {
  uint8_t identifier       =  data[1];
  // uint16_t len             = (data[3] << 8) | data[2];
  uint16_t dstCID = (data[5] << 8) | data[4];
  uint16_t srcCID      = (data[7] << 8) | data[6];
  uint16_t result          = (data[9] << 8) | data[8];
  uint16_t status          = (data[11] << 8) | data[10];

  // VERBOSE_PRINT("L2CAP CONNECTION RESPONSE");
  VERBOSE_PRINT("  identifier      = %02X", identifier);
  VERBOSE_PRINT("  dest cid = %04X", dstCID);
  VERBOSE_PRINT("  src cid      = %04X", srcCID);
  VERBOSE_PRINT("  result          = %04X", result);
  VERBOSE_PRINT("  status          = %04X", status);

  if(result == 0x0000){
      struct l2cap_connection_t connection;
      connection.ch = ch;
      connection.remoteCID = dstCID;
      int idx = l2capAddConnection(connection);
      if(idx == -1){
        VERBOSE_PRINTLN("l2cap connection failed");
        return;
      }
      uint8_t  pbf = 0b10; // Packet Boundary Flag
      uint8_t  bf = 0b00; // Broadcast Flag
      uint16_t channelID           = 0x0001;

      // create command of 'Control frame'
      uint8_t  posi = 0;
      // Command Header
      payload[posi++] = 0x04;  // CODE:CONFIGURATION REQUEST
      payload[posi++] = 0x02;  // Identifier
      payload[posi++] = 0x08;  // Length:     0x0008
      payload[posi++] = 0x00;
      // Destination CID
      payload[posi++] = (uint8_t)(dstCID & 0xFF);
      payload[posi++] = (uint8_t)(dstCID >> 8);
      // Flags
      payload[posi++] = 0x00;
      payload[posi++] = 0x00;
      // type=01 len=02 value=00 40
      payload[posi++] = 0x01;
      payload[posi++] = 0x02;
      payload[posi++] = 0x40;
      payload[posi++] = 0x00;

      uint16_t dataLen = posi;
      uint16_t len = make_acl_l2cap_packet(tmpQueueData, ch, pbf,  bf, channelID, payload, dataLen);
      sendHciPacket(tmpQueueData, len);
      VERBOSE_PRINTLN("queued acl_l2cap_single_packet(CONFIGURATION REQUEST)");
  }
}

static void handleL2capConfigurationResponse(uint16_t ch, uint8_t* data) {
  uint8_t identifier       =  data[1];
  uint16_t len             = (data[3] << 8) | data[2];
  uint16_t cid      = (data[5] << 8) | data[4];
  uint16_t flags           = (data[7] << 8) | data[6];
  uint16_t result          = (data[9] << 8) | data[8];
  // config = data[10..]

  // VERBOSE_PRINTLN("L2CAP CONFIGURATION RESPONSE  ");
  VERBOSE_PRINT("identifier      = %02X  ", identifier);
  VERBOSE_PRINT("len             = %04X  ", len);
  VERBOSE_PRINT("cid      = %04X  ", cid);
  VERBOSE_PRINT("flags           = %04X  ", flags);
  VERBOSE_PRINT("result          = %04X  ", result);
  VERBOSE_PRINT("config          = %s", format2Hex(data+10, len-6));
}

static void handleL2capConfigurationRequest(uint16_t ch, uint8_t* data) {
  uint8_t identifier       =  data[1];
  uint16_t len             = (data[3] << 8) | data[2];
  uint16_t dstCID = (data[5] << 8) | data[4];
  uint16_t flags           = (data[7] << 8) | data[6];

  // VERBOSE_PRINTLN("L2CAP CONFIGURATION REQUEST");
  VERBOSE_PRINT("identifier      = %02X", identifier);
  VERBOSE_PRINT("len             = %02X  ", len);
  VERBOSE_PRINT("dstCID = %04X  ", dstCID);
  VERBOSE_PRINT("flags           = %04X  ", flags);
  VERBOSE_PRINT("config          = %s", format2Hex(data+8, len-4));

  if(flags != 0x0000){
    VERBOSE_PRINTLN("flags!=0x0000");
    return;
  }
  if(len != 0x08){
    VERBOSE_PRINTLN("len!=0x08");
    return;
  }
  if((data[8] == 0x01) && (data[9] == 0x02)){ // MTU
    uint16_t mtu = (data[11] << 8) | data[10];
    VERBOSE_PRINT("  MTU=%d", mtu);

    int idx = l2capFindConnection(ch);
    struct l2cap_connection_t connection = l2capConnectionList[idx];

    uint8_t  pbf = 0b10; // Packet Boundary Flag
    uint8_t  bf = 0b00; // Broadcast Flag
    uint16_t channelID           = 0x0001;
    uint16_t cid           = connection.remoteCID;

    // create command of 'Control frame'
    uint8_t  posi = 0;
    // Command Header
    payload[posi++] = 0x05;        // CODE:CONFIGURATION RESPONSE
    payload[posi++] = identifier;  // Identifier
    payload[posi++] = 0x0A;        // Length:     0x000A
    payload[posi++] = 0x00;
    // Source CID
    payload[posi++] = (uint8_t)(cid & 0xFF);
    payload[posi++] = (uint8_t)(cid >> 8);
    // Flags
    payload[posi++] = 0x00;
    payload[posi++] = 0x00;
    // Res
    payload[posi++] = 0x00;
    payload[posi++] = 0x00;
    // type=01 len=02 value=xx xx
    payload[posi++] = 0x01;
    payload[posi++] = 0x02;
    payload[posi++] = (uint8_t)(mtu & 0xFF);
    payload[posi++] = (uint8_t)(mtu >> 8);

    uint16_t dataLen = posi;
    uint16_t len = make_acl_l2cap_packet(tmpQueueData, ch, pbf,  bf, channelID, payload, dataLen);
    sendHciPacket(tmpQueueData, len);
    VERBOSE_PRINTLN("queued acl_l2cap_single_packet(CONFIGURATION RESPONSE)");
  }
}

static void setPlayerLEDs(uint16_t ch, uint8_t leds) {
  int idx = l2capFindConnection(ch);
  struct l2cap_connection_t connection = l2capConnectionList[idx];

  uint8_t  pbf = 0b10; // Packet Boundary Flag
  uint8_t  bf = 0b00; // Broadcast Flag
  uint16_t channelID           = connection.remoteCID;

  // create information payload of 'Basic information frame'
  // wiimote report: (a2) 11 LL
  uint8_t  posi = 0;
  // Information Payload
  payload[posi++] = 0xA2;  // Output report
  payload[posi++] = 0x11;  // Function:Player LEDs
  payload[posi++] = (uint8_t)(leds << 4); // LL:controls the four LEDs
  uint16_t dataLen = posi;
  uint16_t len = make_acl_l2cap_packet(tmpQueueData, ch, pbf, bf, channelID, payload, dataLen);
  sendHciPacket(tmpQueueData, len);
  VERBOSE_PRINT("queued acl_l2cap_single_packet(Set LEDs)");
}

enum address_space_t {
  EEPROM_MEMORY,
  CONTROL_REGISTER
};

static uint8_t getAddrSpace(int as)
{
  switch(as){
    case EEPROM_MEMORY   : return 0x00;
    case CONTROL_REGISTER: return 0x04;
  }
  return 0xFF;
}

#define OFFSET_EEP_DATA (7)
#define SIZE_EEP_DATA (16)

static void writingEEPROM(uint16_t ch, int as, uint32_t offset, const uint8_t* eepData, uint8_t eepLen) {
  int idx = l2capFindConnection(ch);
  struct l2cap_connection_t connection = l2capConnectionList[idx];

  uint8_t  pbf = 0b10; // Packet Boundary Flag
  uint8_t  bf = 0b00; // Broadcast Flag
  uint16_t channelID           = connection.remoteCID;

  // create information payload of 'Basic information frame'
  // wiimote report: (a2) 16 MM FF FF FF SS DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD
  uint8_t  posi = 0;
  // Information Payload
  payload[posi++] = 0xA2;  // Output report
  payload[posi++] = 0x16;  // Function:Write Memory and Registers
  payload[posi++] = getAddrSpace((int)as);  // MM Address space: 0x00=EEPROM, 0x04=ControlRegister
  payload[posi++] = (uint8_t)((offset >> 16) & 0xFF); //FF
  payload[posi++] = (uint8_t)((offset >>  8) & 0xFF); //FF
  payload[posi++] = (uint8_t)((offset      ) & 0xFF); //FF
  payload[posi++] = eepLen;  // SS write eeprom size: 1..16
  memset(&payload[posi], 0, SIZE_EEP_DATA); // DD
  posi += SIZE_EEP_DATA;

  memcpy(payload+OFFSET_EEP_DATA, eepData, eepLen);

  uint16_t dataLen = posi;
  uint16_t len = make_acl_l2cap_packet(tmpQueueData, ch, pbf, bf, channelID, payload, dataLen);
  sendHciPacket(tmpQueueData, len);
  VERBOSE_PRINTLN("queued writingEEPROM");
}

static void readingEEPROM(uint16_t ch, int as, uint32_t offset, uint16_t size) {
  int idx = l2capFindConnection(ch);
  struct l2cap_connection_t connection = l2capConnectionList[idx];

  uint8_t  pbf = 0b10; // Packet Boundary Flag
  uint8_t  bf = 0b00; // Broadcast Flag
  uint16_t channelID           = connection.remoteCID;

  // create information payload of 'Basic information frame'
  // wiimote report: (a2) 17 MM FF FF FF SS SS
  uint8_t  posi = 0;
  // Information Payload
  payload[posi++] = 0xA2;  // Output report
  payload[posi++] = 0x17;  // Function:Read Memory and Registers
  payload[posi++] = getAddrSpace((int)as);  // MM Address space: 0x00=EEPROM, 0x04=ControlRegister
  payload[posi++] = (uint8_t)((offset >> 16) & 0xFF); // FF
  payload[posi++] = (uint8_t)((offset >>  8) & 0xFF); // FF
  payload[posi++] = (uint8_t)((offset      ) & 0xFF); // FF
  payload[posi++] = (uint8_t)((size >> 8   ) & 0xFF); // SS read eeprom size: 1..16
  payload[posi++] = (uint8_t)((size        ) & 0xFF); // SS

  uint16_t dataLen = posi;
  uint16_t len = make_acl_l2cap_packet(tmpQueueData, ch, pbf, bf, channelID, payload, dataLen);
  sendHciPacket(tmpQueueData, len);
  VERBOSE_PRINTLN("queued readingEEPROM");
}

static void setDataReportingMode(uint16_t ch, uint8_t mode, bool continuous) {
  int idx = l2capFindConnection(ch);
  struct l2cap_connection_t connection = l2capConnectionList[idx];

  uint8_t  pbf = 0b10; // Packet Boundary Flag
  uint8_t  bf = 0b00; // Broadcast Flag
  uint16_t channelID           = connection.remoteCID;
  uint8_t  contReportIsDesired = continuous ? 0x04 : 0x00; // 0x00, 0x04

  // create information payload of 'Basic information frame'
  // report: (a2) 12 TT MM
  uint8_t  posi = 0;
  // Information Payload
  payload[posi++] = 0xA2;  // Output report
  payload[posi++] = 0x12;  // Function:Data Reporting mode
  payload[posi++] = contReportIsDesired; // TT whether continuous reporting is desired
  payload[posi++] = mode; // MM

  uint16_t dataLen = posi;
  uint16_t len = make_acl_l2cap_packet(tmpQueueData, ch, pbf, bf, channelID, payload, dataLen);
  sendHciPacket(tmpQueueData, len);
  VERBOSE_PRINTLN("queued setDataReportingMode");
}

enum {
    REPORT_STATE_INIT = 0,
    REPORT_STATE_WAIT_ACK_OUT_REPORT,
    REPORT_STATE_WAIT_READ_COTRLLER_TYPE,
    REPORT_STATE_WAIT_READ_RESPONSE,
};

static void handleExtensionControllerReports(uint16_t ch, uint16_t channelID, uint8_t* data, uint16_t len) {
  static int controllerReportState = REPORT_STATE_INIT;

  switch(controllerReportState){
  case REPORT_STATE_INIT:
    VERBOSE_PRINT("REPORT_STATE_INIT\n");
    // data report(Status)
    // (a1) 20 BB BB LF 00 00 VV
    if(data[1] == 0x20){
      if(data[4] & 0x02){ // extension controller is connected
        writingEEPROM(ch, CONTROL_REGISTER, 0xA400F0, (const uint8_t[]){0x55}, 1);
        controllerReportState = REPORT_STATE_WAIT_ACK_OUT_REPORT;
      }else{ // extension controller is NOT connected
        setDataReportingMode(ch, 0x30, false); // 0x30: Core Buttons : 30 BB BB
        // [note] Core Buttons and Accelerometer: 31 BB BB AA AA AA
        // [note] Core Buttons and Accelerometer with 12 IR bytes: 33 BB BB AA AA AA II II II II II II II II II II II II 
      }
    }
    break;
  case REPORT_STATE_WAIT_ACK_OUT_REPORT:
    VERBOSE_PRINT("REPORT_STATE_WAIT_ACK_OUT_REPORT\n");
    // data report(Acknowledge output report, return function result)
    // (a1) 22 BB BB 16 00 : OK
    // (a1) 22 BB BB 16 04 : NG
    if((data[1] == 0x22) && (data[4] == 0x16)){
      if(data[5] == 0x00){
        writingEEPROM(ch, CONTROL_REGISTER, 0xA400FB, (const uint8_t[]){0x00}, 1);
        controllerReportState = REPORT_STATE_WAIT_READ_COTRLLER_TYPE;
      }else{
        controllerReportState = REPORT_STATE_INIT;
      }
    }
    break;
  case REPORT_STATE_WAIT_READ_COTRLLER_TYPE:
    VERBOSE_PRINT("REPORT_STATE_WAIT_READ_COTRLLER_TYPE\n");
    if((data[1] == 0x22) && (data[4] == 0x16)){
      if(data[5] == 0x00){
        readingEEPROM(ch, CONTROL_REGISTER, 0xA400FA, 6); // read controller type
        controllerReportState = REPORT_STATE_WAIT_READ_RESPONSE;
      }else{
        controllerReportState = REPORT_STATE_INIT;
      }
    }
    break;
  case REPORT_STATE_WAIT_READ_RESPONSE:
    VERBOSE_PRINT("REPORT_STATE_WAIT_READ_RESPONSE\n");
    // data report(Read response)
    // (a1) 21 BB BB SE FF FF DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD
    if(data[1] == 0x21){
      if(memcmp(data+5, (const uint8_t[]){0x00, 0xFA}, 2) == 0){
        if(memcmp(data+7, (const uint8_t[]){0x00, 0x00, 0xA4, 0x20, 0x00, 0x00}, 6) == 0){ // Nunchuck
          setDataReportingMode(ch, 0x32, false); // 0x32: Core Buttons with 8 Extension bytes : 32 BB BB EE EE EE EE EE EE EE EE
        }
        controllerReportState = REPORT_STATE_INIT;
      }
    }
    break;
  }
}


/**
 * Received Data
 */
struct recv_data_rb {
  uint8_t wp;
  uint8_t rp;
  uint8_t cnt;
};
recv_data_rb receivedDataRb;
#define RECIEVED_DATA_MAX_NUM     (5)
TinyWiimoteData receivedData[RECIEVED_DATA_MAX_NUM];

void putWiimoteReceivedData(uint8_t number, uint8_t* data, uint8_t len) {
  if(receivedDataRb.cnt < RECIEVED_DATA_MAX_NUM) {
    TinyWiimoteData *target = &(receivedData[receivedDataRb.wp]);
    memcpy(target->data, data, len);
    target->number = number;
    target->len = len;
    receivedDataRb.wp = (receivedDataRb.wp + 1) % RECIEVED_DATA_MAX_NUM;
    receivedDataRb.cnt++;
  }
  VERBOSE_PRINTLN("");
}

static void handleReport(uint8_t* data, uint16_t len) {
  VERBOSE_PRINT("REPORT len=%d data=%s", len, format2Hex(data, len));
  uint8_t idx = 0; //only supports one wiimote
  putWiimoteReceivedData(idx, data, len);
}

static void handleL2capData(uint16_t ch, uint16_t channelID, uint8_t* data, uint16_t len) {
  VERBOSE_PRINTLN("handleL2capData");
  VERBOSE_PRINT("data[0]=%02X\n", data[0]);

  switch(data[0]) {
    case L2CAP_CONNECT_RES:
      VERBOSE_PRINT("L2CAP CONNECTION RESPONSE");
      handleL2capConnectionResponse(ch, data);
      break;
    case L2CAP_CONFIG_REQ:
      VERBOSE_PRINTLN("L2CAP CONFIGURATION REQUEST");
      handleL2capConfigurationRequest(ch, data);
      break;
    case L2CAP_CONFIG_RES:
      VERBOSE_PRINTLN("L2CAP CONFIGURATION RESPONSE");
      handleL2capConfigurationResponse(ch, data);
      break;
    case BTCODE_HID:
      if(!wiimoteConnected){
        setPlayerLEDs(ch, 0b0001);
        wiimoteConnected = true;
      }
      handleExtensionControllerReports(ch, channelID, data, len);
      handleReport(data, len);
      break;
    default:
      // handleL2capData no impl
      VERBOSE_PRINT("  L2CAP len=%d data=%s\n", len, format2Hex(data, len));
      break;
  }

}

void handleAclData(uint8_t* data, size_t len) {
    VERBOSE_PRINT("handleAclData\n");
    if(!wiimoteConnected){
      VERBOSE_PRINT("len=%d data=%s\n", len, format2Hex(data, len));
    }

    uint16_t ch    = ((data[1] & 0x0F) << 8) | data[0]; // Connection Handle
    uint8_t  pbf =  (data[1] & 0x30) >> 4; // Packet Boundary Flag
    uint8_t  bf =  (data[1] & 0xC0) >> 6; // Broadcast Flag
    uint16_t aclLen              =  (data[3] << 8) | data[2];
    if(pbf != 0b10){
      VERBOSE_PRINT("packet boundary flag = %d, aclLen = %d", pbf, aclLen);
      return;
    }
    if(bf != 0b00){
      VERBOSE_PRINT("bf = %d", bf);
      return;
    }
    uint16_t l2capLen            =  (data[5] << 8) | data[4];
    uint16_t channelID           =  (data[7] << 8) | data[6];

    handleL2capData(ch, channelID, data + 8, l2capLen);
}

void handleHciData(uint8_t* data, size_t len) {
    switch(data[0]){
    case H4_TYPE_EVENT:
      handleHciEvent(data[1], data[2], data+3);
      break;
    case H4_TYPE_ACL:
      handleAclData(data+1, len-1);
      break;
    default:
      VERBOSE_PRINT("UNKNOWN EVENT");
      VERBOSE_PRINT("len=%d data=%s", len, format2Hex(data, len));
    }
}

bool TinyWiimoteDeviceIsInited(void) {
  return deviceInited;
}

int TinyWiimoteAvailable() {
  return receivedDataRb.cnt;
}

TinyWiimoteData TinyWiimoteRead() {
  TinyWiimoteData target;
  target.number = 0;
  target.len = 0;
  if(receivedDataRb.cnt > 0) {
    target = receivedData[receivedDataRb.rp];
    receivedDataRb.rp = (receivedDataRb.rp + 1) % RECIEVED_DATA_MAX_NUM;
    receivedDataRb.cnt--;
  }
  return target;
}

void TinyWiimoteInit(TwHciInterface hciInterface) {
    receivedDataRb.cnt = 0;
    receivedDataRb.wp = 0;
    receivedDataRb.rp = 0;
    _hciInterface = hciInterface;
}
