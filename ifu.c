/** @file
  This is a simple shell application

  Copyright (c) 2008, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
  
 *-----------------------------------------------------------------------------------
 * Filename: ifu.c         
 *
 * Function: EC flash  utility
 *
 * Author  : Donald Huang  <donald.huang@ite.com.tw>
 * 
 * Copyright (c) 2013 - , ITE Tech. Inc. All Rights Reserved. 
 *
 * You may not present,reproduce,distribute,publish,display,modify,adapt,
 * perform,transmit,broadcast,recite,release,license or otherwise exploit
 * any part of this publication in any form,by any means,without the prior
 * written permission of ITE Tech. Inc.
 *
 * 2013.07.01 <Donald> First Release
 * 2013.09.04 <Donald> Change to use LIBC
 *---------------------------------------------------------------------------------*/
 
#include  <Uefi.h>
#include  <Library/UefiLib.h>
//#include  <Library/ShellCEntryLib.h>

#include  <stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<Library/UefiBootServicesTableLib.h>
#include 	<Library/IoLib.h>


#include <Library/TimerLib.h>



#define inportb(value) IoRead8(value)
#define outportb(port,value) IoWrite8(port,value)

//---------------------------------------------------------------------------
// Global function pointers & Variables & Resources
//---------------------------------------------------------------------------
#define gfpOut32 outportb
#define gfpInp32 inportb

//---------------------------------------------------------------------------
#define _IBF            0x02    //PMIC Input buffer full
#define _OBF            0x01    //PMIC Output buffer full
#define _PMIO_DELAY     60000   //PMIC access delay
#define _NO_TIMEOUT     1       //0:Timeout 1:No Timeout

#define WAITING_DONE  1  //0:No waiting  1:Waiting finish
//---------------------------------------------------------------------------
// I/O Variables & Resources
//---------------------------------------------------------------------------
UINT8     STATUS, PMC_CMD_PORT, PMC_DTA_PORT, ECRAM_BASE,DATA_OUT;
int     RW_TIMER;



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Read Data
//---------------------------------------------------------------------------
UINT8 PM_R_DTA(void)
{
        //-------------------------------------------------------------------
        // Wait data port have data (PMIO OBF full)
        STATUS = 0x00;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS == 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & _OBF;
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        //-------------------------------------------------------------------
        // Read data port
        DATA_OUT = gfpInp32(PMC_DTA_PORT);
        if (RW_TIMER == 0)      return 0xFF;
        //-------------------------------------------------------------------
        return DATA_OUT;
}
//---------------------------------------------------------------------------
// Write Data
//---------------------------------------------------------------------------
int PM_W_DTA(UINT8 DTA_IN)
{
        //-------------------------------------------------------------------
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & _IBF;
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        //-------------------------------------------------------------------
        if (RW_TIMER == 0)      return 0xFFFFFFFF;
        //-------------------------------------------------------------------
        gfpOut32(PMC_DTA_PORT, DTA_IN);

#if  (WAITING_DONE)
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & _IBF;
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        //-------------------------------------------------------------------
        if (RW_TIMER == 0)      return 0xFFFFFFFF;
        //-------------------------------------------------------------------
#endif

        return DATA_OUT;
}
//---------------------------------------------------------------------------
// Command Write
//---------------------------------------------------------------------------
int PM_W_CMD(UINT8 CMD_IN)
{
        //-------------------------------------------------------------------
        // 1. Check PMIO IBF&OBF empty
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & 0x03;
                gfpInp32(PMC_DTA_PORT);
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        if (RW_TIMER == 0)
        {
                return 0xFFFFFFFF;
        }
        //-------------------------------------------------------------------
        // 2. Send command to command port
        gfpOut32(PMC_CMD_PORT, CMD_IN);
        //-------------------------------------------------------------------
        // 3. Check command finished (PMIO IBF empty)
#if  (WAITING_DONE)
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & _IBF;
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        if (RW_TIMER == 0)      return 0xFFFFFFFF;
#endif
        //-------------------------------------------------------------------
        return 0x00000000;
}
//---------------------------------------------------------------------------
// Command Write Data
//---------------------------------------------------------------------------
int PM_W_CMD_W_DTA(UINT8 CMD_IN, UINT8 DTA_IN)
{
        //-------------------------------------------------------------------
        // 1. Check PMIO IBF&OBF empty
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & 0x03;
                gfpInp32(PMC_DTA_PORT);
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        if (RW_TIMER == 0)
        {
                return 0xFFFFFFFF;
        }
        //-------------------------------------------------------------------
        // 2. Send command to command port
        gfpOut32(PMC_CMD_PORT, CMD_IN);
        //-------------------------------------------------------------------
        // 3. Check command finished (PMIO IBF empty)
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & _IBF;
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        if (RW_TIMER == 0)      return 0xFFFFFFFF;
        //-------------------------------------------------------------------
        // 4. Send data to data port
        gfpOut32(PMC_DTA_PORT, DTA_IN);
        // 5. Check data finished (PMIO IBF&OBF empty)
#if  (WAITING_DONE)
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & _IBF;
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        if (RW_TIMER == 0)      return 0xFFFFFFFF;
        //-------------------------------------------------------------------
#endif
        return 0x00000000;
}


//---------------------------------------------------------------------------
// Command Read Data
//---------------------------------------------------------------------------
UINT8 PM_W_CMD_R_DTA(UINT8 CMD_IN)
{
        //int     STATUS, RW_TIMER, DATA_OUT;
        //-------------------------------------------------------------------
        // 1. Check PMIO IBF&OBF empty
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & 0x03;
                gfpInp32(PMC_DTA_PORT);
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        if (RW_TIMER == 0)
        {
                gfpInp32(PMC_DTA_PORT);    //Clear Data
                return 0xFF;
        }
        //-------------------------------------------------------------------
        // 2. Send command to command port
        gfpOut32(PMC_CMD_PORT, CMD_IN);
        //-------------------------------------------------------------------
        // 3. Check command finished (PMIO IBF empty)
        STATUS = 0xFF;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS != 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & _IBF;
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        if (RW_TIMER == 0)      return 0xFF;
        //-------------------------------------------------------------------
        // 4. Wait data port have data (PMIO OBF full)
        STATUS = 0x00;
        RW_TIMER = _PMIO_DELAY;
        while ((STATUS == 0x00) && (RW_TIMER != 0x00))
        {
                STATUS = gfpInp32(PMC_CMD_PORT) & _OBF;
                gfpOut32(0xED, 0xAA);   //IO Delay
#if     (_NO_TIMEOUT == 0)
                RW_TIMER--;
#endif
        }
        //-------------------------------------------------------------------
        // 5. Read data port
        DATA_OUT = gfpInp32(PMC_DTA_PORT);
        if (RW_TIMER == 0)      return 0xFF;
        //-------------------------------------------------------------------
        return DATA_OUT;
}


void burnstart(void)
{
    PMC_CMD_PORT = 0x0066;
    PMC_DTA_PORT = 0x0062;
    PM_W_CMD_W_DTA(0xB1, 0x00); //STOP SMBUS-0 
}

void burnstop(void)
{
    PM_W_CMD_W_DTA(0xB0, 0x00); //Start SMBUS-0 
}

#define DELAY_INTERVAL 100
#define DELAY_INTERVAL_N 2500


UINT8 Readbyte(UINT8 addr, UINT8 reg)
{
#if 0
    PM_W_CMD_W_DTA(0xB2, addr);
    PM_W_CMD_W_DTA(0xB3, reg);
    PM_W_CMD_W_DTA(0xB6, 0x01);	//START READ REG BYTE
#else
    PM_W_CMD_W_DTA(0xBF, addr);
    PM_W_DTA(reg);
#endif
#if 1
//   MicroSecondDelay(DELAY_INTERVAL);
    while (PM_W_CMD_R_DTA(0xB7) != 0) MicroSecondDelay(DELAY_INTERVAL);
    /* RESERVED CHECK BUSY */
#endif
    return PM_W_CMD_R_DTA(0xB8);

}

void Writebyte(UINT8 addr, UINT8 reg, UINT8 dat)
{
#if 0
    PM_W_CMD_W_DTA(0xB2, addr);
    PM_W_CMD_W_DTA(0xB3, reg);
    PM_W_CMD_W_DTA(0xB4, dat);
    PM_W_CMD_W_DTA(0xB6, 0x81); //START WRITE REG BYTE 
#if 1
//   MicroSecondDelay(DELAY_INTERVAL);
    while (PM_W_CMD_R_DTA(0xB7) != 0) MicroSecondDelay(DELAY_INTERVAL);
    /* RESERVED CHECK BUSY */
#endif
    /* RESERVED CHECK BUSY(FINISHED) */
#else
    PM_W_CMD_W_DTA(0xBE, addr);
       PM_W_DTA(reg);
       PM_W_DTA(dat);
#if 1
//   MicroSecondDelay(DELAY_INTERVAL);
    while (PM_W_CMD_R_DTA(0xB7) != 0) MicroSecondDelay(DELAY_INTERVAL);
    /* RESERVED CHECK BUSY */
#endif

#endif
}

void WriteNBytes(UINT8 addr, UINT8 reg, UINT8 n, UINT8 *buf )
{
    UINT8 i;
    PM_W_CMD_W_DTA(0xB2, addr);
    PM_W_CMD_W_DTA(0xB3, reg);
    PM_W_CMD_W_DTA(0xBB, *buf);
//    PM_W_CMD_W_DTA(0xBB, 1);
    for(i=1;i<n;i++){
       PM_W_DTA(*(buf+i));
 //     PM_W_DTA(i);
	}
    PM_W_CMD_W_DTA(0xBA, n);	//BYTE COUNT
    PM_W_CMD_W_DTA(0xB6, 0x83); //START WRITE BLOCK n BYTES
#if 1
//   MicroSecondDelay(DELAY_INTERVAL_N);
    while (PM_W_CMD_R_DTA(0xB7) != 0) MicroSecondDelay(DELAY_INTERVAL);
    /* RESERVED CHECK BUSY */
#endif
    /* RESERVED CHECK BUSY(FINISHED) */
}

void ReadNBytes(UINT8 addr, UINT8 reg, UINT8 n, UINT8 *buf )
{
    UINT8 i;
    PM_W_CMD_W_DTA(0xB2, addr);
    PM_W_CMD_W_DTA(0xB3, reg);
    PM_W_CMD_W_DTA(0xBA, n);	//BYTE COUNT
    PM_W_CMD_W_DTA(0xB6, 0x03); //START READ BLOCK n BYTES
#if 1
//   MicroSecondDelay(DELAY_INTERVAL_N);
    while (PM_W_CMD_R_DTA(0xB7) != 0) MicroSecondDelay(DELAY_INTERVAL);
    /* RESERVED CHECK BUSY */
#endif
   *buf = PM_W_CMD_R_DTA(0xBC);   
    for(i=1;i<n;i++){
		*(buf+i) = PM_R_DTA();   
    	}

}
