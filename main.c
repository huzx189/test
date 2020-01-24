/*****************************************************************************
LCD1602  4线AD显示实验  本实验采用AIN0输入  实验时将跳线帽接至+5V 
将J16短接EF0  J14短接内部基准  

淘宝地址http://shop61271263.taobao.com/
*****************************************************************************/
#include <c8051f020.h>
#define uchar unsigned char
#define uint unsigned int 
#define ulong unsigned long         
#define SYSCLK 22118400

#define DataPort P0
sbit LCM_V0=P0^0; 
sbit LCM_RS=P0^1;		
sbit LCM_RW=P0^2;		
sbit LCM_EN=P0^3; 
sbit k1=P2^0 ;
sbit k2=P2^1 ;
sbit k3=P2^2 ;
sbit jdq=P2^4;
bit aa,bb,a0,a1,a2,a3;

ulong AD,AD0,GD,GS;
uint GC=0 ;
uint mm;
uchar count=0;
uchar temp_l;
uchar temp_h;
uchar dispbuf[6];
uchar code shen[]={"HUST WELCOME !"};
uchar code shen0[]={"GS:       mV"};
uchar code shen1[]={"GD:       mV"};
uchar code shen2[]={"GLOSS:     %  "};
uchar code shen3[]={"              "};
uchar code word[]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x20};
void PORT_Init (void)
{
		XBR2    = 0x40;  		//使能交叉开关                   
	     	
		P0MDOUT  = 0xff;		 //推挽输出
	//	P2MDOUT   = 0x10;
}

void SYSCLK_Init (void)
{
	   int i;                              /*延时计数器*/
	   OSCXCN = 0x67;                      /*打开外部晶体振荡器*/
	   for (i=0; i < 256; i++) ;           /*延时*/
	   while (!(OSCXCN & 0x80)) ;          /*查询外部晶体是否稳定起振*/
	   OSCICN = 0x88;                      /*用外部晶体作为系统时钟,并使能时钟丢失检测*/
}

/*******************************/
void delay(uint z)
{
		uint x,y;
		for(x=0;x<z;x++)
			for(y=0;y<121;y++);	
}
/*******************************/
void EN1(void)
{
		LCM_EN=1;
		delay(100);
		LCM_EN=0;
		delay(100);
}
/*******写命令**********/
void lcd_wmc(uchar CMD)
{
		DataPort=(CMD& 0xf0);
		LCM_RS=0;
		LCM_V0=0;
		EN1();
		DataPort=(CMD& 0x0f)<<4;
		LCM_RS=0;
		LCM_V0=0;
		EN1();
}
/*******写数据***********/

void lcd_wmd(uchar dataW)
{	
		DataPort=(dataW& 0xf0);
		LCM_RS=1;
		LCM_V0=0;
		EN1();
		DataPort=(dataW& 0x0f)<<4;
		LCM_RS=1;
		LCM_V0=0;
		EN1();
}

/*******更新缓冲区子程序*******/
void newbuf(ulong AD1)
{
		dispbuf[0]=(AD1%10000)/1000;
		dispbuf[1]=(AD1%1000)/100;
		dispbuf[2]=(AD1%100)/10;
		dispbuf[3]=AD1%10;
		AD=0;
		mm=0;
}

/*******显示子程序**********/
void disp(uchar dispadd)
{
	   uchar tmp,i;
	   lcd_wmc(dispadd);/*******写命令 主要是写地址**********/
	
		for(i=0;i<4;i++)/*******显示4位数据**********/
		{
		   tmp=dispbuf[i];
		   tmp=word[tmp];
		   lcd_wmd(tmp);
	   }
}

/*******液晶显示*******/
void lcd_display()
{
	  uchar i;
	 
	  if(a1)
	  {	  
		  lcd_wmc(0x80);	  	   //显示“GS:   ”
		  for(i=0;i<12;i++)		
		  lcd_wmd(shen0[i]);  
		  //lcd_wmc(0xc0);
	  }
	  else if(a2)
	  {
		  lcd_wmc(0x80);	  	   //显示“GD：  ”
		  for(i=0;i<12;i++)		
		  lcd_wmd(shen1[i]);
		  //lcd_wmc(0xc0);
	  }
	  else if(a3)				   //显示“GLOSS：  ”
	  {
		  lcd_wmc(0x80);	 
		  for(i=0;i<14;i++)		
		  lcd_wmd(shen2[i]);
		  //lcd_wmc(0xc0);
	  }

}

/*******初始化液晶*******/
void lcd_init()
{
	  uchar i;
	  lcd_wmc(0x01);
	  lcd_wmc(0x28);
	  lcd_wmc(0x0c);
	  lcd_wmc(0x06);
	  lcd_wmc(0x80);
	  for(i=0;i<14;i++)		  //显示HUST WELCOME !
	  lcd_wmd(shen[i]);
	  lcd_wmc(0xc0);
	  a0=1 ;
}

//函数功能:      定时器3初始化
//入口参数:      int counts
void Timer3_Init (int counts)
{
		TMR3CN = 0x00;                      
		TMR3RLL  = -counts;  	//初始化重载值           
		TMR3L    = 0xffff; 		//设置自动重载值                 
		TMR3CN |= 0x04;                   
}

//函数功能:      ADC0初始化
void ADC_Init(void)
{
		ADC0CN  |=0x84;		/*ADC0使能，定时器3溢出使能模/数转换*/
		REF0CN |=0x03;		/*内部电压基准提供从VREF脚输出，ADC0电压基准取自VREF0*/
		ADC0CF |=0x50;		/*转换周期和增益为1*/
		AMX0CF |=0x01;		/*单端输入*/
		EIE2   |=0x02;		/*ADC0转换结束中断*/
}

//函数功能:      AD中断服务程序，采集20次数据求平均，然后关闭中断
void ADC_ISR (void) interrupt 15
{

	AD0INT=0;			//清A/D中断标志
   	temp_l=ADC0L;
    temp_h=ADC0H;
   	mm=temp_h*256+temp_l;
	AD+=mm;
	count++;
	if(count==20)
	{
		AD=AD/20;
		AD0=AD*2430/2048;
	 	aa=1;
		count=0;
		EA=0 ;
	}  	
}

void main (void) 
{	
	uchar i;
	WDTCN = 0xde;
	WDTCN = 0xad;   					//禁止看门狗定时器
	SYSCLK_Init ();
	PORT_Init();
	lcd_init();
 	LCM_V0=0;
	Timer3_Init (SYSCLK / 25000 );  
	ADC_Init();                                     
//	EA = 1;									
	AMX0SL |=0x00;         				//通道0
	delay(1);

	while (1)
	{  
		
		//第一次采样
		if(!k1)
		{
			delay(10) ;					//消抖
			if(!k1)	
			{	
				if(!jdq) jdq=1 ;	
				delay(100) ;	
				if(a0|a2|a3)			 //清屏，这一点很重要不然前一次显示的数据会对后面的数据产生影响
				{			
					lcd_wmc(0x80) ;
		  			for(i=0;i<14;i++)		
		  			lcd_wmd(shen3[i]) ;
				}
				EA=1 ;					//开中断，等待20次平均值采完 ，然后关闭中断
				while(!aa) ;			//这里没设置好，采到的数据会乱跳  
				if(aa)
				{	
					a0=0 ;						
					a1=1 ;
					a2=a3=0 ;
					lcd_display() ;		
		 			GS=AD0 ;	  		//记录采到的AD值
					newbuf(AD0) ;	  						
					disp(0x86) ;
					AD0=0 ;
					aa=0 ;			    //清标志位				
				}		
			}
		}

	   //K2的设置基本上与K1相同，第二次采样
		if(!k2)
		{
			delay(10) ;
			if(!k2)	
			{	
				if(jdq) jdq=0 ;
				delay(10) ;
				if(a0|a1|a3)
				{
					lcd_wmc(0x80) ;
		  			for(i=0;i<14;i++)		
		  			lcd_wmd(shen3[i]) ;							                                         
				}
				EA=1 ;	
				while(!aa) ;				  
				if(aa)
				{
					a0=0 ;
					a2=1 ;
					a1=a3=0 ;
					lcd_display() ;	
					GD=AD0;
					newbuf(AD0) ;
					AD0=0 ; 
					disp(0x86) ;
					aa=0 ;  											
				}
			
			}
		 }

		//K3按下就要求得到光泽度的值
		if(!k3)
		{
			delay(10) ;
			if(!k3)	
			{		
		
					if(a0|a1|a2)				   //清屏
					{
						lcd_wmc(0x80) ;
			  			for(i=0;i<16;i++)		
			  			lcd_wmd(shen3[i]) ;
					}
											
					a0=0 ;						  //清状态标志位
					a3=1 ;
					a1=a2=0 ;
					GC=(GS*100)/GD ;			  //光泽度的计算公式
					lcd_display() ;	
					newbuf(GC) ;
					disp(0x86) ;			
				
			}
		}		   
	}

}