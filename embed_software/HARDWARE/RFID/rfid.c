#include "includes.h"					//ucos 使用	  
#include "main.h"
#include "finsh.h"
#include "debug.h"
#include "rfid.h"
#include "rc522.h"
#include "usart.h"
#include "gpio.h"
#include "bluetooth.h"

/*******************************************************************************
宏定义区
*******************************************************************************/
#define  READCARD   0xA1
#define  WRITECARD  0xA2
#define  KEYCARD    0xA3
#define  SETTIME    0xA4
#define  SENDID     0xA5

#define	MAX_RFID_CNT	2	//最大的RFID个数

/*******************************************************************************
变量定义区
*******************************************************************************/
u16	KuaiN				=  0;
u8	SysTime				=  0;
u8	oprationcard	 	=  0;
u8	bSendID				=  0;
u8	Read_Data[16]		= {0};
u8	PassWd[6]			= {0};
u8	WriteData[16]		= {0};
u8	RevBuffer[30]		= {0};
u8	MLastSelectedSnr[4]	= {0};
u8	NewKey[16]			= {0};
u8  port_id				=  0;
u8  port_data[40]       = {0};
char cardID[4]			= {0};

/*******************************************************************************
变量声明区
*******************************************************************************/
extern OS_FLAG_GRP *TaskCheckOSFlag;

/*******************************************************************************
函数声明区
*******************************************************************************/
extern void ClearBitMask(unsigned char  reg,unsigned char  mask);
extern void WriteRawRC(unsigned char Address, unsigned char  value);
extern void SetBitMask(unsigned char  reg,unsigned char mask);
extern char PcdComMF522(unsigned char Command, unsigned char *pInData, unsigned char  InLenByte,unsigned char *pOutData, unsigned int  *pOutLenBit);
extern void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);
extern unsigned char ReadRawRC(unsigned char Address);
extern void PcdAntennaOn(void);

/*******************************************************************************
* 描  述  :  初始化RC522，开启天线并设置RC522的工作方式为 ISO14443_A
* 输  入  :  无
* 输  出  :  无
* 返  回  :  无
*******************************************************************************/
static void InitRc522(void)
{
  PcdReset();
  PcdAntennaOff();  
  PcdAntennaOn();
  M500PcdConfigISOType( 'A' );
}

 /*******************************************************************************
* 描  述  :  对寻卡、防冲撞、选卡、发送卡号、读卡、写卡、修改密码进行操作
             成功则LED1灯亮
* 输  入  :  无
* 输  出  :  无
* 返  回  :  无
*******************************************************************************/
void ctrlprocess(void)
{
	unsigned char ii;
	char status;

	PcdReset();
	status=PcdRequest(PICC_REQIDL,&RevBuffer[0]);	//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节
	if(status!=MI_OK)
	{
		return;
	}

	status=PcdAnticoll(&RevBuffer[2]);				//防冲撞，返回卡的序列号 4字节
	if(status!=MI_OK)
		return;
	memcpy(MLastSelectedSnr,&RevBuffer[2],4); 
	status=PcdSelect(MLastSelectedSnr);				//选卡
	if(status!=MI_OK)
	{
		InfoPrintf("%s -- %d exit\r\n",__FUNCTION__,__LINE__);
		return;
	}
	if(oprationcard==KEYCARD)						//修改密码
	{
		oprationcard=0;    
		status=PcdAuthState(PICC_AUTHENT1A,KuaiN,PassWd,MLastSelectedSnr);
		if(status!=MI_OK)
		{
			InfoPrintf("%s -- %d exit\r\n",__FUNCTION__,__LINE__);
			return;
		}
		status=PcdWrite(KuaiN,&NewKey[0]);
		if(status!=MI_OK)
		{
			InfoPrintf("%s -- %d exit\r\n",__FUNCTION__,__LINE__);
			return;
		}
		PcdHalt();
		InfoPrintf("modify password success\r\n");
	}
	else if(oprationcard==READCARD)					//读卡
	{
		oprationcard=0;
		status=PcdAuthState(PICC_AUTHENT1A,KuaiN,PassWd,MLastSelectedSnr);	 //验证A密匙
		if(status!=MI_OK)
		{
			InfoPrintf("%s -- %d exit\r\n",__FUNCTION__,__LINE__);
			return;
		}

		status=PcdRead(KuaiN,Read_Data);
		if(status!=MI_OK)
		{
			InfoPrintf("%s -- %d exit\r\n",__FUNCTION__,__LINE__);
			return;
		}

		DebugPrintf(OTHER_DBG,"read card key: ");
		debug_printf(OTHER_DBG,(unsigned char*)Read_Data,16);
		PcdHalt();
		port_data[port_id*5 + 0] = port_id + 1 +'0';
		port_data[port_id*5 + 1] = (Read_Data[0] / 10) % 10 + '0' ;
		port_data[port_id*5 + 2] = (Read_Data[0] % 10) + '0';
		port_data[port_id*5 + 3] = 0x0d;
		port_data[port_id*5 + 4] = 0x0a;
	}
	else if(oprationcard==WRITECARD)				//写卡
	{
		oprationcard=0;
		status=PcdAuthState(PICC_AUTHENT1A,KuaiN,PassWd,MLastSelectedSnr);//
		if(status!=MI_OK)
		{
			InfoPrintf("%s -- %d exit\r\n",__FUNCTION__,__LINE__);
			return;
		}
		status=PcdWrite(KuaiN,&WriteData[0]);
		if(status!=MI_OK)
		{
			InfoPrintf("%s -- %d exit\r\n",__FUNCTION__,__LINE__);
			return;
		}
		PcdHalt();
		InfoPrintf("write card success\r\n");
	} 
	else if(oprationcard==SENDID)					//发送卡号
	{
		oprationcard=0;
		for(ii=0;ii<4;ii++)
			cardID[ii] = MLastSelectedSnr[ii];
		InfoPrintf("send car id: ");
		debug_printf(NORM_DBG,(unsigned char*)cardID,4);
	}  
}
 /*******************************************************************************
 * 描  述  : 将ASC字符转换成hex数字
 * 输  入  :  无
 * 输  出  :  无
 * 返  回  :  无
 *******************************************************************************/
static u8 asc_to_hex(u8 data)
{
	if( data >= '0' && data <= '9' )
		return data - '0';
	else if( data >= 'a' && data <= 'z' )
		return data - 'a' + 10;
	else if( data >= 'A' && data <= 'Z' )
		return data - 'A' + 10;
	return 0;
}
/*******************************************************************************
* 描  述  : 判断操作类型（发送卡号、读卡、写卡、修改密码 ）
* 输  入  :  无
* 输  出  :  无
* 返  回  :  无
*******************************************************************************/
void rfid_ctrl(u8* p_data)
{
	u8 ii;
	for( ii = 0 ; ii < 30 ; ii++ )
		RevBuffer[ii] = (asc_to_hex(p_data[ii*2+0])<<4) + asc_to_hex(p_data[ii*2+1]);
	DebugPrintf(OTHER_DBG,"found rfid input: ");
	debug_printf(OTHER_DBG,RevBuffer,30);
  
	switch(RevBuffer[1])
	{
	case 0xa0:	//发送卡号
		oprationcard=SENDID;
		break;
	case 0xa1://读数据
		oprationcard=READCARD;
		for(ii=0;ii<6;ii++)
			PassWd[ii] = RevBuffer[ii+2];
		KuaiN=RevBuffer[8];
		break;
	case 0xa2://写数据
		oprationcard=WRITECARD;
		for(ii=0;ii<6;ii++)
			PassWd[ii]=RevBuffer[ii+2];
		KuaiN=RevBuffer[8];
		for(ii=0;ii<16;ii++)
			WriteData[ii]=RevBuffer[ii+9];
		break;  
	case 0xa3: //修改密码
		oprationcard=KEYCARD; 
		for(ii=0;ii<6;ii++)
			PassWd[ii]=RevBuffer[ii+2];
		KuaiN=RevBuffer[8];
		for(ii=0;ii<6;ii++)
		{
			NewKey[ii]=RevBuffer[ii+9];
			NewKey[ii+10]=RevBuffer[ii+9];
		}
		break;
	default:
		break;
	}
}
FINSH_FUNCTION_EXPORT(rfid_ctrl,ctrl rfid)

/*******************************************************************************
 rc522主程序
*******************************************************************************/
static void RC522(void)
{
#if 1
	oprationcard=SENDID;
	port_id = 0;
	rfid_ctrl("19A2FFFFFFFFFFFF04280102030405060708090A0B0C0D0E0F");
	ctrlprocess();
	return;
#endif
	for( port_id = 0 ; port_id < MAX_RFID_CNT ; port_id++ )
	{
		port_data[port_id*5 + 0] = port_id + 1 +'0';
		port_data[port_id*5 + 1] = '0';
		port_data[port_id*5 + 2] = '0';
		port_data[port_id*5 + 3] = 0x0d;
		port_data[port_id*5 + 4] = 0x0a;

		rfid_ctrl("09A1FFFFFFFFFFFF04");
		ctrlprocess();
	}
	rfid_send_cmd_to_bluetooth_task(RFID_SEND_CARD_MSG,strlen((const char*)port_data),port_data);
}
/****************************************************************************************
 rfid处理程序
****************************************************************************************/
void rfid_proc_task(void *pdata)
{
	u8  err;
    
	pdata = pdata;
	rfid_gpio_init();
	for( port_id = 0 ; port_id < MAX_RFID_CNT ; port_id++ )
		InitRc522();
	while(1)
    {
    	RC522();
        OSTimeDly(5);
		OSFlagPost(TaskCheckOSFlag,OSFLAG_RFID_PROC_TASK,OS_FLAG_SET,&err);
    }
}

