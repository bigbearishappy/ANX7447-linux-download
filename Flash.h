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
#ifndef FLASH_H
#define FLASH_H

//#define VERIFY_IN_FLASHING

//#define VERIFY_AFTER_FLASHING

#define NO_BLOCKREAD

#define VERIFY_BY_HW

//#define LOG_RESULT_FILE

enum tagCommand {
	WriteEnable     = 0x06, // WREN
	DeepPowerDown   = 0xB9, // DP
	Nop             = 0x00, //
	// Erase
	PageErase       = 0x81, // Page Erase
	SectorErase     = 0x20, // Sector Erase (4K bytes)
	BlockErase32K   = 0x52, // Block Erase (32K bytes)
	BlockErase64K   = 0xD8, // Block Erase (64K bytes)
	ChipErase       = 0x60, // Chip Erase

};

#define INDEX_NONE 0

enum HEXINDEX {
	INDEX_OCM     = _BIT0,
	INDEX_CUSTOM   = _BIT1,
	INDEX_ERROR      = _BIT2,

};

#define AC_CHIP_CRC_POS 0xDFF0
#define AA_CHIP_CRC_POS 0xFBF0
#define HEX_LINE_SIZE 44
extern unsigned char OCM_FW_HEX[][HEX_LINE_SIZE];

extern unsigned char OCM_FW_BIN[64*1024];
extern unsigned int bin_size;



extern unsigned char CUSTOM_ZONE_HEX[][HEX_LINE_SIZE];
extern unsigned char CUST_ZONE_BIN[1*1024];
extern unsigned int cust_size;

extern unsigned char CHIP_VER[MAX_USBC_PORT_NUM];



void prog_hex(const char * pstr, char port, char section);

void flash_read(unsigned char port, unsigned char section, unsigned long len);
void flash_read_start(unsigned char port, unsigned char section, unsigned long addr, unsigned long len);

void flash_erase_all(unsigned char port);
void flash_operation_initial(unsigned char port);
void flash_page_erase(unsigned char port, unsigned char section,  unsigned int address);
void Hex_Verify(const char * pstr, char port, char section);

void TRACE_ARRAY(unsigned char array[], unsigned char len);
char GetLineData(const unsigned char *pLine, unsigned char *pByteCount,
                 unsigned int *pAddress, unsigned char *pRecordType,
                 unsigned char *pData);
u8 burnhex(u8 port_id, int file_index);
void burnhexauto(u8 port_id);
void verifyhex(u8 port_id, u8 section_index);
void GetFwVersion(u8 port_id,  u8 *VerBuf );
u8 burnbin(u8 port_id, int file_index);
u8 MatchHex(u8 port_id);
unsigned char readbinver(unsigned char selectbin);
unsigned char readhexver(unsigned char selecthex);


void power_on(u8 port);
void power_standby(unsigned char port_id);


#endif  // end of HEXFILE_H definition

