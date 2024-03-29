#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <wiringPi.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define PCD_IDLE              0x00               
#define PCD_AUTHENT           0x0E              
#define PCD_RECEIVE           0x08              
#define PCD_TRANSMIT          0x04              
#define PCD_TRANSCEIVE        0x0C               
#define PCD_RESETPHASE        0x0F               
#define PCD_CALCCRC           0x03               

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26              
#define PICC_REQALL           0x52               
#define PICC_ANTICOLL1        0x93               
#define PICC_ANTICOLL2        0x95               
#define PICC_AUTHENT1A        0x60              
#define PICC_AUTHENT1B        0x61              
#define PICC_READ             0x30               
#define PICC_WRITE            0xA0              
#define PICC_DECREMENT        0xC0               
#define PICC_INCREMENT        0xC1               
#define PICC_RESTORE          0xC2               
#define PICC_TRANSFER         0xB0               
#define PICC_HALT             0x50               

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte
#define MAXRLEN  18

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01    
#define     ComIEnReg             0x02    
#define     DivlEnReg             0x03    
#define     ComIrqReg             0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  		  0x3F

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
#define 	MI_OK                 0
#define 	MI_NOTAGERR           1
#define 	MI_ERR                2

#define	SHAQU1		0X01
#define	KUAI4			0X04
#define	KUAI7			0X07
#define	REGCARD		0xa1
#define	CONSUME		0xa2
// #define READCARD	0xa3
#define ADDMONEY	0xa4

#define RFID_DEBUG(a)	puts(a)
#define RFID_DEBUGF		printf
#define SPI_DEV "/dev/spidev0.0"
#define RFID_DelayMs(s)	usleep(s*1000)
static int __spidev = 0;
static struct spi_ioc_transfer spixfr;

static uint32_t mode = SPI_MODE_0;
static uint8_t bits = 8;
static uint32_t speed = 100000;


static void pabort(const char *s)
{
	perror(s);
	abort();
}
void MFRC522_HAL_Delay(unsigned int ms){
	usleep(ms * 1000);
}
void MFRC522_HAL_init(void) {
	int ret;

	//RFID_DEBUG("Try to open device "SPI_DEV);
	__spidev = open(SPI_DEV, O_RDWR);
	if (__spidev < 0) {
		pabort("Device Cannot open");
	} else {
		//RFID_DEBUG("Device opened");
	}
	// RFID_DEBUGF("Device Number:%d\r\n",__spidev);

	/*
	 * spi mode
	 */
	ret = ioctl(__spidev, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(__spidev, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");
	//RFID_DEBUG("SPI mode [OK]");

	/*
	 * bits per word
	 */
	ret = ioctl(__spidev, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(__spidev, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");
	//RFID_DEBUG("SPI word bits[OK]");

	/*
	 * max speed hz
	 */
	ret = ioctl(__spidev, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(__spidev, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");
	//RFID_DEBUG("SPI max speed[OK]");

	spixfr.speed_hz = speed;
	spixfr.delay_usecs = 0;
	spixfr.bits_per_word = 8;
	spixfr.cs_change = 0;
	spixfr.pad = 0;

}

void MFRC522_HAL_write(unsigned char addr, unsigned char val) {
	int ret;
	char _dummytx[2];
	static char _devnull[2];
	_dummytx[0] = (addr << 1) & 0x7E;
	_dummytx[1] = val;

	spixfr.tx_buf = (unsigned long) _dummytx;
	spixfr.rx_buf = (unsigned long) _devnull;
	spixfr.len = 2;
	ret = ioctl(__spidev, SPI_IOC_MESSAGE(1), &spixfr);
	if (ret < 0) {
		//RFID_DEBUG("SPI transfer failed");
		exit(-1);
	} 
}


unsigned char MFRC522_HAL_read(unsigned char addr) {
	int ret;
	char _dummytx[2];
	char _rxbuf[2];
	_dummytx[0] = ((addr << 1) & 0xFE)|0x80;
	_dummytx[1] = 0xFF;

	spixfr.tx_buf = (unsigned long) _dummytx;
	spixfr.rx_buf = (unsigned long) _rxbuf;
	spixfr.len = 2;
	ret = ioctl(__spidev, SPI_IOC_MESSAGE(1), &spixfr);
	if (ret < 0) {
		//RFID_DEBUG("SPI transfer failed");
		exit(-1);
	}
	return _rxbuf[1];
}


void InitRc522(void);																
void ClearBitMask(uint8_t   reg,uint8_t   mask);							
void WriteRawRC(uint8_t   Address, uint8_t   value);					
void SetBitMask(uint8_t   reg,uint8_t   mask);							
char PcdComMF522(uint8_t   Command, uint8_t*pIn ,uint8_t InLenByte,uint8_t*pOut,uint8_t*pOutLenBit);	
void CalulateCRC(uint8_t 	*pIn ,uint8_t   len,uint8_t *pOut );		
uint8_t ReadRawRC(uint8_t   Address);													


char PcdReset(void);																						
char PcdRequest(unsigned char req_code,unsigned char *pTagType);//¹¦    ÄÜ£ºÑ°¿¨
void PcdAntennaOn(void);																				
void PcdAntennaOff(void);																				
char M500PcdConfigISOType(unsigned char type);									
char PcdAnticoll(unsigned char *pSnr);												
char PcdSelect(unsigned char *pSnr);														
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr);
char PcdWrite(unsigned char addr,unsigned char *pData);					
char PcdRead(unsigned char addr,unsigned char *pData);					
char PcdHalt(void);																							
void Reset_RC522(void);																					
uint8_t MFRC522_SelectTag(uint8_t *serNum); 

unsigned char SN[4]; //card number
void print_info(unsigned char *p,int cnt);
int read_card();
int card_passworld(uint8_t auth_mode,uint8_t addr,uint8_t *Src_Key,uint8_t *New_Key,uint8_t *pSnr);
uint8_t write_card_data(uint8_t *data);
uint8_t read_card_data();
void MFRC522_HAL_Delay(unsigned int ms);

void MFRC522_HAL_init(void);
void MFRC522_HAL_write(unsigned char addr, unsigned char val);
unsigned char MFRC522_HAL_read(unsigned char addr);


int main(int argc, char **argv)
{
    uint8_t data[16]={0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x20,0x21,0x22,0x23,255,255,255,255};
	uint8_t status=1;
    InitRc522();		 //RC522
	memset(data,0,16);
	printf("Reading...Please place the card...\r\n");
	// scanf("%s", data);
    while (1)
    {
        /* code */
        // status=write_card_data(data);
		// if(status==MI_OK)
		// {
		// 	break;
		// }
		// MFRC522_HAL_Delay(1000);
        status=read_card_data(data);
		if(status==MI_OK)
		{
			break;
		}
    }
    
}

void print_info(unsigned char *p,int cnt)
{
  int i;
	for(i=0;i<cnt;i++)
	{
		printf("%c",p[i]);
	}
	printf("\r\n");
}

 
int read_card() //read the card
{
	unsigned char CT[2];//card type
	uint8_t status=1;
	status=PcdRequest(PICC_REQIDL ,CT);//（card search mode, card type）
	
	if(status==MI_OK)//searching card successes
	{
	  status=MI_ERR;
	  status=PcdAnticoll(SN);  //Anti-collision, return 0 successfully; SN is the address of the read card.
		

		printf("Card type: ");
		if(CT[0] == 0x44)
		{
			printf("Mifare_UltraLight \r\n");
		}
		else if(CT[0]==0x4)
		{
			printf("MFOne_S50 \r\n");
		}
		else if(CT[0]==0X2)
		{
			printf("MFOne_S70 \r\n");
		}
		else if (CT[0]==0X8)
		{
			printf("Mifare_Pro(X)\r\n");
		}
		
		printf("Card ID: 0x");
	  	printf("%X%X%X%X\r\n",SN[0],SN[1],SN[2],SN[3]);
	}
	if(status==MI_OK)
	{
		status=MI_ERR;	
		status =PcdSelect(SN);	//Select the card.
	}
	  return status;
}
/*
Function: reset the password of the card.
Parameter Comment: auth_mode: Verify the type of code.
		addr：the address where the codes are stored（the password is saved at the third block of every section, and the first section is special.）
		Src_Key ：old password
		New_Key：new password
		pSnr : card number
*/
int card_passworld(uint8_t auth_mode,uint8_t addr,uint8_t *Src_Key,uint8_t *New_Key,uint8_t *pSnr)
{
		int status;
	 /*1.search card*/
    status=read_card(); 
	 /*2. verify the passwoord of the card*/
		if(status==MI_OK)
		{
			status=PcdAuthState(auth_mode,addr,Src_Key,pSnr);   
	  }
	 /*3. write data in the card*/
		if(status==MI_OK)
		{
		   status=PcdWrite(addr,New_Key); 
		}
		return status;
}
/*
Function: Write the data to the specific block
Parameter Comment: addr：the address where the codes are stored; the block 0.1.2 of every section are used to store the codes and 3 is used to store the password.
*/
uint8_t write_card_data(uint8_t *data)
{
	uint8_t KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff}; //Initial password--the factory password of the white card.
	
	int status=MI_ERR;
    status=read_card(); 
	   
	
		if(status==MI_OK)
		{
			 status=PcdAuthState(PICC_AUTHENT1A,3,KEY,SN);  
			
		}
		
	 
		if(status==MI_OK)
		{
		   status=PcdWrite(2,data); 
		}
		if(status==MI_OK)
		{
			printf("Write Data: ");
			print_info(data,16);
		}
		return status;
}
/*
Function: Read data to the designated block.
*/
uint8_t read_card_data()
{
	uint8_t KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff}; 
	int status;
	uint8_t data[16];
	
    status=read_card(); 
	
		if(status==MI_OK)
		{
			status=PcdAuthState(PICC_AUTHENT1A,3,KEY,SN);  
		}
	 
		if(status==MI_OK)
		{
		   status=PcdRead(2,data); 
		}
		if(status==MI_OK)
		{
				printf("Data: ");
				print_info(data,16);
		}
		return status;
}

 uint8_t MFRC522_SelectTag(uint8_t *serNum) 
{     
	uint8_t  i;     
	uint8_t  status;     
	uint8_t  size;     
	uint8_t recvBits;     
	uint8_t  buffer[9];
	//     
	buffer[0] = PICC_ANTICOLL1;	// anticollision code1     
	buffer[1] = 0x70;
	buffer[6] = 0x00;						     
	for (i=0; i<4; i++)					
	{
		buffer[i+2] = *(serNum+i);	//buffer[2]-buffer[5] is the serial number of the card
		buffer[6]  ^=	*(serNum+i);	// card-verified code
	}
	//
	CalulateCRC(buffer, 7, &buffer[7]);	//buffer[7]-buffer[8] is RCR-verified code
	ClearBitMask(Status2Reg,0x08);
	status =PcdComMF522(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
	//
	if ((status == MI_OK) && (recvBits == 0x18))    
		size = buffer[0];     
	else    
		size = 0;
	//	     
	return size; 
}

void delay_ns(uint32_t ns)
{
    uint32_t i;
    uint32_t j;
    for(i=0;i<ns;i++)
    {
        for(j=0;j<100;j++){}
    }
}


void WriteRawRC(uint8_t addr, uint8_t val) {
	MFRC522_HAL_write(addr, val);
}

uint8_t ReadRawRC(uint8_t addr) {
	return MFRC522_HAL_read(addr);
}

char PcdReset(void)  //Function: reset RC522
{
    
    WriteRawRC(CommandReg,PCD_RESETPHASE);  //Write RC632 register, reset 
	WriteRawRC(CommandReg,PCD_RESETPHASE);	//Write RC632 register, reset
    delay_ns(10); 
    
    WriteRawRC(ModeReg,0x3D);            //communicate with Mifare card, CRC initial value 0x6363   
    WriteRawRC(TReloadRegL,30);           //Write RC632 register
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
	
	WriteRawRC(TxAutoReg,0x40);
    
    return MI_OK;
}

void InitRc522(void) //initialize RC522
{
    MFRC522_HAL_init();
    PcdReset();  			// reset RC522
    PcdAntennaOff();	// Turn off the antenna 
    MFRC522_HAL_Delay(2);  		// Delay 2ms
    PcdAntennaOn();		//Turn on the antenna 
    M500PcdConfigISOType( 'A' ); // Set working mode 
}

void Reset_RC522(void) //Reset RC522
{
    PcdReset();				
    PcdAntennaOff();	
    MFRC522_HAL_Delay(2);  		 
    PcdAntennaOn();		 	
} 
/*
Function: search card 
 Parameter comment：   req_code :card search mode 
            [IN]:0x52 = all cards in the induction area that meet the 14443A standard
                 0x26 = find a card that has not entered sleeping mode
            pTagType[out] : card type code 
Return: Successfully returned MI_OK 
*/
char PcdRequest(uint8_t   req_code,uint8_t *pTagType)
{
	char status;  
	uint8_t   unLen;
	uint8_t   ucComMF522Buf[MAXRLEN];  	// MAXRLEN  18

	ClearBitMask(Status2Reg,0x08);	//Clear RC522 register bit,/ receive data command 
	WriteRawRC(BitFramingReg,0x07); //Write RC632 register 
	SetBitMask(TxControlReg,0x03);  //Set the RC522 register bit  
 
	ucComMF522Buf[0] = req_code; 	  
	
	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen); //Communicate with ISO14443 card via rc522. 
	
	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}
	else
	{   status = MI_ERR;   }  
	return status;
}    
/*
Function: anti-collision
Parameter Comment: psnr[out] card serial number, 4 bytes
Return: MI_OK is returned successfully
 */
char PcdAnticoll(uint8_t *pSnr)
{
    char status;
    uint8_t   i,snr_check=0;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN]; 
    
    ClearBitMask(Status2Reg,0x08);  
    WriteRawRC(BitFramingReg,0x00); 
    ClearBitMask(CollReg,0x80);    
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;   //PICC_ANTICOLL1 = 0x93
    ucComMF522Buf[1] = 0x20;
	
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen); 
											 
    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];  
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   status = MI_ERR;    }
    }   
    SetBitMask(CollReg,0x80);
    return status;
}
/*
Function: select card
Parameter Comment：psnr[out] card serial number, 4 bytes 
Return: MI_OK is returned successfully
*/
char PcdSelect(uint8_t *pSnr)
{
    char status;
    uint8_t   i;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
	
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
		
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]); //Calculate CRC16 function with MF522 and verify data  
    ClearBitMask(Status2Reg,0x08);
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}
/*
Function: verify the card password
Parameter Comment auth_mode[IN]: password verification mode  
            0x60 = Verify A key 
            0x61 = Verify B key  
        addr[IN]: block address 
        pKey[IN]: sector password 
        psnr[in]: card serial number 
Return: Successfully return MI_OK    
*/
char PcdAuthState(uint8_t   auth_mode,uint8_t   addr,uint8_t *pKey,uint8_t *pSnr)
{
    char   status;
    uint8_t   unLen;
   	int i=0;
    uint8_t   ucComMF522Buf[MAXRLEN];  //MAXRLEN 18
	  
    ucComMF522Buf[0] = auth_mode;		
    ucComMF522Buf[1] = addr;				
    memcpy(&ucComMF522Buf[2], pKey, 6); 
    memcpy(&ucComMF522Buf[8], pSnr, 4); 
	 
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}
/*
Function: read M1 card data
Parameter Comment: addr: block address
        P: read block data, 16 bytes
Return: MI_OK is returned successfully
*/
char PcdRead(uint8_t   addr,uint8_t *p )
{
    char status;
    uint8_t   unLen;
    uint8_t   i,ucComMF522Buf[MAXRLEN]; //18

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK && (unLen == 0x90)))//
    {
        for (i=0; i<16; i++)
        {    *(p +i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}
/*
Function: write data to M1 card     
Parameter Comment: addr：block address 
        p ：write data to block, 16 bytes  
Return: MI_OK is returned successfully  
*/
char PcdWrite(uint8_t   addr,uint8_t *p )
{
    char   status;
    uint8_t   unLen;
    uint8_t   i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        for (i=0; i<16; i++)//Write 16Byte data to FIFO    
        {    
        	ucComMF522Buf[i] = *(p +i);   
        }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}
/*
Function： command the card to enter the sleep mode  
Return：MI_OK is returned successfully 
*/
char PcdHalt(void)
{
    uint8_t   status;

	
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN]; //MAXRLEN==18
	   status=status;
    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}
/*
Function：Calculate CRC16 function with MF522  
Parameter Comment： pin：to read CRC data  
         len：  data length 
         pout： Calculate results of CRC   
*/
void CalulateCRC(uint8_t *pIn ,uint8_t   len,uint8_t *pOut )
{
    uint8_t   i,n;
    ClearBitMask(DivIrqReg,0x04);//CRCIrq = 0  
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);//clear FIFO
    	  
		for (i=0; i<len; i++)
    {  
			WriteRawRC(FIFODataReg, *(pIn +i));   }
		//begin to calculate 
			WriteRawRC(CommandReg, PCD_CALCCRC);
				//Wait until the calculation is complete  
			i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));//CRCIrq = 1
			
		pOut [0] = ReadRawRC(CRCResultRegL);
    pOut [1] = ReadRawRC(CRCResultRegM);
}
/*
set working mode of RC632   
*/
char M500PcdConfigISOType(uint8_t   type)
{
   if (type == 'A')                     //ISO14443_A
   { 
        ClearBitMask(Status2Reg,0x08);     
        WriteRawRC(ModeReg,0x3D);
        WriteRawRC(RxSelReg,0x86);
        WriteRawRC(RFCfgReg,0x7F);    
        WriteRawRC(TReloadRegL,30); 
        WriteRawRC(TReloadRegH,0);
        WriteRawRC(TModeReg,0x8D);
        WriteRawRC(TPrescalerReg,0x3E);
        delay_ns(1000);
        PcdAntennaOn();		
   }
   else{ return 1; }    

   return MI_OK;				
}
/*
Function：set RC522 register bit 
Parameter Comment：reg[IN]:register address 
          mask[IN]:set value 
*/
void SetBitMask(uint8_t   reg,uint8_t   mask)  
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);				
    WriteRawRC(reg,tmp | mask);   
}
/*
Function：clear RC522 register bit 
Parameter Comment: reg[IN]: register address 
          mask[IN]: clear bit value 
*/
void ClearBitMask(uint8_t   reg,uint8_t   mask)  
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);   
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 
/*
Function：communicate with ISO14443 via RC522 
Parameter Comment: command[IN]:RC522 command word 
        pin[IN]: Data sent to the card via RC522   
        InLenByte[IN]:The byte length of the data sent 
        pOut[OUT]: The received card returns data  
        pOutLenBit[OUT]：Returns the bit length of the data  
*/
char PcdComMF522(uint8_t Command,uint8_t *pIn,uint8_t InLenByte,uint8_t *pOut,uint8_t *pOutLenBit)
{
    char status = MI_ERR;
    uint8_t   irqEn   = 0x00;
    uint8_t   waitFor = 0x00;
    uint8_t   lastBits;
    uint8_t   n;
    uint16_t  i;
	
    switch (Command)
    {
		case PCD_AUTHENT:    //verify password
				 irqEn   = 0x12;
				 waitFor = 0x10;
				 break;
		case PCD_TRANSCEIVE: // Send and receive data. 
				 irqEn   = 0x77;
				 waitFor = 0x30;
				 break;
		default:
				 break;
    }
    WriteRawRC(ComIEnReg,irqEn|0x80);	
    ClearBitMask(ComIrqReg,0x80);			//Clear all interrupt bits 
    WriteRawRC(CommandReg,PCD_IDLE);	
    SetBitMask(FIFOLevelReg,0x80);	 	//clear FIFO buffer memory  
    

		
    for (i=0; i<InLenByte; i++)
    {   
				WriteRawRC(FIFODataReg, pIn [i]);  }
				WriteRawRC(CommandReg, Command);	 
			if (Command == PCD_TRANSCEIVE)
				{  
					SetBitMask(BitFramingReg,0x80);  }	 //Start transmission 
    
		
		
  //  i = 600;// According to clock frequency to adjustment, operation M1 card has maximum waiting time 25ms 
				i = 2000;
			do 
				{
					n = ReadRawRC(ComIrqReg);
					i--;
				}
			while ((i!=0) && !(n&0x01) && !(n&waitFor));
				
		
			ClearBitMask(BitFramingReg,0x80);
			if (i!=0)
			{    
        if(!(ReadRawRC(ErrorReg)&0x1B))
        {
            status = MI_OK;
            // printf("test_n:%d",n);
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            if (Command == PCD_TRANSCEIVE)
            {
               	n = ReadRawRC(FIFOLevelReg);
              	lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOut [i] = ReadRawRC(FIFODataReg);    }
            }
        }
				
        else
        {   status = MI_ERR;   }
        
    }
    SetBitMask(ControlReg,0x80);           // stop timer now
    WriteRawRC(CommandReg,PCD_IDLE); 
    return status;
}
/*
Turn on the antenna 
There should be at least 1ms interval between each antenna start or close  
*/
void PcdAntennaOn(void)
{
    uint8_t   i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}
/*
Turn off the antenna 
*/
void PcdAntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03); 
}
