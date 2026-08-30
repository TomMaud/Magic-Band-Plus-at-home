#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

// Core C Standard Library Features
#define HAVE_MALLOC

// Prevent macro redefinition warnings from Pico SDK build flags
#ifndef ENABLE_BLE
#define ENABLE_BLE
#endif

#ifndef ENABLE_LE_PERIPHERAL
#define ENABLE_LE_PERIPHERAL
#endif

#ifndef ENABLE_LE_ADVERTISING
#define ENABLE_LE_ADVERTISING
#endif

// CYW43 Bus & Buffer Alignment Settings
#define HCI_OUTGOING_PRE_BUFFER_SIZE 4
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4

// NVM TLV Key-Value Storage Settings
#define NVM_NUM_DEVICE_DB_ENTRIES 16
#define NVM_NUM_LINK_KEYS 16

// Debugging & Hexdump Logging
#define ENABLE_PRINTF_HEXDUMP
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR
#define HAVE_HAL_AUDIO_SINK_DUMMY

// Memory Pool Allocations
#define MAX_NR_HCI_CONNECTIONS 1
#define MAX_NR_L2CAP_SERVICES  1
#define MAX_NR_L2CAP_CHANNELS  1
#define MAX_NR_GATT_CLIENTS    0

// HCI Controller Settings
#define HCI_ACL_PAYLOAD_SIZE 255
#define MAX_NR_WHITELIST_ENTRIES 1
#define MAX_NR_SM_LOOKUP_ENTRIES 3

#endif // BTSTACK_CONFIG_H