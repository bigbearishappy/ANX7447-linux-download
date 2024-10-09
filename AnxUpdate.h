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
#ifndef ANXUPDATE_H
#define ANXUPDATE_H

#define PORT_I2C_DIRECT_ACCESS 0xff

#define MAX_USBC_PORT_NUM 4

#define PD_PORT_COUNT MAX_USBC_PORT_NUM

#define TOOL_VER "1.0.01"

enum I2C_ADDRESS_DEF {
	SLAVE_ADDR_TCPC = 0,
	SLAVE_ADDR_SPI = 1,
	SLAVE_ADDR_EMTB = 2,
	SLAVE_ADDR_EMRB = 3,
	SLAVE_ADDR_BILLBOARD = 4,
	MAX_I2C_NUM_PER_CHIP
};
#ifndef u8
#define u8 UINT8
#endif

#ifndef uint8_t
#define uint8_t UINT8
#endif

#ifndef uint
#define uint UINT32
#endif

unsigned char ReadReg(u8 port_id, unsigned char DevAddr, unsigned char RegAddr);

void WriteReg(u8 port_id, unsigned char DevAddr, unsigned char RegAddr,
              unsigned char RegVal);
int ReadBlockReg(u8 port_id, unsigned char DevAddr, u8 RegAddr,
                 u8 len, u8 *dat);
int WriteBlockReg(u8 port_id, unsigned char DevAddr, u8 RegAddr,
                  u8 len, u8 *dat);


/*Liberty I2C opertation*/
#define lbt_rdbyte(port, addr, offset) ReadReg(port, addr, offset)

#define lbt_wrbyte(port, addr, offset, val) WriteReg(port, addr, offset, val)

#define lbt_rdbytes(port, addr, offset, n, pBuf) ReadBlockReg(port, addr, offset, n, pBuf)

#define lbt_wrbytes(port, addr, offset, n, pBuf) WriteBlockReg(port, addr, offset, n, pBuf)

#define lbt_wrbyte_or(port, address, offset, mask) \
		lbt_wrbyte(port, address, offset, ((unsigned char)lbt_rdbyte(port, address, offset) | (mask)))
#define lbt_wrbyte_and(port, address, offset, mask) \
	      lbt_wrbyte(port, address, offset, ((unsigned char)lbt_rdbyte(port, address, offset) & (mask)))


UINT8 Readbyte(UINT8 addr, UINT8 reg);
void Writebyte(UINT8 addr, UINT8 reg, UINT8 dat);
void WriteNBytes(UINT8 addr, UINT8 reg, UINT8 n, UINT8 *buf );
void ReadNBytes(UINT8 addr, UINT8 reg, UINT8 n, UINT8 *buf );

void burnstart(void);
void burnstop(void);




/*
#define TRACE(a) Print(L##a)
#define TRACE1(a,b) Print(L##a,b)
#define TRACE2(a,b,c) Print(L##a,b,c)
#define TRACE3(a,b,c,d) Print(L##a,b,c,d)
#define TRACE4(a,b,c,d,e) Print(L##a,b,c,d,e)
*/
#define TRACE(a) printf(a)
#define TRACE1(a,b) printf(a,b)
#define TRACE2(a,b,c) printf(a,b,c)
#define TRACE3(a,b,c,d) printf(a,b,c,d)
#define TRACE4(a,b,c,d,e) printf(a,b,c,d,e)


#endif  // end of HEXFILE_H definition

