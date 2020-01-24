/*****************************************************************************
LCD1602  4��AD��ʾʵ��  ��ʵ�����AIN0����  ʵ��ʱ������ñ����+5V 
��J16�̽�EF0  J14�̽��ڲ���׼  

�Ա���ַhttp://shop61271263.taobao.com/
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
		XBR2    = 0x40;  		//ʹ�ܽ��濪��                   
	     	
		P0MDOUT  = 0xff;		 //�������
	//	P2MDOUT   = 0x10;
}

void SYSCLK_Init (void)
{
	   int i;                              /*��ʱ������*/
	   OSCXCN = 0x67;                      /*���ⲿ��������*/
	   for (i=0; i < 256; i++) ;           /*��ʱ*/
	   while (!(OSCXCN & 0x80)) ;          /*��ѯ�ⲿ�����Ƿ��ȶ�����*/
	   OSCICN = 0x88;                      /*���ⲿ������Ϊϵͳʱ��,��ʹ��ʱ�Ӷ�ʧ���*/
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
/*******д����**********/
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
/*******д����***********/

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

/*******���»������ӳ���*******/
void newbuf(ulong AD1)
{
		dispbuf[0]=(AD1%10000)/1000;
		dispbuf[1]=(AD1%1000)/100;
		dispbuf[2]=(AD1%100)/10;
		dispbuf[3]=AD1%10;
		AD=0;
		mm=0;
}

/*******��ʾ�ӳ���**********/
void disp(uchar dispadd)
{
	   uchar tmp,i;
	   lcd_wmc(dispadd);/*******д���� ��Ҫ��д��ַ**********/
	
		for(i=0;i<4;i++)/*******��ʾ4λ����**********/
		{
		   tmp=dispbuf[i];
		   tmp=word[tmp];
		   lcd_wmd(tmp);
	   }
}

/*******Һ����ʾ*******/
void lcd_display()
{
	  uchar i;
	 
	  if(a1)
	  {	  
		  lcd_wmc(0x80);	  	   //��ʾ��GS:   ��
		  for(i=0;i<12;i++)		
		  lcd_wmd(shen0[i]);  
		  //lcd_wmc(0xc0);
	  }
	  else if(a2)
	  {
		  lcd_wmc(0x80);	  	   //��ʾ��GD��  ��
		  for(i=0;i<12;i++)		
		  lcd_wmd(shen1[i]);
		  //lcd_wmc(0xc0);
	  }
	  else if(a3)				   //��ʾ��GLOSS��  ��
	  {
		  lcd_wmc(0x80);	 
		  for(i=0;i<14;i++)		
		  lcd_wmd(shen2[i]);
		  //lcd_wmc(0xc0);
	  }

}

/*******��ʼ��Һ��*******/
void lcd_init()
{
	  uchar i;
	  lcd_wmc(0x01);
	  lcd_wmc(0x28);
	  lcd_wmc(0x0c);
	  lcd_wmc(0x06);
	  lcd_wmc(0x80);
	  for(i=0;i<14;i++)		  //��ʾHUST WELCOME !
	  lcd_wmd(shen[i]);
	  lcd_wmc(0xc0);
	  a0=1 ;
}

//��������:      ��ʱ��3��ʼ��
//��ڲ���:      int counts
void Timer3_Init (int counts)
{
		TMR3CN = 0x00;                      
		TMR3RLL  = -counts;  	//��ʼ������ֵ           
		TMR3L    = 0xffff; 		//�����Զ�����ֵ                 
		TMR3CN |= 0x04;                   
}

//��������:      ADC0��ʼ��
void ADC_Init(void)
{
		ADC0CN  |=0x84;		/*ADC0ʹ�ܣ���ʱ��3���ʹ��ģ/��ת��*/
		REF0CN |=0x03;		/*�ڲ���ѹ��׼�ṩ��VREF�������ADC0��ѹ��׼ȡ��VREF0*/
		ADC0CF |=0x50;		/*ת�����ں�����Ϊ1*/
		AMX0CF |=0x01;		/*��������*/
		EIE2   |=0x02;		/*ADC0ת�������ж�*/
}

//��������:      AD�жϷ�����򣬲ɼ�20��������ƽ����Ȼ��ر��ж�
void ADC_ISR (void) interrupt 15
{

	AD0INT=0;			//��A/D�жϱ�־
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
	WDTCN = 0xad;   					//��ֹ���Ź���ʱ��
	SYSCLK_Init ();
	PORT_Init();
	lcd_init();
 	LCM_V0=0;
	Timer3_Init (SYSCLK / 25000 );  
	ADC_Init();                                     
//	EA = 1;									
	AMX0SL |=0x00;         				//ͨ��0
	delay(1);

	while (1)
	{  
		
		//��һ�β���
		if(!k1)
		{
			delay(10) ;					//����
			if(!k1)	
			{	
				if(!jdq) jdq=1 ;	
				delay(100) ;	
				if(a0|a2|a3)			 //��������һ�����Ҫ��Ȼǰһ����ʾ�����ݻ�Ժ�������ݲ���Ӱ��
				{			
					lcd_wmc(0x80) ;
		  			for(i=0;i<14;i++)		
		  			lcd_wmd(shen3[i]) ;
				}
				EA=1 ;					//���жϣ��ȴ�20��ƽ��ֵ���� ��Ȼ��ر��ж�
				while(!aa) ;			//����û���úã��ɵ������ݻ�����  
				if(aa)
				{	
					a0=0 ;						
					a1=1 ;
					a2=a3=0 ;
					lcd_display() ;		
		 			GS=AD0 ;	  		//��¼�ɵ���ADֵ
					newbuf(AD0) ;	  						
					disp(0x86) ;
					AD0=0 ;
					aa=0 ;			    //���־λ				
				}		
			}
		}

	   //K2�����û�������K1��ͬ���ڶ��β���
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

		//K3���¾�Ҫ��õ�����ȵ�ֵ
		if(!k3)
		{
			delay(10) ;
			if(!k3)	
			{		
		
					if(a0|a1|a2)				   //����
					{
						lcd_wmc(0x80) ;
			  			for(i=0;i<16;i++)		
			  			lcd_wmd(shen3[i]) ;
					}
											
					a0=0 ;						  //��״̬��־λ
					a3=1 ;
					a1=a2=0 ;
					GC=(GS*100)/GD ;			  //����ȵļ��㹫ʽ
					lcd_display() ;	
					newbuf(GC) ;
					disp(0x86) ;			
				
			}
		}		   
	}

}