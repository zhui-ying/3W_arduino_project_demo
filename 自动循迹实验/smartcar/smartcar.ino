/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：小车红外黑线循迹
* 实验说明 ：小车自动沿黑线行驶,按KEY_H键，开始运行，按下KEY_L键，停止运行
* 实验平台 ：流星5号、arduino UNO R3 
* 连接方式 ：请参考interface.h文件
* 注    意 ：因地面和轮子的差异，左右转向时需要根据实际需要调节差速的差值
* 作    者 ：航太电子产品研发部    QQ ：1909197536
* 店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/

#include <MsTimer2.h> 
#include "interface.h"
#include <IRremote.h>

//变量定义
int timer_count;
unsigned int speed_count;//占空比计数器 50次一周期
char front_left_speed_duty;
char front_right_speed_duty;
unsigned char tick_5ms = 0;//5ms计数器，作为主函数的基本周期
unsigned char tick_1ms = 0;//1ms计数器，作为电机的基本计数器
unsigned char tick_200ms = 0;
unsigned char switch_flag=0;

char ctrl_comm = COMM_STOP;//控制指令
char ctrl_comm_last = COMM_STOP;
char sys_status = 0;//系统状态 0 停止 1 运行
//函数声明
void CarMove();
void CarGo();
void CarBack();
void CarLeft();
void CarRight();
void CarStop();
void SearchRun();
int KeyScan(void);

#if 1

//根据占空比驱动电机转动
void CarMove()
{
	//左前轮
	if(front_left_speed_duty > 0)//向前
	{
		if(speed_count < front_left_speed_duty)
		{
			FRONT_LEFT_GO;
		}else
		{
			FRONT_LEFT_STOP;
		}
	}
	else if(front_left_speed_duty < 0)//向后
	{
		if(speed_count < (-1)*front_left_speed_duty)
		{
			FRONT_LEFT_BACK;
		}else
		{
			FRONT_LEFT_STOP;
		}
	}
	else                //停止
	{
		FRONT_LEFT_STOP;
	}
	
	//右前轮
	if(front_right_speed_duty > 0)//向前
	{
		if(speed_count < front_right_speed_duty)
		{
			FRONT_RIGHT_GO;
		}else                //停止
		{
			FRONT_RIGHT_STOP;
		}
	}
	else if(front_right_speed_duty < 0)//向后
	{
		if(speed_count < (-1)*front_right_speed_duty)
		{
			FRONT_RIGHT_BACK;
		}else                //停止
		{
			FRONT_RIGHT_STOP;
		}
	}
	else                //停止
	{
		FRONT_RIGHT_STOP;
	}
}

//向前
void CarGo()
{
	front_left_speed_duty=SPEED_DUTY;
	front_right_speed_duty=SPEED_DUTY;
}

//后退
void CarBack()
{
	front_left_speed_duty=-SPEED_DUTY;
	front_right_speed_duty=-SPEED_DUTY;
}

//向左
void CarLeft()
{
	front_left_speed_duty=0;
	front_right_speed_duty=SPEED_DUTY/2;
}

//向右
void CarRight()
{
	front_left_speed_duty=SPEED_DUTY/2;
	front_right_speed_duty=0;
}

//停止
void CarStop()
{
	front_left_speed_duty=0;
	front_right_speed_duty=0;
}

#endif

void IOInit()
{
	//用户LED
	pinMode(USER_PIN, OUTPUT);
	//KEY
	pinMode(KEY_L, INPUT_PULLUP);
	pinMode(KEY_H, INPUT);
	//红外
	pinMode(IRIN, INPUT);
	//超声波
	pinMode(Echo, INPUT);
    pinMode(Trig, OUTPUT);
	//电机
	pinMode(FRONT_LEFT_F_IO, OUTPUT);
	pinMode(FRONT_LEFT_B_IO, OUTPUT);
	pinMode(FRONT_RIGHT_F_IO, OUTPUT);
	pinMode(FRONT_RIGHT_B_IO, OUTPUT);
	//循迹
	pinMode(SEARCH_M_IO, INPUT);
	pinMode(SEARCH_R_IO, INPUT);
	pinMode(SEARCH_L_IO, INPUT);
	//避障
	pinMode(VOID_R_PIN, INPUT);
	pinMode(VOID_L_PIN, INPUT);	
	//舵机
	pinMode(DUOJI_IO, OUTPUT);
	//测速
	pinMode(FRONT_RIGHT_S_IO, INPUT);
	pinMode(FRONT_LEFT_S_IO, INPUT);

}

//读取键值，如果没读到返回 -1
int KeyScan(void)
{
	int value = -1;
	if(KEY_L_DOWN == KEY_L_BIT)
	{
		delay(5);//delay for a while
		if(KEY_L_DOWN == KEY_L_BIT)
		{
			value = KEY_L;
		}
		while(KEY_L_DOWN == KEY_L_BIT);
	}

	if(KEY_H_DOWN == KEY_H_BIT)
	{
		delay(5);//delay for a while
		if(KEY_H_DOWN == KEY_H_BIT)
		{
			value = KEY_H;
		}
		while(KEY_H_DOWN == KEY_H_BIT);
	}
	return value;
}

//循迹，通过判断三个光电对管的状态来控制小车运动
void SearchRun()
{
	//三路都检测到 前进
	if(SEARCH_M_BIT == BLACK_AREA && SEARCH_L_BIT == BLACK_AREA && SEARCH_R_BIT == BLACK_AREA)
	{
		ctrl_comm = COMM_UP;
		return;
	}
	
	if(SEARCH_R_BIT == BLACK_AREA)//右转
	{
		ctrl_comm = COMM_RIGHT;
	}
	else if(SEARCH_L_BIT == BLACK_AREA)//左转
	{
		ctrl_comm = COMM_LEFT;
	}
	else if(SEARCH_M_BIT == BLACK_AREA)//前进
	{
		ctrl_comm = COMM_UP;
	}
}

//中断处理函数，改变灯的状态
void flash()  
{                        
	tick_5ms++;
	speed_count++;
	if(speed_count >= 50)//50ms周期 //modfied by LC 2015.09.12 9:56
	{
		speed_count = 0;               
	}
	CarMove();
}

void setup() {

	IOInit();
	// 初始化串口参数
	Serial.begin(9600);
	MsTimer2::set(1, flash);   // 中断设置函数，每 1ms 进入一次中断
	MsTimer2::start();   //开始计时
	CarStop();
}

void loop() {  
	//指令接收部分      
	//按键处理
	int key_value;
	key_value = KeyScan();
	if(key_value == KEY_L) sys_status = 0;
	if(key_value == KEY_H) sys_status = 1;
	//执行部分
	if(tick_5ms >= 5)
	{
		tick_5ms = 0;
		tick_200ms++;
		if(tick_200ms >= 40)
		{
			tick_200ms = 0;
			if(switch_flag)
			{
				LED_ON;
				switch_flag = 0;
			}else
			{
				LED_OFF;
				switch_flag = 1;				
			}
		}
		if(sys_status == 1)
		{
			SearchRun();
		}else
		{
			ctrl_comm = COMM_STOP;
		}
		
		//do something
		if(ctrl_comm_last != ctrl_comm)//接收到红外信号
		{
			ctrl_comm_last = ctrl_comm;
			switch(ctrl_comm)
			{
			case COMM_UP:    CarGo();break;
			case COMM_DOWN:  CarBack();break;
			case COMM_LEFT:  CarLeft();break;
			case COMM_RIGHT: CarRight();break;
			case COMM_STOP:  CarStop();break;
			default : break;
			}
            Serial.println(ctrl_comm,HEX);
		}
	}             
}



