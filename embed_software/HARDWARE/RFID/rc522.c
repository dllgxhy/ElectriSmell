#include"rc522.h"
#include "stm32f10x_gpio.h"
#include"rc522_config.h"      //定义RC522的引脚
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
//功    能：SPI写数据
//输    入： 无
// 无返回值
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
		TCLK_SET(0);//上升沿有效 
		for(i=0;i<15;i++);	 
		TCLK_SET(1);
		for(i=0;i<15;i++);	 
	} 
} 
 /////////////////////////////////////////////////////////////////////
//功    能：SPI读数据
//输    入： 无
//返    回:	 无
///////////////////////////////////////////////////////////////////// 
unsigned char Read_SPI(void)	  
{ 	 
	unsigned char SPICount,i;                                       // Counter used to clock out the data

	unsigned char  SPIData;                  
	SPIData = 0;                    //下降沿有效 
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
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
int PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
	int status;  
	unsigned int unLen;
	unsigned char  ucComMF522Buf[MAXRLEN]; 
	ClearBitMask(Status2Reg,0x08);
	WriteRawRC(BitFramingReg,0x07);
	SetBitMask(TxControlReg,0x03);
 
	ucComMF522Buf[0] = req_code;  //寻天线为进入休眠状态的卡

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
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
int PcdAnticoll(unsigned char *pSnr)
{
    int  status;
    unsigned char  i,snr_check=0;
    unsigned int  unLen;
    unsigned char  ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);	 //清0，当验证密码成功时此位置1
    WriteRawRC(BitFramingReg,0x00);	 //取消发送和接受
    ClearBitMask(CollReg,0x80);	   //清除所有接受到的信息
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;	 //防冲撞
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
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
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
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////               
int PcdAuthState(unsigned char  auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    int  status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;  //读第几块
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   } //存放密码
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
       status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
///////////////////////////////////////////////////////////////////// 
int  PcdRead(unsigned char  addr,unsigned char *pData)
{
    int status;
    unsigned int unLen;
    unsigned char  i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ; //读块
    ucComMF522Buf[1] = addr;   //块地址
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
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
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
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
int PcdHalt(void)
{
    unsigned int unLen;
    unsigned char  ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;	 //休眠
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
    PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
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
        n = ReadRawRC(DivIrqReg);  //看CRC是否有效	，第2位
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);	//读校验值
}

/////////////////////////////////////////////////////////////////////
//功    能：复位RC522
//返    回: 成功返回MI_OK
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
    WriteRawRC(CommandReg,PCD_RESETPHASE);	//软复位RC522
    delay_us(100);  
    WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL,30);           
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);	
	WriteRawRC(TxAutoReg,0x40);//必须要   
    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//设置RC522的工作方式 
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
//功    能：读RC522寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
	unsigned char  ucAddr;
	unsigned char ucResult=0;

	switch( port_id )
	{
	case 0:
		TCS_SET(0);
		ucAddr = ((Address<<1)&0x7E)|0x80;	// //读寄存器的时候，地址最高位为 1，最低位为0，1-6位取决于地址
		Write_SPI(ucAddr);
		ucResult=Read_SPI();
		TCS_SET(1);
		break;
	case 1:
		TCS1_SET(0);
		ucAddr = ((Address<<1)&0x7E)|0x80;	// //读寄存器的时候，地址最高位为 1，最低位为0，1-6位取决于地址
		Write_SPI(ucAddr);
		ucResult=Read_SPI();
		TCS1_SET(1);
		break;
	}
	return ucResult;
}

/////////////////////////////////////////////////////////////////////
//功    能：写RC522寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char  Address, unsigned char  value)
{  
	unsigned char ucAddr;

	switch( port_id )
	{
	case 0:
		TCS_SET(0);
		ucAddr = ((Address<<1)&0x7E);	 //写寄存器的时候，地址最高位为 0，最低位为0，1-6位取决于地址
		Write_SPI(ucAddr);
		Write_SPI(value);
		TCS_SET(1);
		break;
	case 1:
		TCS1_SET(0);
		ucAddr = ((Address<<1)&0x7E);	 //写寄存器的时候，地址最高位为 0，最低位为0，1-6位取决于地址
		Write_SPI(ucAddr);
		Write_SPI(value);
		TCS1_SET(1);
		break;
	}
}
/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char  reg,unsigned char mask)  
{
    char  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char  reg,unsigned char  mask)  
{
    char  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
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
   
    WriteRawRC(ComIEnReg,irqEn|0x80);  //使能接受和发送中断请求
    ClearBitMask(ComIrqReg,0x80);     //置ComIrqReg为0xff,		
    WriteRawRC(CommandReg,PCD_IDLE); //取消当前命令
    SetBitMask(FIFOLevelReg,0x80);	
    
    for (i=0; i<InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pInData[i]);    }
    WriteRawRC(CommandReg, Command);
    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg,0x80);  }	  //开始发送
    
	i = 700;//根据时钟频率调整，操作M1卡最大等待时间25ms
    do 
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);  //发送结束

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
              	lastBits = ReadRawRC(ControlReg) & 0x07;   //得出接受字节中的有效位，如果为0，全部位都有效
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
//开启天线  
//每次启动或关闭天险发射之间应至少有1ms的间隔
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
//关闭天线
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}

