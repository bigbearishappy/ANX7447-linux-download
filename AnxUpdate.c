/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include  <Library/ShellCEntryLib.h>
#include <Library/BaseMemoryLib.h>
#include  <Library/ShellLib.h> 
#include  <Library/MemoryAllocationLib.h> 

#include  <stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<Library/UefiBootServicesTableLib.h>
#include 	<Library/IoLib.h>
 
#include <Library/TimerLib.h>
#include <fcntl.h>
#include <sys/EfiSysCall.h>

#include "AnxUpdate.h"
#include "liberty_reg.h"
#include "Flash.h"


//unsigned char OCMFW[4*1024*HEX_LINE_SIZE] = {\"
//#include \"test.hex" 
//};

EFI_STATUS GetMemoryFile (char idex, char *InputFileName);
void I2C_TEST(unsigned char port_id);
void LogToFile ( char *InputFileName, char rslt);

/***
  Print a welcoming message.

  Establishes the main structure of the application.

  @retval  0         The application exited normally.
  @retval  Other     An error occurred.
***/
/*
INTN
EFIAPI
ShellAppMain (
    IN UINTN Argc,
    IN CHAR16 **Argv
)*/
  int
main (
  IN int  ArgC,
  IN char **ArgV
  )
{

	UINT8 Index,start_i,end_i,update_i[MAX_USBC_PORT_NUM];
	char *ocm_file;
	char *cust_file;
	EFI_STATUS loadfile_state = EFI_SUCCESS;
	EFI_STATUS loadcust_state = EFI_SUCCESS;


	start_i =0;
	end_i =MAX_USBC_PORT_NUM;

	for (Index=0; Index<MAX_USBC_PORT_NUM;Index++){
		update_i[Index] = 0;
	}

	printf("Anx Update tool version %s \n",TOOL_VER);

	if(ArgC>2) {
		ocm_file=(char*)ArgV[1];
		cust_file=(char*)ArgV[2];
		printf("Update New Fw: %s, Custom File: %s\n",ocm_file,cust_file);
		loadfile_state = GetMemoryFile(INDEX_OCM,ocm_file);
		loadcust_state = GetMemoryFile(INDEX_CUSTOM,cust_file);
		
		if (ArgC>2){
			/*LBT-705 anx7447 GBL plateform update efi need support 4 port i2c*/
			for (Index=0; Index<(ArgC-3);Index++){
			if (*(ArgV[Index+3]) == '0'){
				update_i[0] = 1;
			}else if (*(ArgV[Index+3]) == '1'){
				update_i[1] = 1;
			}else if (*(ArgV[Index+3]) == '2'){
				update_i[2] = 1;
			}else if (*(ArgV[Index+3]) == '3'){
				update_i[3] = 1;
			}else{
				start_i = 0;
				end_i = 0;
				printf("Usage:\n");
				printf("  AnxUpdate.efi <Main OCM file> <Custom Zone file> <id (0:chip0, 1:chip1, 2:chip2, 3:chip3)>\n");
				printf("For example: \n");
				printf("  AnxUpdate.efi mainocm.hex cust.hex 0 1  --update chip0&chip1 both main OCM and custom zone\n");
				printf("  AnxUpdate.efi mainocm.hex cust.hex 0 1 2 3  --update all 4 chips both main OCM and custom zone\n");
				printf("  AnxUpdate.efi mainocm.hex nocust 2    --update chip2 only main OCM\n");

			}

			}
		}
	}else{
			printf("Usage:\n");
			printf("  AnxUpdate.efi <Main OCM file> <Custom Zone file> <id (0:chip0, 1:chip1, 2:chip2, 3:chip3)>\n");
			printf("For example: \n");
			printf("  AnxUpdate.efi mainocm.hex cust.hex 0 1  --update chip0&chip1 both main OCM and custom zone\n");
			printf("  AnxUpdate.efi mainocm.hex cust.hex 0 1 2 3  --update all 4 chips both main OCM and custom zone\n");
			printf("  AnxUpdate.efi mainocm.hex nocust 2    --update chip2 only main OCM\n");
			start_i = 0;
			end_i = 0;
	}
	
	if ( loadfile_state == EFI_SUCCESS)
	{
#if 1
		if (bin_size > 0)
			readbinver(INDEX_OCM);
		else
			readhexver(INDEX_OCM);

		burnstart();
		if (loadcust_state == EFI_SUCCESS){
			//LBT-830 OCM file is a bin file, CUST_DEFINE file cannot be a HEX file
			if (((bin_size>0)&&(cust_size==0))||\
			((bin_size==0)&&(cust_size>0)))
			{
				printf("OCM file type and Custom file type should be the same!\n");
				printf("************ No Update *****************\n");
				start_i = 0;
				end_i = 0;
			}
		}
		for (Index = start_i; Index < end_i; Index ++) 
			if (update_i[Index]==1){
				TRACE1("ANX UPDATE FIRMWARE %x\n", Index);
			
			if (MatchHex(Index)) {
				flash_erase_all(Index);
				if (loadcust_state == EFI_SUCCESS)
					if (bin_size > 0)
						burnbin(Index, INDEX_OCM | INDEX_CUSTOM);
					else
						burnhex(Index, INDEX_OCM | INDEX_CUSTOM);
				else
					if (bin_size > 0)
						burnbin(Index, INDEX_OCM);
					else
						burnhex(Index, INDEX_OCM);
			}
			else
				continue;

#ifdef VERIFY_AFTER_FLASHING			
				verifyhex(Index, INDEX_OCM|INDEX_CUSTOM);
#endif
#ifdef VERIFY_BY_HW
				if ( loadcust_state == EFI_SUCCESS) 
					verifyhex(Index, INDEX_CUSTOM);
				else
					verifyhex(Index, INDEX_NONE);
#endif
		}
				

	{
		char ver0[MAX_USBC_PORT_NUM][2];
		for (Index = 0; Index < MAX_USBC_PORT_NUM; Index ++) 
		{
				GetFwVersion(Index,ver0[Index]);
		}

		for (Index = 0; Index < MAX_USBC_PORT_NUM; Index ++) 
		{
			TRACE4("ANX Chip[%x]-%X OCM Version = %02X%02X\n", Index,(int)CHIP_VER[Index],ver0[Index][0],ver0[Index][1]);
		}
	}

		burnstop();
#endif

	}

#ifdef LOG_RESULT_FILE
{
extern unsigned char hex_ver[2];
extern unsigned char ocm_ver[2];
extern unsigned char update_result;

	if (((ocm_ver[0]==hex_ver[0])&&(ocm_ver[1]==hex_ver[1]))&&(update_result==0))
		LogToFile("pass.txt",0);
	else
		LogToFile("fail.txt",-1);

}
#endif

	return 0;
}

const u8 Anx_Liberty_addr[MAX_USBC_PORT_NUM][MAX_I2C_NUM_PER_CHIP] = {
	{0x58, 0x7e, 0x7A, 0x84, 0x72}
#if ((MAX_USBC_PORT_NUM==2)||(MAX_USBC_PORT_NUM==4))
	, {0x52, 0x62, 0x6a, 0x74, 0x82}
#endif
#if (MAX_USBC_PORT_NUM==4)
	, {0x56, 0x6e, 0x78, 0x86, 0x76}
	,{0x54, 0x64, 0x68,0x6c,0x5C}
#endif

};


unsigned char ReadReg(unsigned char port_id, unsigned char i2c_id, unsigned char RegAddr)
{
	int ret = 0;
	unsigned char DevAddr;

	DevAddr = Anx_Liberty_addr[port_id][i2c_id];

	/*here we get real I2C address, start operation. */
	ret= Readbyte(DevAddr, RegAddr);

	/*Read Done! */
	return (uint8_t) ret;

}


int ReadBlockReg(unsigned char port_id, unsigned char i2c_id, u8 RegAddr, u8 len, u8 *dat)
{
	UINT8 ret = 0;
	unsigned char DevAddr;

	DevAddr = Anx_Liberty_addr[port_id][i2c_id];


#ifdef NO_BLOCKREAD
	for(ret=0;ret<len;ret++)
		dat[ret] = Readbyte(DevAddr, (RegAddr+ret));
#else
	/*here we get real I2C address, start operation. */
	ReadNBytes(DevAddr, RegAddr, len, dat);
#endif
	ret = 0;
	/*Read Done! */

	return (int)ret;
}


int WriteBlockReg(unsigned char port_id, unsigned char i2c_id, u8 RegAddr, u8 len,
                   u8 *dat)
{
	int ret = 0;
	unsigned char DevAddr;

	DevAddr = Anx_Liberty_addr[port_id][i2c_id];

	/*here we get real I2C address, start operation. */
	WriteNBytes(DevAddr, RegAddr,len, dat);
	ret = 0;
	/*Write Done! */

	return (int)ret;
}

void WriteReg(unsigned char port_id, unsigned char i2c_id, unsigned char RegAddr,
              unsigned char RegVal)
{
	unsigned char DevAddr;

	DevAddr = Anx_Liberty_addr[port_id][i2c_id];

	/*here we get real I2C address, start operation. */
	Writebyte(DevAddr, RegAddr, RegVal);

}


EFI_STATUS GetMemoryFile ( char idex, char *InputFileName)
{
  EFI_FILE_HANDLE   FileHandle; 
  RETURN_STATUS     Status; 
  EFI_FILE_INFO     *FileInfo = NULL; 
  EFI_HANDLE        *HandleBuffer=NULL; 
  UINTN             ReadSize,i; 
  CHAR8 *p_Temp , file_type;
  CHAR16 file_name[50];
  unsigned char *p_img;
  unsigned int *p_size;

  p_Temp = InputFileName;
  for(i=0;i<50;i++){
  	file_name[i] = *p_Temp;
	p_Temp++;
	if (file_name[i]=='\0') break;
  	}

  file_type = 0;
  if (((file_name[i-3]=='b')&&(file_name[i-2]=='i')&&(file_name[i-1]=='n'))|| \
  	((file_name[i-3]=='B')&&(file_name[i-2]=='I')&&(file_name[i-1]=='N')))
  	file_type = 1; //binary file.
  	

  //Open the file given by the parameter 
  Status = ShellOpenFileByName(file_name,
        (SHELL_FILE_HANDLE *)& 
                FileHandle, 
                EFI_FILE_MODE_READ , 0); 
  if(Status != RETURN_SUCCESS) {
  	if (idex==INDEX_OCM)
        	Print(L"Main OCM OpenFile failed!\n");
	else
		Print(L"No Custom Zone File!\n");
	 //Print(L"file name = %s\n",file_name); 
        return EFI_ABORTED; 
      }          

  //Get file size      
  FileInfo = ShellGetFileInfo( (SHELL_FILE_HANDLE)FileHandle);   
  //Allocate a memory buffer 
  HandleBuffer = AllocateZeroPool((UINTN) FileInfo-> FileSize); 
  if (HandleBuffer == NULL) { 
      return (SHELL_OUT_OF_RESOURCES);   } 
  ReadSize=(UINTN) FileInfo-> FileSize; 

  //Load the whole file to the buffer 
  Status = ShellReadFile(FileHandle,&ReadSize,HandleBuffer); 
  //Close the source file 
  ShellCloseFile(&FileHandle); 


  p_Temp = (CHAR8*)HandleBuffer;

  if (idex == INDEX_OCM){
  	p_img = OCM_FW_BIN;
	p_size = &bin_size;
  } else if (idex == INDEX_CUSTOM){
  	p_img = CUST_ZONE_BIN;
	p_size = &cust_size;
  } else
   return EFI_ABORTED; 

  

  if (file_type==1){
	memcpy(p_img,p_Temp,(UINTN) FileInfo-> FileSize);
	(*p_size) = (unsigned int) FileInfo-> FileSize;
//	TRACE1 ("bin file size 0x%x:\n", (*p_size));
//	TRACE_ARRAY(OCM_FW_BIN,16);
//	TRACE ("last data:\n");
//	TRACE_ARRAY(p_img,16);


  }else{

//  TRACE ("original hex first line:\n");
//  TRACE_ARRAY(OCM_FW_HEX[0],HEX_LINE_SIZE);

  
     for (i=0; ; i++) {
	while(*p_Temp!=':') p_Temp++;
	if (idex == INDEX_OCM)
		memcpy(OCM_FW_HEX[i],p_Temp,HEX_LINE_SIZE);
	else
		memcpy(CUSTOM_ZONE_HEX[i],p_Temp,HEX_LINE_SIZE);
	
	p_Temp += 12;
	if (p_Temp > ((CHAR8*)HandleBuffer + (UINTN) FileInfo-> FileSize)) break;
    }

//  TRACE ("Update hex first line:\n");
//  TRACE_ARRAY(OCM_FW_HEX[0],HEX_LINE_SIZE);

// TRACE1 ("hex LAST LINE %x:\n",i-1);
// TRACE_ARRAY(OCM_FW_HEX[i-1],HEX_LINE_SIZE);
// TRACE_ARRAY(OCM_FW_HEX[i],HEX_LINE_SIZE);

  }
  FreePool (HandleBuffer);
  return EFI_SUCCESS; 

}


#define TEST_COUNT 1000
void I2C_TEST(unsigned char port_id)
{
	int i,j;
	char tmp,buf[32];
	char buf_out[32];


	burnstart();
	power_on(port_id);

	TRACE("START BYTE TEST:\n");
	for(i=0;i<TEST_COUNT;i++){
		
		lbt_wrbyte(port_id, RX_P0, R_FLASH_LEN_H, (unsigned char)i);

		if (lbt_rdbyte(port_id, RX_P0, R_FLASH_LEN_H)!=(unsigned char)i)
		{
			TRACE1("###BYTE DATA check fail !## %d\n",i);
			break;
		}
	}


	TRACE("START BLOCK TEST:\n");	
	memcpy(buf,"1234567890abcdef0987654321fedcba",32);
	for(i=0;i<TEST_COUNT;i++){

		tmp = buf[0];
		for(j=1;j<32;j++){
			buf[j-1]=buf[j];
		}
		buf[31]=tmp;
		
		lbt_wrbytes(port_id, RX_P0, (FLASH_WRITE_DATA_0), 32, (unsigned char* )buf);	
		TRACE1("%d block write and read.\n",i);
		lbt_rdbytes(port_id, RX_P0, (FLASH_WRITE_DATA_0), 32, (unsigned char* )buf_out);	

		if(memcmp(buf,buf_out,32)){
			TRACE("##############BLOCK DATA check fail !##############INPUT:\n");
			TRACE_ARRAY((unsigned char *)buf, 32);
			TRACE("OUTPUT:\n");
			TRACE_ARRAY((unsigned char *)buf_out, 32);
			break;
		}

	}

	TRACE("ALL TEST passed.\n");	

	
	power_standby(port_id);
	burnstop();


}

void LogToFile ( char *InputFileName, char rslt)
{
  EFI_FILE_HANDLE   FileHandle; 
  RETURN_STATUS     Status; 
  EFI_FILE_INFO     *FileInfo = NULL; 
//  EFI_HANDLE        *HandleBuffer=NULL; 
  
  UINTN             WSize;
  UINTN i; 
  CHAR8 *p_Temp ;
  CHAR16 file_name[50];
  CHAR8 wr_string[20]="success+1\r\n";
  CHAR8 wr2_string[20]="fail+1\r\n";


    // open file with create enabled
  p_Temp = InputFileName;
  for(i=0;i<50;i++){
  	file_name[i] = *p_Temp;
	p_Temp++;
	if (file_name[i]=='\0') break;
  	}
    
    Status = ShellOpenFileByName(file_name,(SHELL_FILE_HANDLE *)& FileHandle, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE, 0);
	  if(Status != RETURN_SUCCESS) { 
      		Print(L"Open Log File failed!\n"); 
	        return ; 
     }  

  //Get file size      
  FileInfo = ShellGetFileInfo( (SHELL_FILE_HANDLE)FileHandle);   

	ShellSetFilePosition( FileHandle, FileInfo-> FileSize);

	if (rslt == 0){//success
		WSize = 11;//sizeof(wr_string);
		
		Status = ShellWriteFile(FileHandle, &WSize, wr_string);
	}else{
		WSize = 8;//sizeof(wr_string);
		Status = ShellWriteFile(FileHandle, &WSize, wr2_string);
	}

  	if(Status != RETURN_SUCCESS) { 
        Print(L"Write LOG File failed!\n"); 
      }

	  ShellCloseFile(&FileHandle); 

}


