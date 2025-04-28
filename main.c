/*------------HeadFile-----------------*/
#include <STC15F2K60S2.H>
#include "IIC.h"
#include "onewire.h"

/*------------Define&sbit-------------*/
sbit ROW1=P3^0;
sbit ROW2=P3^1;
sbit ROW3=P3^2;
sbit ROW4=P3^3;
//矩阵键盘的行

sbit COL1=P4^4;
sbit COL2=P4^2;
sbit COL3=P3^5;
sbit COL4=P3^4;
//矩阵键盘的列

sbit US_TX=P1^0;
sbit US_RX=P1^1;
//超声波

code unsigned char Seg_Table[]=
{0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,
 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E, 0xBF, 0x7f, 0xFF};

 code unsigned char Seg_Table_Dot[]=
{0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};

/*------------变量---------------------*/
unsigned char Key_Val,Key_Down,Key_Old;
unsigned char Key_Slow_Down=0; 
unsigned int Key_count=0;
//键盘相关

unsigned char ain_value=0;
unsigned char light_level=0;
//光敏相关

float DS18B20_Temp;
unsigned int DS18B20_Temp_Seg=0;
unsigned char DS18B20_Temp_Con=0;//转化过的
unsigned char DS18B20_Temp_P=30;
//温度相关

unsigned char mode1=1;
unsigned char mode2=1;
//界面切换相关

unsigned char Dis_P=30;
float Dis=0;
unsigned char Dis_smg=0;
unsigned char count_t=0;
unsigned char count_s=0;
unsigned char Dis_stat=1;
//超声波&距离相关

unsigned int Layer_times=0;//继电器
unsigned char Layer_stat=0x00;
unsigned char LED_stat=0xff;//LED状态
//输出控制相关


/*------------HC573&Delay--------------------*/
void Set_HC573(unsigned char n,dat)
{
	P0=dat;
	switch(n)
	{
		case 4:
			P2=(P2&0x1f)|0x80;
		break;
		
		case 5:
			P2=(P2&0x1f)|0xa0;
		break;
		
		case 6:
			P2=(P2&0x1f)|0xc0;
		break;
		
		case 7:
			P2=(P2&0x1f)|0xe0;
		break;
	}
		P2=P2&0x1f;
}
//

void Delay(unsigned int xms)	//@12.000MHz
{
	unsigned char data i, j;
		while(xms)
		{
		i = 12;
		j = 169;
		do
		{
			while (--j);
		} while (--i);
		xms--;
	}
}
//

/*------------Key----------------------*/
unsigned char Get_KeyNum()
{
	unsigned char temp=0;
	ROW4=0;
	ROW2=ROW3=ROW1=1;
	if(COL1==0)
		temp=4;
	if(COL2==0)
		temp=8;
	
	ROW3=0;
	ROW2=ROW4=ROW1=1;
	if(COL1==0)
		temp=5;
	if(COL2==0)
		temp=9;
	
//	COL2=0;
//	COL4=COL3=COL1=1;
//	if(ROW3==0)
//	{
//		if(ROW4==0)
//		temp=10;
//	}	
//	if(ROW4==0)
//	{
//		if(ROW3==0)
//		temp=10;
//	}
	
	return temp;
}
//

void Key_Proc()
{
	if(Key_Slow_Down) return;
	Key_Slow_Down=1;
	
	Key_Val=Get_KeyNum();
	Key_Down=Key_Val&(Key_Val^Key_Old);
	Key_Old=Key_Val;
	
	if(Key_Down==4)
	{
		if(mode1!=4)
			mode1++;
		else
			mode1=1;
		mode2=1;
	}
	
	if(Key_Down==5)
	{
		if(mode1==3)
		{
			if(mode2==1)
				mode2=2;
			else
				mode2=1;
		}
	}
	
	if(Key_Down==8)
	{
		if(mode1==3)
		{
			if(mode2==1)
			{
				if(DS18B20_Temp_P<80)
					DS18B20_Temp_P++;
			}
			if(mode2==2)
			{
				if(Dis_P<75)
					Dis_P+=5;
			}
		}
	}
	
	if(Key_Down==9)
	{
		if(mode1==3)
		{
			if(mode2==1)
			{
				if(DS18B20_Temp_P>20)
					DS18B20_Temp_P--;
			}
			if(mode2==2)
			{
				if(Dis_P>25)
				Dis_P-=5;
			}
		}
	}
	
//	COL2=0;
//	COL4=COL3=COL1=1;
//	if(ROW3==0&&ROW4==0)
//	{
//		Delay(20);
//		if(ROW3==0&&ROW4==0)
//		{
//			Key_count=0;
//			while(ROW3==0&&ROW4==0);
//			Layer_times=0;
//		}
//	}
	
//	if(Key_Down==10)
//	{
//		Key_count=0;
//		while(Key_Down==10);
//		if(Key_count>2000)
//		{
//			if(mode1==4)
//			{
//				Layer_times=0;
//			}
//		}
//	}
}
//

/*------------Seg----------------------*/
void Show_Seg_Bit(unsigned char pos,dat)
{
	Set_HC573(6,0x01<<pos);
	Set_HC573(7,dat);
	Delay(1);
	Set_HC573(6,0x01<<pos);
	Set_HC573(7,0xff);
}
//

void Close_All_Seg()
{
	unsigned char i;
	for(i=0;i<8;i++)
		Show_Seg_Bit(i,0xff);
}
//

void Display_Seg()
{
	switch (mode1)
	{
		case 1:
			Show_Seg_Bit(0,Seg_Table[12]);
			Show_Seg_Bit(1,Seg_Table[(DS18B20_Temp_Seg/1000)%10]);
			Show_Seg_Bit(2,Seg_Table[(DS18B20_Temp_Seg/100)%10]);
			
			Show_Seg_Bit(6,0xc8);
			Show_Seg_Bit(7,Seg_Table[light_level]);
		break;
		//环境
		
		case 2:
			Show_Seg_Bit(0,0xc7);
			Show_Seg_Bit(1,Seg_Table[Dis_stat]);
		
			if(Dis_smg>1000)
				Show_Seg_Bit(5,Seg_Table[Dis_smg/1000]);
			Show_Seg_Bit(6,Seg_Table[(Dis_smg/100)%10]);
			Show_Seg_Bit(7,Seg_Table[(Dis_smg/10)%10]);
		break;
		//运动
		
		case 3:
			switch (mode2)
			{
				case 1:
					Show_Seg_Bit(0,0x8c);
					Show_Seg_Bit(1,Seg_Table[12]);
					
					Show_Seg_Bit(6,Seg_Table[DS18B20_Temp_P/10]);
					Show_Seg_Bit(7,Seg_Table[DS18B20_Temp_P%10]);
				break;
				//温度参数
				
				case 2:
					Show_Seg_Bit(0,0x8c);
					Show_Seg_Bit(1,0xc7);
					
					Show_Seg_Bit(6,Seg_Table[Dis_P/10]);
					Show_Seg_Bit(7,Seg_Table[Dis_P%10]);
				break;
				//距离参数
			}
		break;
		//参数
		
		case 4:
			Show_Seg_Bit(0,0xc8);
			Show_Seg_Bit(1,Seg_Table[12]);
		
			if(Layer_times>1000)
				Show_Seg_Bit(4,Seg_Table[Layer_times/1000]);
			if(Layer_times>100)
				Show_Seg_Bit(5,Seg_Table[(Layer_times/100)%10]);
			if(Layer_times>10)
				Show_Seg_Bit(6,Seg_Table[(Layer_times/10)%10]);
			
			Show_Seg_Bit(7,Seg_Table[Layer_times%10]);

		break;
		//统计
	}

}
//

/*------------Timer&interrupt----------*/
void Timer0_Isr(void) interrupt 1
{
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	if(++Key_Slow_Down==5)
		Key_Slow_Down=0;
//	Key_count++;
//	if(Key_count>20000)
//		Key_count=0;
}
//

void Timer_Init(void)		//Timer0 1ms@12.000MHz Timer1 50ms@12.000MHz
{
	TMOD = 0x11;			//设置定时器模式
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
	EA=1;
	
	TL1 = 0xB0;				//设置定时初始值
	TH1 = 0x3C;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
}
//

void Timer1_Isr(void) interrupt 3
{
	TL1 = 0xB0;				//设置定时初始值
	TH1 = 0x3C;				//设置定时初始值
	count_t++;
	if(count_t==20)
	{
		count_t=0;
		count_s++;
	}
}
//

/*------------PCF8591------------------*/
void Set_PCF8591_Ain1()
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	
	I2CSendByte(0x01);
	I2CWaitAck();
	I2CStop();
}
//

unsigned char Read_PCF8591()
{
	unsigned char temp=0;
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	
	temp=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	return temp;
}
//

void PCF8591_Proc()
{
	ain_value=Read_PCF8591();

	if(ain_value>=153)
		light_level=1;
	else if(ain_value>=102&&ain_value<153)
		light_level=2;
	else if(ain_value>=25&&ain_value<102)
		light_level=3;
	else if(ain_value<25)
		light_level=4;
}
//

/*------------DS18B20--------------------*/
void Begin_DS18B20()
{
	unsigned char i=0;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	for(i=0;i<8;i++)
	{
		Delay(100);
	}
}

void Read_DS18B20_Temp()
{
	unsigned char MSB,LSB;
	unsigned int dat;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	Display_Seg();
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	Display_Seg();
	
	LSB=Read_DS18B20();
	MSB=Read_DS18B20();
	dat=MSB;
	dat=(dat<<8)|LSB;
	Display_Seg();
	
	if((dat&0xf800)==0x0000)
		DS18B20_Temp=dat*0.0625;
	
	DS18B20_Temp_Seg=DS18B20_Temp*100;
	Display_Seg();
	
	DS18B20_Temp_Con=(DS18B20_Temp_Seg/1000)%10+(DS18B20_Temp_Seg/100)%10;
}
//

/*------------超声---------------------*/
float Read_Dis()
{
	float temp=0;
	US_TX=1;
	count_t=0;
	count_s=0;
	if(US_RX==1)
	{
		temp=(170*count_s)+(8.5*count_t);
	}
	US_TX=0;
	return temp;
}

void Dis_Proc()
{
	int Dis_Now,Dis_Old;
	Dis_Now=Dis_Old;
	
	Dis_Now=Read_Dis();
	if(Dis_Now!=Dis_Old)
	{
		Dis=Dis_Now;
		if(Dis_Now-Dis_Old<5||Dis_Old-Dis_Now<5)
			Dis_stat=1;
		else if(Dis_Now-Dis_Old>10||Dis_Old-Dis_Now>10)
			Dis_stat=3;
		else
			Dis_stat=2;
	}
	Dis_smg=Dis*10;
}


/*------------Output-------------------*/
void OutPut_Porc()
{
	switch(light_level)
	{
		case 1:
			LED_stat=(LED_stat&0xf0)|0x0e;
		break;
		
		case 2:
			LED_stat=(LED_stat&0xf0)|0x0c;
		break;
		
		case 3:
			LED_stat=(LED_stat&0xf0)|0x08;
		break;
		
		case 4:
			LED_stat=(LED_stat&0xf0)|0x00;
		break;
	}
	
	switch (Dis_stat)
	{
		case 1:
			LED_stat=(LED_stat&0x7f)|0x80;
		break;
		//
		case 2:
			LED_stat=(LED_stat&0x7f);
		break;
		//
		case 3:
			LED_stat=(LED_stat&0x7f)|0x80;
		break;
	}
	Set_HC573(4,LED_stat);
	
	if(DS18B20_Temp_Seg>=DS18B20_Temp_P*100&&Dis>=Dis_P)//温度&距离比较
	{
		Layer_times++;
		Layer_stat=0x10;	
	}
	else
		Layer_stat=0x00;
	
	Set_HC573(5,Layer_stat);
}
//


/*------------Init--------------------*/
void Init_sys()
{
	Set_HC573(4,0xff);
	Set_HC573(5,0x00);
	Close_All_Seg();
	Timer_Init();
	Set_PCF8591_Ain1();
	Begin_DS18B20();
}
//

/*------------Main---------------------*/
void main()
{
	Init_sys();
	while(1)
	{
		Key_Proc();
		Display_Seg();
		PCF8591_Proc();
		Read_DS18B20_Temp();
		OutPut_Porc();
		Dis_Proc();
	}
}
