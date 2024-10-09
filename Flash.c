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
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>

#include <Library/PrintLib.h>

#include  <stdio.h>
#include 	<stdlib.h>
#include 	<string.h>

#include "AnxUpdate.h"
#include "liberty_reg.h"
#include "flash.h"

#define snprintf AsciiSPrint
#define msleep(a) MicroSecondDelay(a*1000)

#define HEX_LINE_COUNT 4*1024
#define  MAX_BYTE_COUNT_PER_RECORD_FLASH    16

/* record types; only I8HEX files are supported,*/
/*so only record types 00 and 01 are used */
#define  HEX_RECORD_TYPE_DATA   0
#define  HEX_RECORD_TYPE_EOF    1



unsigned char OCM_FW_HEX[HEX_LINE_COUNT][HEX_LINE_SIZE] = {
#include "Liberty_ocm.hex"
};
unsigned char CUSTOM_ZONE_HEX[HEX_LINE_COUNT][HEX_LINE_SIZE] = {
#include "custom_zone.hex"
};

unsigned char OCM_FW_BIN[64*1024];
unsigned char CUST_ZONE_BIN[1*1024];
unsigned int bin_size = 0;
unsigned int cust_size = 0;

static unsigned short int  BurnHexIndex;
static unsigned char *g_CmdLineBuf;
static unsigned char current_section;   //0: section A, 1: section B

unsigned char CHIP_VER[MAX_USBC_PORT_NUM];

unsigned char hex_ver[2];
unsigned char ocm_ver[2];

#define FW_LEN          0xFC00 // 
#define FW_A_START_ADDR 0x0000
#define FW_B_START_ADDR 0xFC00
#define PDFU_POINTER_ADDR   0x1F800
#define FLASH_END   0x1FFFF

unsigned char g_bBurnHex;
unsigned char ping = 0;
unsigned char message_flag = 0;
//int Reg_addr;

void power_on(u8 port)
{
	/* power on chip */
	unsigned char c1;

	c1 = ReadReg(port, TCPC_INTERFACE, TCPC_COMMAND);
	TRACE1("Checking ANX Chip[%d]...\n", port);
	
	msleep(100);

}

void power_standby(unsigned char port_id)
{
  unsigned char chip_version;
	//ocm reset
	 lbt_wrbyte_or(port_id, RX_P0, OCM_CTRL_0, OCM_RESET);

	chip_version = lbt_rdbyte(port_id, RX_P0, 0x95);
	lbt_wrbyte(port_id, TCPC_INTERFACE, ANALOG_CTRL_10, 0x80);
       lbt_wrbyte(port_id, TCPC_INTERFACE, TCPC_ROLE_CONTROL, 0x4A); //CC with  RD

	if (chip_version >= 0xAC && chip_version != 0xFF) {		
		lbt_wrbyte(port_id, TCPC_INTERFACE, TCPC_COMMAND , 0x99);  //DRP en
	}	else {
		lbt_wrbyte_or(port_id, TCPC_INTERFACE, ANALOG_CTRL_1, TOGGLE_CTRL_MODE | R_TOGGLE_ENABLE); 
		lbt_wrbyte_or(port_id, TCPC_INTERFACE, ANALOG_CTRL_1, R_LATCH_TOGGLE_ENABLE); 
	}

	//TRACE1("chip_version 0x%X, power down!\n", (int)chip_version);
	CHIP_VER[port_id] = chip_version;

	WriteReg(port_id, TCPC_INTERFACE, TCPC_COMMAND , 0xFF);
}




unsigned char CheckWIP(unsigned char port)
{
	lbt_wrbyte_or(port, RX_P0, R_I2C_0 , read_Status_en);
	// wait for Read Status Register (RDSR) Sequence done
	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));
	return (lbt_rdbyte(port, RX_P0, R_FLASH_STATUS_REGISTER_READ_0) & WIP);
}

void flash_write_enable(unsigned char port)
{

	lbt_wrbyte(port, RX_P0, FLASH_INSTRUCTION_TYPE , WriteEnable);
	lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , GENERAL_INSTRUCTION_EN);
	// wait for Write Enable (WREN) Sequence done
	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));
}

void flash_operation_initial(unsigned char port)
{
	//ocm reset
	lbt_wrbyte_or(port, RX_P0, OCM_CTRL_0, OCM_RESET);

	// disable Hardware Protected Mode (HPM)
	lbt_wrbyte_or(port, RX_P0, ADDR_GPIO_CTRL_0 , SPI_WP);

	flash_write_enable(port);

	// disable protect
	lbt_wrbyte_and(port, RX_P0, R_FLASH_STATUS_0 , 0x43);
	lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , WRITE_STATUS_EN);
	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));
}

void flash_exit_sleep(unsigned char port)
{
	// LBT-553, patch for external MCU exit Deep Power-down mode issue by read flash
	lbt_wrbyte(port, RX_P0, R_FLASH_LEN_H, 0x0);
	lbt_wrbyte(port, RX_P0, R_FLASH_LEN_L, 0x1f);
	lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , FLASH_READ);
	msleep(4);
	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE)); 
	msleep(5);
}

void flash_update_abort(unsigned char port)
{
	flash_write_enable(port);
	// protect all blocks
	lbt_wrbyte_or(port, RX_P0, R_FLASH_STATUS_0 , 0xbc);
	lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , WRITE_STATUS_EN);

	// wait for Write Status Register (WRSR) Sequence done
	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));

	// enable Hardware Protected Mode (HPM)
	lbt_wrbyte_and(port, RX_P0, ADDR_GPIO_CTRL_0 , ~SPI_WP);
	// enable Deep Power-down mode
	lbt_wrbyte(port, RX_P0, FLASH_INSTRUCTION_TYPE , DeepPowerDown);
	lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , GENERAL_INSTRUCTION_EN);
	// wait for Deep Power-down (DP) Sequence done
	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));
}
void flash_config_extendBit(unsigned char port, unsigned char section, int Reg_addr)
{
	if(g_bBurnHex == INDEX_CUSTOM)
	{/*customer define zone*/

	      /*section B*/
		lbt_wrbyte_or(port, RX_P0, R_RAM_LEN_H , FLASH_ADDR_EXTEND);
		lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_H,  (unsigned char)((Reg_addr ) >> 8));
		lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_L,  (unsigned char)((Reg_addr ) & 0xff));
	} else if (section) {
		//section B
		if(Reg_addr < 0x400)
			lbt_wrbyte_and(port, RX_P0, R_RAM_LEN_H , ~FLASH_ADDR_EXTEND);
		else
			lbt_wrbyte_or(port, RX_P0, R_RAM_LEN_H , FLASH_ADDR_EXTEND);

		lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_H, (unsigned char)((Reg_addr + FW_B_START_ADDR) >> 8));
		lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_L, (unsigned char)((Reg_addr + FW_B_START_ADDR) & 0xff));
	} else {
		//section A
		lbt_wrbyte_and(port, RX_P0, R_RAM_LEN_H , ~FLASH_ADDR_EXTEND);
		lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_H,  (unsigned char)(Reg_addr >> 8));
		lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_L,  (unsigned char)(Reg_addr & 0xff));
	}
}

void liberty_block_write(unsigned char port, unsigned char section, int Reg_addr)
{

	flash_write_enable(port);
	//Address
	flash_config_extendBit(port, section, Reg_addr);
	//Length
	lbt_wrbyte(port, RX_P0, R_FLASH_LEN_H, 0x0);
	lbt_wrbyte(port, RX_P0, R_FLASH_LEN_L, 0x1f);
	//write
	lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , FLASH_WRITE);
	// wait flash write  Sequence done
	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));
	while((CheckWIP(port)));
}
void liberty_block_read(unsigned char port, unsigned char section, int Reg_addr, unsigned char * buff)
{
	unsigned char ReadDataBuf[32];
	unsigned int Address;


	flash_operation_initial(port);

	if(section) {
		//section B
		if(Reg_addr < 0x400)
			lbt_wrbyte_and(port, RX_P0, R_RAM_LEN_H , ~FLASH_ADDR_EXTEND);
		else
			lbt_wrbyte_or(port, RX_P0, R_RAM_LEN_H , FLASH_ADDR_EXTEND);

		Address = (FW_B_START_ADDR + Reg_addr) & 0xFFFF;
	} else {
		//section A
		lbt_wrbyte_and(port, RX_P0, R_RAM_LEN_H , ~FLASH_ADDR_EXTEND);
		Address = Reg_addr; //address;
	}

	{
		lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_H, (unsigned char)(Address >> 8));
		lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_L, (unsigned char)(Address  & 0xff));
		//Length
		lbt_wrbyte(port, RX_P0, R_FLASH_LEN_H, 0x0);
		lbt_wrbyte(port, RX_P0, R_FLASH_LEN_L, 0x1f);
		//read
		lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , FLASH_READ);
		msleep(4);
		// wait flash read  Sequence done
		while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));
		msleep(5);
//		lbt_rdbytes(port, RX_P0, FLASH_READ_DATA_0, 32, (unsigned char *)ReadDataBuf);
	}

	//Address
	lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_H, (unsigned char)(Address >> 8));
	lbt_wrbyte(port, RX_P0, R_FLASH_ADDR_L, (unsigned char)(Address  & 0xff));
	//Length
	lbt_wrbyte(port, RX_P0, R_FLASH_LEN_H, 0x0);
	lbt_wrbyte(port, RX_P0, R_FLASH_LEN_L, 0x1f);
	//read
	lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , FLASH_READ);
	msleep(4);
	// wait flash read  Sequence done
	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));
	msleep(5);
	lbt_rdbytes(port, RX_P0, FLASH_READ_DATA_0, 32, (unsigned char *)ReadDataBuf);

	memcpy(buff, ReadDataBuf, 32);
	msleep(3);
}


unsigned char update_result;
void Hex_Verify(const char * pstr, char port, char section)
{
	unsigned char WriteDataBuf[32];
	static unsigned char ReadDataBuf[32];
	static int stored_addr=0xffff;
	static char stored_section=0xff;
	unsigned char ByteCount;
	int Address;
	unsigned char RecordType;
	unsigned char i = 0;

	unsigned char  LineBuf[16];

	if(GetLineData(pstr, &ByteCount, &Address, &RecordType, WriteDataBuf) == 0) {
		if(RecordType == 0) {
			for(i = 0; i < 16; i++) {
				LineBuf[i] = WriteDataBuf[i];
			}

			if ((stored_addr == Address )&&(stored_section==section)){
				memcpy(&ReadDataBuf[0],&ReadDataBuf[16],16);
//				TRACE1("CPY ADDR:%X\n",stored_addr);
			}else{

			flash_config_extendBit(port, section, Address);

			//Length
			lbt_wrbyte(port, RX_P0, R_FLASH_LEN_H, 0x0);
			lbt_wrbyte(port, RX_P0, R_FLASH_LEN_L, 0x1f);
			//read
			lbt_wrbyte(port, RX_P0, R_FLASH_RW_CTRL , 0x00);
			lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , FLASH_READ);

			while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));

			lbt_rdbytes(port, RX_P0, FLASH_READ_DATA_0, 32, (unsigned char *)ReadDataBuf);

			stored_addr = Address + 16;
			stored_section = section;

			}
			if(ByteCount == 0x10) {
				if(memcmp((unsigned char *)LineBuf, (unsigned char *)ReadDataBuf, 16)) {
					update_result = 1;
					if (g_bBurnHex == INDEX_OCM)
						TRACE("Main OCM ");
					else
						TRACE("Custom Zone ");
					TRACE("Hex check fail !\n");
					TRACE_ARRAY((unsigned char *)LineBuf, 16);
					TRACE("Flash Data:\n");
					TRACE_ARRAY((unsigned char *)ReadDataBuf, 16);
					g_bBurnHex = 0;
				}
			}

		} else if(RecordType == 1) {
		/*	if(update_result)
				TRACE("Hex Verify ... failed!\n");
			else
				TRACE("Hex Verify ... pass!\n");*/

			g_bBurnHex = 0;

		}
	}
}

unsigned char update_verify_flag = 1;
void prog_hex(const char * pstr, char port, char section)
{
	static int Reg_addr;
	static unsigned char DataBuf_bak[32];
	unsigned char WriteDataBuf[32];
	unsigned char ByteCount;
	int Address;
	unsigned char RecordType;
#ifdef VERIFY_IN_FLASHING
	unsigned char ReadDataBuf[32];
#endif

	if(GetLineData(pstr, &ByteCount, &Address, &RecordType, WriteDataBuf) == 0) {
		if(RecordType == 0) {
			// data record
			if((Address & 0x0F) == 0) {// address alignment
				// key or fw
				if(ByteCount == 16) {
					memcpy((unsigned char*)DataBuf_bak + ping * 16, (unsigned char* )WriteDataBuf, 16);
					if (ping==1) lbt_wrbytes(port, RX_P0, (FLASH_WRITE_DATA_0), 32, (unsigned char* )DataBuf_bak);

					if(ping == 0) {
						ping = 1;
						Reg_addr = Address;
						if((Address == AA_CHIP_CRC_POS)||(Address == AC_CHIP_CRC_POS)) {
							Reg_addr -= 0x10;
							ping = 0;

							memset(DataBuf_bak, 0xff, 16);
							memcpy((unsigned char*)DataBuf_bak + 16, (unsigned char* )WriteDataBuf, 16);
							lbt_wrbytes(port, RX_P0, (FLASH_WRITE_DATA_0 ), 32, (unsigned char* )DataBuf_bak);

						}
					} else {
						ping = 0;
						if((Address == AA_CHIP_CRC_POS)||(Address == AC_CHIP_CRC_POS)) {
							//Address = Reg_addr ;
							memset((unsigned char*)DataBuf_bak + 16, 0xff, 16);

							lbt_wrbytes(port, RX_P0, (FLASH_WRITE_DATA_0 ), 32, (unsigned char*)DataBuf_bak );
							liberty_block_write(port, section, Reg_addr);

							if (Address == AA_CHIP_CRC_POS) Reg_addr = AA_CHIP_CRC_POS - 0x10;
							else if (Address == AC_CHIP_CRC_POS) Reg_addr = AC_CHIP_CRC_POS - 0x10;
							ping = 0;

							memset(DataBuf_bak, 0xff, 16);
							memcpy((unsigned char*)DataBuf_bak + 16, (unsigned char* )WriteDataBuf, 16);
							lbt_wrbytes(port, RX_P0, (FLASH_WRITE_DATA_0 ), 32, (unsigned char* )DataBuf_bak);

						}


					}

					if(ping == 0) {
						liberty_block_write(port, section, Reg_addr);
					}
#ifdef VERIFY_IN_FLASHING
					if((ping == 0) && (update_verify_flag == 1)) {
						//read
						lbt_wrbyte(port, RX_P0, R_FLASH_RW_CTRL , 0x00);
						lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , FLASH_READ);
						// wait flash read  Sequence done
						while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));
						lbt_rdbytes(port, RX_P0, FLASH_READ_DATA_0, 32, (unsigned char *)ReadDataBuf);

						if(memcmp((unsigned char *)DataBuf_bak, (unsigned char *)ReadDataBuf, 32)) {
							update_result = 1;
							TRACE("##############Hex check fail !##############\n");
							TRACE_ARRAY((unsigned char *)ReadDataBuf, 32);
							TRACE("HEX file:\n");
							TRACE_ARRAY((unsigned char *)DataBuf_bak, 32);
						}
					}
#endif
				} else {
					TRACE("Byte count in Hex file MUST be 16!\n");
				}

			} else {
				TRACE1("Address alignment error: Address=%04X\n", Address);
			}
		} else if(RecordType == 1) {
			if ((g_bBurnHex&INDEX_OCM) == INDEX_OCM){
				g_bBurnHex = g_bBurnHex&(~INDEX_OCM);
			}else if ((g_bBurnHex&INDEX_CUSTOM) == INDEX_CUSTOM){
				g_bBurnHex = g_bBurnHex&(~INDEX_CUSTOM);
			}
			BurnHexIndex = 0;
			if (g_bBurnHex == 0){
			flash_update_abort(port);
			TRACE("Program finished.\n");
#ifdef VERIFY_IN_FLASHING
			if(update_result)
				TRACE("#############verify .. Failed!##################\n");
			else
				TRACE("Verify ... Pass!\n");
#endif
		}
		}
	} else {
		TRACE("Hex file error!\n");
		update_result = 1;
	}


}

void flash_read(unsigned char port, unsigned char section, unsigned long len)
{
	unsigned char ReadDataBuf[32];
	unsigned int Address;
	unsigned long length;

	flash_operation_initial(port);

	if(message_flag == 0)
		TRACE3("read hex port  %02x, section %02x len %02x\n", port, section, (uint)len);

	if(section) {
		//section B
		Address = FW_B_START_ADDR;
		length = len;
	} else {
		//section A
		Address = FW_A_START_ADDR;
		length = len;
	}

	do {
		liberty_block_read(port, section, Address, ReadDataBuf);
		if(message_flag == 0)
			TRACE_ARRAY((unsigned char *)ReadDataBuf, 32);
		Address += 32;

		if(Address >= length) {
			if(message_flag == 0)
				TRACE1("Read %x finished.\n", (uint)length);
			return;
		}
	} while((Address != 0xFC00) && (Address != 0x1F800));

	if(message_flag == 0)
		TRACE("Read all finished.\n");

}


void flash_read_start(unsigned char port, unsigned char section, unsigned long addr, unsigned long len)
{
	unsigned char ReadDataBuf[32];
	unsigned int Address;
	unsigned long length;

	flash_operation_initial(port);
	if(message_flag == 0)
		TRACE4("read hex port  %02x, section %02x, Address: %02x to %02x\n", port, section, (uint)addr, (uint)len);

	if(section) {
		//section B
		Address = addr;
		length = len;

	} else {
		//section A
		Address = addr;
		length = len;
	}

	do {
		liberty_block_read(port, section, Address, ReadDataBuf);
		if(message_flag == 0)
			TRACE_ARRAY((unsigned char *)ReadDataBuf, 32);
		Address += 32;

		if(Address >= length) {
			if(message_flag == 0)
				TRACE1("Read %x finished.\n", (uint)length);
			return;
		}
	} while((Address != 0xFC00) && (Address != 0x1F800));

}


void flash_erase_all(unsigned char port)
{
	TRACE("Chip erase start...!\n");

	power_on(port);

	TRACE(".");

	lbt_wrbyte_or(port, RX_P0, OCM_CTRL_0, OCM_RESET);
	flash_read(port, 0, 0x20);
	flash_operation_initial(port);
	flash_write_enable(port);
	lbt_wrbyte(port, RX_P0, FLASH_ERASE_TYPE , ChipErase);

	msleep(12);
       TRACE(". \n");

	flash_write_enable(port);
	lbt_wrbyte_or(port, RX_P0, R_FLASH_RW_CTRL , FLASH_ERASE_EN);

      TRACE(". \n");

	while(!(lbt_rdbyte(port, RX_P0, R_RAM_CTRL) & FLASH_DONE));

	flash_read(port, 0, 0x20);

	TRACE("Chip all blocks erase done ... \n");

}

/* program Flash with data in a HEX file, 32 bytes at a time */
/* When address is not 32-byte aligned, simply notify the user and exit */
/* Since address is always 32-byte aligned, and 32 bytes are written */
/* into Flash at a time, */
/* crossing 256-byte (FLASH_PAGE_SIZE) boundary will NEVER happen, */
/* thus no need to handle this. */
void flash_program(u8 port_id, int hex_index)
{

	if ((BurnHexIndex % 512) == 0)
		TRACE1("%d+\n",g_bBurnHex);
//	TRACE(".");

	if ((g_bBurnHex&INDEX_OCM)==INDEX_OCM)
		g_CmdLineBuf = (unsigned char *)OCM_FW_HEX[BurnHexIndex++];
	else if ((g_bBurnHex&INDEX_CUSTOM)==INDEX_CUSTOM)
		g_CmdLineBuf = (unsigned char *)CUSTOM_ZONE_HEX[BurnHexIndex++];
	else
		TRACE("File Index Error!\n");

	/*Burn hex file function*/
	prog_hex((const char *) g_CmdLineBuf, port_id, current_section);
}


unsigned short int strtoval(const unsigned char *str, unsigned char n)
{
	unsigned char c;
	unsigned char i;
	unsigned short int rsult = 0;

	for (i = 0; i < n; i++) {
		c = str[i];

		//rsult = rsult * 16;
		rsult = rsult << 4 ;

		if (c >= '0' && c <= '9')
			rsult += (c - '0');
		else if (c >= 'A' && c <= 'F')
			rsult += (c - 'A' + 10);
		else if (c >= 'a' && c <= 'f')
			rsult += (c - 'a' + 10);

	}
	return rsult;
}

#define RET_FAIL -1
#define RET_OK 0

char GetLineData(const unsigned char *pLine, unsigned char *pByteCount,
                 unsigned int *pAddress, unsigned char *pRecordType,
                 unsigned char *pData)
{
	unsigned char checksum;
	unsigned char sum;
	unsigned char i;

	if ((*pLine) == ':') {

		*pByteCount = (unsigned char)strtoval(pLine + 1, 2);
		*pAddress = (unsigned int)strtoval(pLine + 1 + 2, 4);
		*pRecordType = (unsigned char)strtoval(pLine + 1 + 2 + 4, 2);

		sum = (unsigned char)(*pByteCount + *pAddress / 256 +	 *pAddress % 256 + *pRecordType);


		pLine += 1 + 2 + 4 + 2;
		for (i = *pByteCount; i != 0; i--) {
			if ((*pLine) != '\0') {
				*pData = (unsigned char)strtoval(pLine, 2);
				sum += *pData;
				pData++;
				pLine += 2;
			} else {
				return RET_FAIL;
			}
		}
//		if (sscanf(pLine, "%2x", ValBuf) == 1) {
		if ((*pLine) != '\0') {
			checksum = (unsigned char)strtoval(pLine, 2);

			if ((char)(sum + checksum) == 0)
				return RET_OK;
			else
				return RET_FAIL;

		} else
			return RET_FAIL;

	} else
		return RET_FAIL;

}

void SetLineData(unsigned char *pLine, unsigned char ByteCount,
                 unsigned short int Address, unsigned char RecordType,
                 unsigned char *pData)
{
	unsigned char checksum;

	snprintf(pLine, 10, ":%02hhX%04hX%02hhX",
	         ByteCount, Address, RecordType);
	pLine += 1 + 2 + 4 + 2;
	checksum = (unsigned char)(ByteCount + Address / 256 + Address % 256 + RecordType);
	for (; ByteCount;  ByteCount--) {
		snprintf(pLine, 3, "%02hhX", *pData);
		pLine += 2;
		checksum += *pData;
		pData++;
	}
	snprintf(pLine, 3, "%02hhX", -checksum);
	pLine += 2;
	*pLine = '\0';
}

void TRACE_ARRAY(unsigned char array[], unsigned char len)
{

	unsigned char i;

	i = 0;
	while (1) {
		TRACE1("%02X", array[i]);
		i++;
		if (i != len) {
			TRACE(" ");
		} else {
			TRACE("\n");
			break;
		}
	}

}


u8 burnhex(u8 port_id, int file_index)
{
	/* power on chip first*/
	power_on(port_id);

	flash_exit_sleep(port_id);

	flash_operation_initial(port_id);

	msleep(50);

	BurnHexIndex = 0;
	g_bBurnHex = (unsigned char) file_index ;
	current_section = 0;
	update_result = 0;

	TRACE1("Starting programing...%d\n",file_index);

	while  ((g_bBurnHex)&&(update_result==0))
		flash_program(port_id, file_index);

	power_standby(port_id);

	return update_result;

}

void verifyhex(u8 port_id, u8 section_index)
{

	/* power on chip first*/
	power_on(port_id);

	ocm_ver[0]= ReadReg(port_id, RX_P0, OCM_VERSION_REG);
	ocm_ver[1] = ReadReg(port_id, RX_P0, OCM_VERSION_REG + 1);

	if ((ocm_ver[0]==hex_ver[0])&&(ocm_ver[1]==hex_ver[1]))
		TRACE("<<<<<Main Ocm Version Verify Passed!>>>>>\n");
	else
		TRACE("#################Main Ocm Version Verify Failed##########!\n");

//	flash_exit_sleep(port_id);

	flash_operation_initial(port_id);


	msleep(50);

	if (section_index&INDEX_OCM)
		{

	BurnHexIndex = 0;
	g_bBurnHex = INDEX_OCM;
	current_section = 0;
	update_result = 0;

	TRACE("Starting verify mainocm hex...\n");

	while  (g_bBurnHex) {
		g_CmdLineBuf = (unsigned char *)OCM_FW_HEX[BurnHexIndex];

		/*Verify hex file function*/
		Hex_Verify((const char *) g_CmdLineBuf, port_id, current_section);
		
		if ((BurnHexIndex % 512) == 0)
			TRACE1("pass line %d\n",BurnHexIndex);

		BurnHexIndex++;

	}

		if (update_result==0)
			TRACE("<<<<<Main OCM Hex Verify Passed!>>>>>\n");


		}

	if (section_index&INDEX_CUSTOM)
		{
	
	BurnHexIndex = 0;
	g_bBurnHex = INDEX_CUSTOM;
	current_section = 1;
	update_result = 0;

	//TRACE("Starting verify custom hex...\n");

	while  (g_bBurnHex) {
		g_CmdLineBuf = (unsigned char *)CUSTOM_ZONE_HEX[BurnHexIndex];

		/*Verify hex file function*/
		Hex_Verify((const char *) g_CmdLineBuf, port_id, current_section);
		//if ((BurnHexIndex % 512) == 0)
		//	TRACE1("pass line %d\n",BurnHexIndex);
		BurnHexIndex++;
	}

	if (update_result==0)
		TRACE("<<<<<Custom Zone Verify Passed!>>>>>\n");
		}

	//TRACE("Done!\n");

	power_standby(port_id);

}


#define VERSION_ADDR 0x1000

unsigned char readhexver(unsigned char selectHex)
{
	unsigned char *LineBuf;
	unsigned short int Index = 0;
	unsigned short int Lines = 0;
	unsigned char WriteDataBuf[16];
	unsigned char ByteCount;
	unsigned int Address;
	unsigned char RecordType;

	if (selectHex == 1)
		Lines = ARRAY_SIZE(OCM_FW_HEX);
	else
		Lines = ARRAY_SIZE(OCM_FW_HEX);

	while (Index < Lines) {
		if (selectHex == 1)
			LineBuf = (unsigned char *)OCM_FW_HEX[Index++];
		else
			LineBuf = (unsigned char *)OCM_FW_HEX[Index++];

		if (GetLineData(LineBuf, &ByteCount,
		                &Address, &RecordType, WriteDataBuf) == 0) {
			if (RecordType == 0) {
				if (Address == VERSION_ADDR) {
					/* main version*/
					hex_ver[0] = WriteDataBuf[VERSION_ADDR - Address];
					/* minor version*/
					hex_ver[1] = WriteDataBuf[VERSION_ADDR + 1 - Address];
					break;
				}
			} else if (RecordType == 1) {
				TRACE("Hex file end!\n");
				return 1;
			}
		} else {
			TRACE("Hex file error!\n");
			return 1;
		}
	}
	return 0;
}
//LBT-829 the OCM bin file version number failed
unsigned char readbinver(unsigned char selectbin)
{
	hex_ver[0] = OCM_FW_BIN[VERSION_ADDR];
	hex_ver[1] = OCM_FW_BIN[VERSION_ADDR+1];

	return 0;
}


void burnhexauto(u8 port_id)
{
	unsigned char VerBuf[2];
	unsigned short int VersionHex;
	unsigned short int VersionChip;

	TRACE("burnhexauto.\n");

	/* power on chip first*/
	power_on(port_id);

	readhexver(0);
	VerBuf[0]=hex_ver[0];
	VerBuf[1]=hex_ver[1];
	
	VersionHex = ((unsigned short int)VerBuf[0] << 8) + VerBuf[1];
	TRACE2("Version from hex: %2hx%2hx\n",
	       VersionHex >> 8, VersionHex & 0xFF);

	VerBuf[0] = ReadReg(port_id, RX_P0, OCM_VERSION_REG);
	VerBuf[1] = ReadReg(port_id, RX_P0, OCM_VERSION_REG + 1);

	VersionChip = ((unsigned short int)VerBuf[0] << 8) + VerBuf[1];
	TRACE2("Version from chip: %2hx%2hx\n",
	       VersionChip >> 8, VersionChip & 0xFF);

	/* Check load done*/
	if (VersionHex == VersionChip) {
		power_standby(port_id);
		TRACE("Same version not need update.\n");
		return;
	}

	TRACE("Update version.\n");
	flash_erase_all(port_id);
	burnhex(port_id, 0);
}


u8 burnbin(u8 port_id, int file_index)
{
unsigned char *current_p;
unsigned int cur_addr;
unsigned int burn_size;
unsigned char DefaultBuf[32]={\
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,\
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,\
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,\
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

#ifdef VERIFY_IN_FLASHING
	unsigned char ReadDataBuf[32];
#endif

	if ((file_index & (INDEX_OCM|INDEX_CUSTOM))==0){
		TRACE("Please select correct one Bin file!\n");
		return 2;
	}
	
	g_bBurnHex = (unsigned char)file_index ;

	/* power on chip first*/
	power_on(port_id);

	flash_exit_sleep(port_id);
	
	flash_operation_initial(port_id);

	msleep(50);

	update_result = 0;
	current_section = 0;
	
	while((g_bBurnHex!=0)&&(update_result ==0))
		{
	if (g_bBurnHex & INDEX_OCM){
		TRACE("Burn MainOCM Bin file!\n");
		current_p = OCM_FW_BIN;
		cur_addr = 0;
		burn_size = bin_size;
	}else if (g_bBurnHex & INDEX_CUSTOM){
		TRACE("Burn Custom Zone Bin file!\n");
		current_p = CUST_ZONE_BIN;
		cur_addr = 0xE000;
		burn_size = 0xE000 + cust_size;
	}
	TRACE("Starting programing...\n");

	while  (cur_addr < burn_size){
		if ((cur_addr%4096) == 0) TRACE("=\n");
		if(memcmp((unsigned char *)current_p, (unsigned char *)DefaultBuf, 32)) {
			lbt_wrbytes(port_id, RX_P0, (FLASH_WRITE_DATA_0), 32, (unsigned char* )current_p);
			liberty_block_write(port_id, current_section, cur_addr);
#ifdef VERIFY_IN_FLASHING
		//read
		lbt_wrbyte(port_id, RX_P0, R_FLASH_RW_CTRL , 0x00);
		lbt_wrbyte_or(port_id, RX_P0, R_FLASH_RW_CTRL , FLASH_READ);
		// wait flash read  Sequence done
		while(!(lbt_rdbyte(port_id, RX_P0, R_RAM_CTRL) & FLASH_DONE));
		lbt_rdbytes(port_id, RX_P0, FLASH_READ_DATA_0, 32, (unsigned char *)ReadDataBuf);
		if(memcmp((unsigned char *)current_p, (unsigned char *)ReadDataBuf, 32)) {
			update_result = 1;
			TRACE1("##############Hex check fail !##############0x%x#\n",cur_addr);
			TRACE_ARRAY((unsigned char *)ReadDataBuf, 32);
			TRACE("HEX file:\n");
			TRACE_ARRAY((unsigned char *)current_p, 32);
			break;
		}
#endif
		}
		
		cur_addr += 32;
		current_p += 32;
		
		
	
	}
		if ((g_bBurnHex&INDEX_OCM) == INDEX_OCM){
				g_bBurnHex = g_bBurnHex&(~INDEX_OCM);
		}else if ((g_bBurnHex&INDEX_CUSTOM) == INDEX_CUSTOM){
			g_bBurnHex = g_bBurnHex&(~INDEX_CUSTOM);
		}
	
	}
	
	power_standby(port_id);

	return update_result;

}


void GetFwVersion(u8 port_id,  u8 *VerBuf )
{
	/* power on chip first*/
	power_on(port_id);
	

	*VerBuf = ReadReg(port_id, RX_P0, OCM_VERSION_REG);
	//msleep(1);
	*(VerBuf+1) = ReadReg(port_id, RX_P0, OCM_VERSION_REG + 1);

	power_standby(port_id);

}



u8 MatchHex(u8 port_id)
{
  	unsigned char chip_version;

	/* power on chip first*/
	power_on(port_id);
	
	if ((hex_ver[0]==0x24)&&(hex_ver[1]==0xF8))
	{/*For v0.4 image, version number is not available from hex*/
		hex_ver[0] = 0x04;
		hex_ver[1] = 0x00;
	}
	
	/*read chip revision from register.*/
	chip_version = lbt_rdbyte(port_id, RX_P0, 0x95);

	if (chip_version == 0) {
		TRACE1("Anx Chip[%x] Not Found!\n", (int)port_id);
		return 0;
	}

	if (((hex_ver[0]>=05)&&(chip_version<0xAC))||\
		((hex_ver[0]<05)&&(chip_version>=0xAC)))
	{/*image version <0.5 is not suitable for AC chip*/
	  /*image version >=0.5 is not suitable for AA chip*/
		TRACE("IMAGE doesn't match CHIP.No Update!\n");
		return 0;
	}
	else
		return 1;

}





