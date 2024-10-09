/**************************************************
*
Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V1.0

Filename :

Project  : Anx_Liberty

Created  : 02 Apr. 2017

Devices  : Anx_Liberty

Toolchain: Android

Description:

Revision History:

***************************************************/

#ifndef __LIBERTY_REG_H__
#define __LIBERTY_REG_H__

#define TCPC_INTERFACE				0 /*0x58*/
#define RX_P0				1 /*0x7e*/
#define RX_P1				2 /*0x7a*/
#define RX_P2				3 /*0x84*/

#define _BIT0	0x01
#define _BIT1	0x02
#define _BIT2	0x04
#define _BIT3	0x08
#define _BIT4	0x10
#define _BIT5	0x20
#define _BIT6	0x40
#define _BIT7	0x80
/***************************************************************/
/*Register definition of device address 0x58*/

#define VENDOR_ID_L 0x00
#define VENDOR_ID_H 0x01

#define PRODUCT_ID_L 0x02
#define PRODUCT_ID_H 0x03
#define ALERT_MASK_0                            0x12
#define VBUS_VOLTAGE_ALARM_HI _BIT7
#define ALERT_MASK_1                            0x13
#define VD_DEF_INT _BIT7
#define TCPC_ROLE_CONTROL                            0x1A
#define DRP_CONTROL _BIT6
#define RP_VALUE    (_BIT5|_BIT4)
#define CC2_CONTROL (_BIT3|_BIT2)
#define CC1_CONTROL (_BIT1|_BIT0)
#define DRP_EN _BIT6 /* DRP_CONTROL*/


#define TCPC_COMMAND  0x23

#define ANALOG_CTRL_0  0xA0
#define DFP_OR_UFP _BIT6

#define VD_ALERT_MASK  0xC7
#define VD_ALERT  0xC8
#define INTR_OCP_ALARM _BIT1 

#define INTR_ALERT_0  0xCB
#define INTR_RECEIVED_MSG _BIT7
#define INTR_SOFTWARE_INT _BIT6

#define INTR_ALERT_1  0xCC
#define INTR_INTP_POW_ON _BIT7 /* digital powerup indicator*/
#define INTR_INTP_POW_OFF _BIT6


#define TCPC_CTRL_2   0xCD
#define SOFT_INTP_1 _BIT1

#define  TCPC_CONTROL       0x19

#define  POWER_CONTROL       0x1C

#define TX_OBJ1_BYTE_0  0x54

#define VBUS_VOLTAGE_0 0x70
#define VBUS_VOLTAGE_BIT7_0 0xFF

#define VBUS_VOLTAGE_1 0x71
#define VBUS_VOLTAGE_BIT9_8 (_BIT0 | _BIT1)

#define  PD_1US_PERIOD       0x80
#define  PD_TX_BIT_PERIOD       0x86

#define ANALOG_CTRL_1                           0xA1
#define R_TOGGLE_ENABLE _BIT7
#define R_LATCH_TOGGLE_ENABLE _BIT6
#define TOGGLE_CTRL_MODE _BIT5
#define CC2_VRD_USB _BIT2
#define CC2_VRD_1P5 _BIT1
#define CC2_VRD_3P0 _BIT0


#define  ANALOG_CTRL_9      0xA9

#define ANALOG_CTRL_10                          0xAA
#define FRSWAP_CTRL                             0xAB
#define FR_SWAP _BIT7
#define FR_SWAP_EN _BIT6
#define R_FRSWAP_CONTROL_SELECT _BIT3
#define R_SIGNAL_FRSWAP _BIT2
#define TRANSMIT_FRSWAP_SIGNAL _BIT1
#define FRSWAP_DETECT_ENABLE _BIT0

#define  ANALOG_CTRL_11 0xD8
#define RING_OSC_CTRL 0xD9
#define VBUS_OCP_0 0xE6
#define VBUS_OCP_1 0xE7
#define VBUS_OCP_BIT9_8 (_BIT0 | _BIT1)

#define VBUS_VOLTAGE_ALARM_HI_CFG_0             0x76
#define VBUS_VOLTAGE_ALARM_HI_CFG_1             0x77
#define VBUS_VOLTAGE_ALARM_LO_CFG_0             0x78
#define VBUS_VOLTAGE_ALARM_LO_CFG_1             0x79
#define VBUS_OCP_HI_THRESHOLD_0                 0xDD
#define VBUS_OCP_HI_THRESHOLD_1                 0xDE

#define PD_PORT_RANDOM_NUM		0xEF

#define PD_MSG_CTRL0 0xF0
#define UNSTRUC_VDM_ENABLE  _BIT0
#define PD_TASK_RUN_ENABLE  _BIT1
#define VSAFE0V_VOLTAGE0 _BIT2
#define VSAFE0V_VOLTAGE1 _BIT3
#define LEXINGTON_DRIVER_ENABLE _BIT4
#define DEBUG_ACCESSORY_ENABLE _BIT5 // LBT-475
//#define PREFER_FUNC_DISABLE _BIT6 // LBT-452

#define PD_FUNC_CTRL0		0xF1 // 
#define FAST_ROLE_SWAP_CTL_MODE  _BIT0  // 0: OCM internal detect, 1: external detect  LBT-293 
#define FAST_ROLE_SWAP_TRIGGER_EN  _BIT1
#define PPS_FUNC_EN _BIT2   //LBT-428 1: select APDO if Source_Cap have this PDO

#define PD_FUNC_CTRL1 0xF4 // 
#define PREFER_FUNC_DISABLE      _BIT0
#define DFP_DR_SWAP_TRIGGER _BIT1   //trigger DR_swap on DFP  LBT-549
#define UFP_DR_SWAP_TRIGGER _BIT2   //trigger DR_swap on UFP


#define CUSTOM_ZONE_VERSION_REG 0xFF

/***************************************************************/
/*Register definition of device address 0x7a*/
#define TX_DATA_BYTE_30  0x00

/***************************************************************/
/*Register definition of device address 0x7e*/
#define R_RAM_LEN_H  0x03
#define FLASH_ADDR_EXTEND  _BIT7

#define R_RAM_CTRL 0x05
#define FLASH_DONE  _BIT7
#define BOOT_LOAD_DONE _BIT6
#define LOAD_CRC_OK _BIT5 /* CRC_OK*/
#define LOAD_DONE _BIT4

#define R_FLASH_ADDR_H 0x0c
#define R_FLASH_ADDR_L 0x0d

#define FLASH_WRITE_DATA_0 0xe
#define FLASH_READ_DATA_0 0x3c

#define R_FLASH_LEN_H 0x2e
#define R_FLASH_LEN_L 0x2f

#define  R_FLASH_RW_CTRL  0x30
#define GENERAL_INSTRUCTION_EN  _BIT6
#define FLASH_ERASE_EN _BIT5
#define WRITE_STATUS_EN _BIT2
#define FLASH_READ  _BIT1
#define FLASH_WRITE _BIT0

#define R_FLASH_STATUS_0 0x31

#define  FLASH_INSTRUCTION_TYPE  0x33
#define FLASH_ERASE_TYPE 0x34

#define R_FLASH_STATUS_REGISTER_READ_0	0x35
#define WIP	_BIT0

#define R_I2C_0	0x5C
#define read_Status_en	_BIT7


#define  OCM_CTRL_0  0x6e
#define OCM_RESET  _BIT6

#define ADDR_GPIO_CTRL_0  0x88
#define SPI_WP _BIT7
#define SPI_CLK_ENABLE  _BIT6

#define ADDR_PDFU_LOAD_0 0x98

/*
* For SKIP highest voltage
* Maximum Voltage for Request Data Object
* 100mv units
*/
#define MAX_VOLTAGE 0xAC /* 0x7E:0xAC*/
/*
* For selection PDO
* Maximum Power for Request Data Object
* 500mW units
*/
#define MAX_POWER 0xAD /* 0x7E:0xAD*/
/*
* For mismatch
* Minimum Power for Request Data Object
* 500mW units
*/
#define MIN_POWER 0xAE /* 0x7E:0xAE*/
/*Show Maximum voltage of RDO*/
#define RDO_MAX_VOLTAGE 0xAF /* 0x7E:0xAF*/
/*Show Maximum Powe of RDO*/
#define RDO_MAX_POWER 0xB0 /* 0x7E:0xB0*/
/*Show Maximum current of RDO*/
#define RDO_MAX_CURRENT 0xB1 /* 0x7E:0xB1*/

#define FIRMWARE_CTRL 0xB2 /* 0x7E:0xB2*/
#define disable_usb30 _BIT0
#define auto_pd_en _BIT1
#define trysrc_en _BIT2
#define trysnk_en _BIT3
#define support_goto_min_power _BIT4
#define snk_remove_refer_cc _BIT5
#define rev _BIT6
#define high_voltage_for_same_power _BIT7

#define FW_STATE_MACHINE 0xB3 /* 0x7E:0xB3*/

#define OCM_VERSION_REG 0xB4

#define INT_MASK 0xB6 /* 0x7E:0xB6*/
/*same with 0x28 interrupt mask*/
#define CHANGE_INT 0xB7 /* 0x7E:0xB7*/
#define OCM_BOOT_UP _BIT0
#define OC_OV_EVENT _BIT1     // OC/OV protection  LBT-399
#define VCONN_CHANGE _BIT2
#define VBUS_CHANGE _BIT3
#define CC_STATUS_CHANGE _BIT4
#define DATA_ROLE_CHANGE _BIT5
#define PR_CONSUMER_GOT_POWER _BIT6
#define DP_HPD_CHANGE _BIT7

#define SYSTEM_STSTUS 0xB8 /* 0x7E:0xB8*/
/*0: no OC/OC ; 1: OC/OV event*/
#define OC_OV_STATUS _BIT0
/*0: vbus sink off; 1: vbus sink on*/
#define VBUS_SINK _BIT1
/*0: VCONN off; 1: VCONN on*/
#define VCONN_STATUS _BIT2
/*0: vbus off; 1: vbus on*/
#define VBUS_STATUS _BIT3
/*0: no ; 1: self charging*/
#define SELF_CHARGING _BIT4
/*1: host; 0:device*/
#define S_DATA_ROLE _BIT5
/*0: Chunking; 1: Unchunked*/
#define SUPPORT_UNCHUNKING _BIT6
/*0: HPD low; 1: HPD high*/
#define HPD_STATUS _BIT7

#define NEW_CC_STATUS 0xB9 /* 0x7E:0xB9*/

/* PD Revision configure*/
/* 0: default, 1:PD_REV20, 2:PD_REV30*/
#define PD_REV_CTRL 0xBA /* 0x7E:0xBA*/
#define PD_REV_INIT (_BIT1|_BIT0)
#define PD_REV_USED (_BIT5|_BIT4)

#define PD_EXT_MSG_CTRL 0xBB /* 0x7E:0xBB*/
#define SRC_CAP_EXT_REPLY 0
#define MANUFACTURER_INFO_REPLY 1
#define BATTERY_STS_REPLY 2
#define BATTERY_CAP_REPLY 3
#define ALERT_REPLY	4
#define STATUS_REPLY	5
#define PPS_STATUS_REPLY	6
#define FIXED_INFO_REPLY 	7   //country code, country info  

#endif  /* __LIBERTY_REG_H__ */

