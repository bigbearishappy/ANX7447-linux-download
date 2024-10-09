#include<stdio.h>

/*******************************************************************************************
Copyright (c) 2017, Analogix Semiconductor, Inc.
Description: This code is to introduce how to erase/programming Flash in a sample way,
 it is for the fixed the firmware pointer as A block, after
 erased Flash, all of Flash data are 0xFF, the firmware pointer(0x1F800)
is 0xFF, so the block A shall be selected.
Revision History:
*********************************************************************************************/

#define DATA_BLOCK_SIZE 32
#define I2C_TCPC_ADDR 0x58
#define I2C_SPI_ADDR 0x7E
#define R_RAM_LEN_H 0x03
#define FLASH_ADDR_EXTEND 0x80
#define R_RAM_CTRL 0x05
#define FLASH_DONE 0x80
#define R_FLASH_ADDR_H 0x0c
#define R_FLASH_ADDR_L 0x0d
#define FLASH_WRITE_DATA_0 0x0e
#define R_FLASH_LEN_H 0x2e
#define R_FLASH_LEN_L 0x2f
#define R_FLASH_RW_CTRL 0x30
#define GENERAL_INSTRUCTION_EN 0x40
#define FLASH_ERASE_EN 0x20
#define WRITE_STATUS_EN 0x04
#define FLASH_READ 0x02
#define FLASH_WRITE 0x01
#define R_FLASH_STATUS_0 0x31
#define FLASH_INSTRUCTION_TYPE 0x33
#define FLASH_ERASE_TYPE 0x34
#define R_FLASH_STATUS_REGISTER_READ_0 0x35
#define WIP 0x01
#define FLASH_READ_DATA_0 0x3C
#define R_I2C_0 0x5C
#define read_Status_en 0x80
#define OCM_CTRL_0 0x6e
#define OCM_RESET 0x40
#define ADDR_GPIO_CTRL_0 0x88
#define SPI_WP 0x80
#define WriteEnable 0x06
#define DeepPowerDown 0xB9
#define ChipErase 0x60 // Chip Erase

/*
* Description: I2C read function
* Arguments : devAddr is I2C address, regAddr is offset
* Returns : Register value
*/
unsigned char i2c_ReadByte(unsigned char devAddr, unsigned char regAddr)
{
 unsigned char tmpData = 0;
 ReadRegister(devAddr, regAddr,&tmpData);
 return tmpData;
}

/*
* Description: I2C write function
* Arguments : devAddr is I2C address, regAddr is offset, value is register value
* Returns : Register value
*/
void i2c_WriteByte(unsigned char devAddr, unsigned char regAddr, unsigned char value)
{
 WriteRegister(devAddr, regAddr, value);
}
#define i2c_WriteByte_or(dev, reg, value) i2c_WriteByte(dev, reg, (i2c_ReadByte(dev, reg) | value))
#define i2c_WriteByte_and(dev, reg, value) i2c_WriteByte(dev, reg, (i2c_ReadByte(dev, reg) & value))

/*
* Description: wake up for flash operation
* Arguments : No
* Returns : No
*/
void flash_operation_init(void)
{
/*chip wake-up, to read one register of I2C_TCPC_ADDR to
to wake up chip */
i2c_ReadByte(I2C_TCPC_ADDR, 0x0);
/*Add 10ms delay to make sure chip is waked up*/
 delay_ms(10);
 /*Reset On chip MCU*/
 i2c_WriteByte_or(I2C_SPI_ADDR, OCM_CTRL_0, OCM_RESET);
 /*read flash for deep power down wake up*/
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_LEN_H, 0x0);
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_LEN_L, 0x1f);
 i2c_WriteByte_or(I2C_SPI_ADDR, R_FLASH_RW_CTRL , FLASH_READ);
 delay_ms(4);
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE));
 delay_ms(5);
}

/*
* Description: unprotect write protect
* Arguments : No
* Returns : No
*/
void flash_unprotect(void)
{
 /* disable Hardware Protected Mode (HPM) */
 i2c_WriteByte_or(I2C_SPI_ADDR, ADDR_GPIO_CTRL_0 , SPI_WP);
 i2c_WriteByte(I2C_SPI_ADDR, FLASH_INSTRUCTION_TYPE , WriteEnable);
 i2c_WriteByte_or(I2C_SPI_ADDR, R_FLASH_RW_CTRL , GENERAL_INSTRUCTION_EN);
 /* wait for Write Enable (WREN) Sequence done*/
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE));
 /* disable protect*/
 i2c_WriteByte_and(I2C_SPI_ADDR, R_FLASH_STATUS_0 , 0x43);
 i2c_WriteByte_or(I2C_SPI_ADDR, R_FLASH_RW_CTRL , WRITE_STATUS_EN);
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE));
 delay_ms(10);
}

/*
* Description: flash write
* Arguments : address is flash address, *buf is firmware data
* Returns : No
* Note: The data length must be 32 bytes, so there is no one argument for
 Length, the default length is 32. If data length is less than 32 bytes,
 have to fill in 0xFF.
*/
void flash_write(unsigned int address, unsigned char *buf)
{
 unsigned char i;
 flash_operation_init();
 flash_unprotect();
 /*Move data to registers*/
 for(i = 0; i < DATA_BLOCK_SIZE; i++)
 i2c_WriteByte(I2C_SPI_ADDR, (FLASH_WRITE_DATA_0 +i), buf[i]);

 /*Flash write enable */
 i2c_WriteByte(I2C_SPI_ADDR, FLASH_INSTRUCTION_TYPE , WriteEnable);
 i2c_WriteByte_or(I2C_SPI_ADDR, R_FLASH_RW_CTRL , GENERAL_INSTRUCTION_EN);
 /* wait for Write Enable (WREN) Sequence done*/
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE));
 /* write flash address*/
 i2c_WriteByte_and(I2C_SPI_ADDR, R_RAM_LEN_H , ~FLASH_ADDR_EXTEND);
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_ADDR_H, (unsigned char)((address ) >> 8));
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_ADDR_L, (unsigned char)((address ) & 0xff));
 /* write data length*/
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_LEN_H, 0x0);
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_LEN_L, 0x1f);
 /* flash write star*/
 i2c_WriteByte_or(I2C_SPI_ADDR, R_FLASH_RW_CTRL , FLASH_WRITE);
 /* wait flash write Sequence done*/
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE));
 do{
 i2c_WriteByte_or(I2C_SPI_ADDR, R_I2C_0 , read_Status_en);
 /* wait for Read Status Register (RDSR) Sequence done*/
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE));
 } while(i2c_ReadByte(I2C_SPI_ADDR, R_FLASH_STATUS_REGISTER_READ_0) & WIP);
}

/*
* Description: flash read
* Arguments : address is flash address, *buf is flash data
* Returns : No
* Note: The data length must be 32 bytes.
*/
void flash_read(unsigned int address, unsigned char *buf)
{
 unsigned char i;
 flash_operation_init();
 /* write flash address*/
 i2c_WriteByte_and(I2C_SPI_ADDR, R_RAM_LEN_H , ~FLASH_ADDR_EXTEND);
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_ADDR_H, (unsigned char)((address ) >> 8));
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_ADDR_L, (unsigned char)((address ) & 0xff));
 /* write data length*/
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_LEN_H, 0x0);
 i2c_WriteByte(I2C_SPI_ADDR, R_FLASH_LEN_L, 0x1f);
 /* flash read star*/
 i2c_WriteByte_or(I2C_SPI_ADDR, R_FLASH_RW_CTRL , FLASH_READ);
 delay_ms(4);
 /* wait flash read Sequence done*/
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE)); 
 delay_ms(5);
 /*Move data from registers to buffer*/
 for(i = 0; i < DATA_BLOCK_SIZE; i++)
 buf[i] = i2c_ReadByte(I2C_SPI_ADDR, (FLASH_READ_DATA_0 +i));
}

/*
* Description: erase all flash content
* Arguments : No
* Returns : No
* Note : No
*/
void flash_chip_erase(void)
{
 flash_operation_init();
 flash_unprotect();
 /* flash_write_enable*/
 i2c_WriteByte(I2C_SPI_ADDR, FLASH_INSTRUCTION_TYPE , WriteEnable);
 i2c_WriteByte_or(I2C_SPI_ADDR, R_FLASH_RW_CTRL , GENERAL_INSTRUCTION_EN);
 /* wait for Write Enable (WREN) Sequence done*/
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE));
 i2c_WriteByte(I2C_SPI_ADDR, FLASH_ERASE_TYPE , ChipErase);
 i2c_WriteByte_or(I2C_SPI_ADDR, R_FLASH_RW_CTRL , FLASH_ERASE_EN);
 while(!(i2c_ReadByte(I2C_SPI_ADDR, R_RAM_CTRL) & FLASH_DONE));
}

void main()
{
	printf("anx7447\n");
}
