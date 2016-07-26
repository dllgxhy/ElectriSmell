#include"rc522.h"
#include "stm32f10x_gpio.h"
#include"rc522_config.h"      //����RC522������
void ClearBitMask(unsigned char  reg,unsigned char  mask);
void WriteRawRC(unsigned char Address, unsigned char  value);
void SetBitMask(unsigned char  reg,unsigned char mask);
int PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char  InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit);
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);
unsigned char ReadRawRC(unsigned char Address);
void PcdAntennaOn(void);
extern void delay_us(u32 Nus);	
extern u16  KuaiN;
extern u8 SysTime, oprationcard,bSendID;				
extern u8  Read_Data[16],PassWd[6],WriteData[16],RevBuffer[30], MLastSelectedSnr[4],NewKey[16];	
extern u8  port_id;
/////////////////////////////////////////////////////////////////////
//��    �ܣ�SPIд����
//��    �룺 ��
// �޷���ֵ
///////////////////////////////////////////////////////////////////// 
void Write_SPI(unsigned char num)    
{  
	u8 count=0,i;     
	for(count=0;count<8;count++)  
	{
		if(num&0x80)
			TDIN_SET(1);  
		else
			TDIN_SET(0);   
		num<<=1;	
		TCLK_SET(0);//��������Ч 
		for(i=0;i<15;i++);	 
		TCLK_SET(1);
		for(i=0;i<15;i++);	 
	} 
} 
 /////////////////////////////////////////////////////////////////////
//��    �ܣ�SPI������
//��    �룺 ��
//��    ��:	 ��
///////////////////////////////////////////////////////////////////// 
unsigned char Read_SPI(void)	  
{ 	 
	unsigned char SPICount,i;                                       // Counter used to clock out the data

	unsigned char  SPIData;                  
	SPIData = 0;                    //�½�����Ч 
	for (SPICount = 0; SPICount < 8; SPICount++)				  // Prepare to clock in the data to be read
	{
		SPIData <<=1;												// Rotate the data
		   
		TCLK_SET(0);		
		for(i=0;i<15;i++);										 // Raise the clock to clock the data out of the MAX7456
		if(DOUT)
			SPIData|=0x01;
		TCLK_SET(1);	
		for(i=0;i<15;i++);								// Drop the clock ready for the next bit
	}													// and loop back
	return (SPIData);  
	   
}                  
/////////////////////////////////////////////////////////////////////
//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
int PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
	int status;  
	unsigned int unLen;
	unsigned char  ucComMF522Buf[MAXRLEN]; 
	ClearBitMask(Status2Reg,0x08);
	WriteRawRC(BitFramingReg,0x07);
	SetBitMask(TxControlReg,0x03);
 
	ucComMF522Buf[0] = req_code;  //Ѱ����Ϊ��������״̬�Ŀ�

	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}
	else
	{   status = MI_ERR;   }
   
	return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////  
int PcdAnticoll(unsigned char *pSnr)
{
    int  status;
    unsigned char  i,snr_check=0;
    unsigned int  unLen;
    unsigned char  ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);	 //��0������֤����ɹ�ʱ��λ��1
    WriteRawRC(BitFramingReg,0x00);	 //ȡ�����ͺͽ���
    ClearBitMask(CollReg,0x80);	   //������н��ܵ�����Ϣ
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;	 //����ײ
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

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
int PcdSelect(unsigned char *pSnr)
{
    int   status;
    unsigned char  i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���֤��Ƭ����
//����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ 
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////               
int PcdAuthState(unsigned char  auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    int  status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;  //���ڼ���
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   } //�������
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
       status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          pData[OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
///////////////////////////////////////////////////////////////////// 
int  PcdRead(unsigned char  addr,unsigned char *pData)
{
    int status;
    unsigned int unLen;
    unsigned char  i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ; //����
    ucComMF522Buf[1] = addr;   //���ַ
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]); 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          pData[IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////                  
int PcdWrite(unsigned char  addr,unsigned char *pData)
{
    int status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        for (i=0; i<16; i++)
        {    
        	ucComMF522Buf[i] = *(pData+i);   
        }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
int PcdHalt(void)
{
    unsigned int unLen;
    unsigned char  ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;	 //����
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
    PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//��MF522����CRC16����
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata,unsigned char  len,unsigned char *pOutData)
{
    unsigned char  i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   WriteRawRC(FIFODataReg, *(pIndata+i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);  //��CRC�Ƿ���Ч	����2λ
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);	//��У��ֵ
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���λRC522
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
	switch( port_id )
	{
	case 0:
		SET_RC522RST;
		delay_us(100);
		CLR_RC522RST;
		delay_us(100);
		SET_RC522RST;
		delay_us(100);
		break;
	case 1:
		SET1_RC522RST;
		delay_us(100);
		CLR1_RC522RST;
		delay_us(100);
		SET1_RC522RST;
		delay_us(100);
		break;
	}
    WriteRawRC(CommandReg,PCD_RESETPHASE);	//��λRC522
    delay_us(100);  
    WriteRawRC(ModeReg,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL,30);           
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);	
	WriteRawRC(TxAutoReg,0x40);//����Ҫ   
    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//����RC522�Ĺ�����ʽ 
//////////////////////////////////////////////////////////////////////
int M500PcdConfigISOType(unsigned char type)
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
	   delay_us(10000);
       PcdAntennaOn();
   }
   else{ return -1; }
   
   return MI_OK;
}
/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
	unsigned char  ucAddr;
	unsigned char ucResult=0;

	switch( port_id )
	{
	case 0:
		TCS_SET(0);
		ucAddr = ((Address<<1)&0x7E)|0x80;	// //���Ĵ�����ʱ�򣬵�ַ���λΪ 1�����λΪ0��1-6λȡ���ڵ�ַ
		Write_SPI(ucAddr);
		ucResult=Read_SPI();
		TCS_SET(1);
		break;
	case 1:
		TCS1_SET(0);
		ucAddr = ((Address<<1)&0x7E)|0x80;	// //���Ĵ�����ʱ�򣬵�ַ���λΪ 1�����λΪ0��1-6λȡ���ڵ�ַ
		Write_SPI(ucAddr);
		ucResult=Read_SPI();
		TCS1_SET(1);
		break;
	}
	return ucResult;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�дRC522�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char  Address, unsigned char  value)
{  
	unsigned char ucAddr;

	switch( port_id )
	{
	case 0:
		TCS_SET(0);
		ucAddr = ((Address<<1)&0x7E);	 //д�Ĵ�����ʱ�򣬵�ַ���λΪ 0�����λΪ0��1-6λȡ���ڵ�ַ
		Write_SPI(ucAddr);
		Write_SPI(value);
		TCS_SET(1);
		break;
	case 1:
		TCS1_SET(0);
		ucAddr = ((Address<<1)&0x7E);	 //д�Ĵ�����ʱ�򣬵�ַ���λΪ 0�����λΪ0��1-6λȡ���ڵ�ַ
		Write_SPI(ucAddr);
		Write_SPI(value);
		TCS1_SET(1);
		break;
	}
}
/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char  reg,unsigned char mask)  
{
    char  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char  reg,unsigned char  mask)  
{
    char  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pInData[IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOutData[OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
/////////////////////////////////////////////////////////////////////
int PcdComMF522(unsigned char  Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int *pOutLenBit)
{
    int   status = MI_ERR;
    unsigned char  irqEn   = 0x00;
    unsigned char  waitFor = 0x00;
    unsigned char  lastBits;
    unsigned char  n;
    unsigned int i;
    switch (Command)
    {
        case PCD_AUTHENT:
			irqEn   = 0x12;
			waitFor = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irqEn   = 0x77;
			waitFor = 0x30;
			break;
		default:
			break;
    }
   
    WriteRawRC(ComIEnReg,irqEn|0x80);  //ʹ�ܽ��ܺͷ����ж�����
    ClearBitMask(ComIrqReg,0x80);     //��ComIrqRegΪ0xff,		
    WriteRawRC(CommandReg,PCD_IDLE); //ȡ����ǰ����
    SetBitMask(FIFOLevelReg,0x80);	
    
    for (i=0; i<InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pInData[i]);    }
    WriteRawRC(CommandReg, Command);
    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg,0x80);  }	  //��ʼ����
    
	i = 700;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
    do 
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);  //���ͽ���

    if (i!=0)
    {    
        if(!(ReadRawRC(ErrorReg)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            if (Command == PCD_TRANSCEIVE)
            {
               	n = ReadRawRC(FIFOLevelReg); 
              	lastBits = ReadRawRC(ControlReg) & 0x07;   //�ó������ֽ��е���Чλ�����Ϊ0��ȫ��λ����Ч
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = ReadRawRC(FIFODataReg);    }
            }
        }
        else
        {   status = MI_ERR;   }
        
    }
    SetBitMask(ControlReg,0x80);           // stop timer now
    WriteRawRC(CommandReg,PCD_IDLE); 
    return status;
}

/////////////////////////////////////////////////////////////////////
//��������  
//ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn(void)
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/////////////////////////////////////////////////////////////////////
//�ر�����
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}

