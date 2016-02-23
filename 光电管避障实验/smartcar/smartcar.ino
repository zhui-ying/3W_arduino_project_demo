/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：小车光电对管避障实验
* 实验说明 ：通过采集车头前的两组光电对管信号来实现对小车的自动控制
*           按KEY_H键，开始运行，按下KEY_L键，停止运行
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
	front_left_speed_duty=-(SPEED_DUTY+20);//迅速刹车，防止装上
	front_right_speed_duty=-(SPEED_DUTY+20);
}

//向左
void CarLeft()
{
	front_left_speed_duty=0;
	front_right_speed_duty=SPEED_DUTY;
}

//向右
void CarRight()
{
	front_left_speed_duty=SPEED_DUTY;
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

#define VOID_LEFT 1//左边检测到
#define VOID_RIGHT 2//右边检测到
#define VOID_BOTH 3 //两边都检测到
#define VOID_NONE 0//都没检测到
//获取红外避障模块状态
char GetVoidStatus(void)
{
	char left=0,right=0;
	char count;
	if(VOID_L_BIT == BARRIER_Y)
	{
		count = 2;
		while(--count)//10ms 采集10次均要采集到前面障碍物信息，滤波
		{
			if(VOID_L_BIT == BARRIER_N)
				break;
			delay(1);
		}
		if(count == 0) left = 1;
	}
	
	if(VOID_R_BIT == BARRIER_Y)
	{
		count = 2;
		while(--count)//10ms 采集10次均要采集到前面障碍物信息，滤波
		{
			if(VOID_R_BIT == BARRIER_N)
				break;
			delay(1);
		}
		if(count == 0) right = 2;
	}
	
	return left + right;
}

//延时的同时检测红外，一旦发生障碍物，就停止并跳出延时
void DelayCheck(int ms)
{
	while(ms--)
	{
		delay(1);
		if(VOID_NONE != GetVoidStatus())
		{
			CarStop();
			return;
		}
	}
}

//红外避障处理
//处理方式：左边检测到  后退500ms 右转500ms
//			右边检测到  后退500ms 左转500ms
//			两边检测到  后退1000ms 右转500ms
//          没检测到    直行
void VoidRun(void)
{
	char status;
	status = GetVoidStatus();
	
	switch(status)
	{
		case VOID_LEFT: 
			CarBack(); delay(500); CarRight(); DelayCheck(500);
			break;
		case VOID_RIGHT:
			CarBack(); delay(500); CarLeft(); DelayCheck(500);	
			break;
		case VOID_BOTH:
			CarBack(); delay(1000); CarRight(); DelayCheck(500);
			break;
		case VOID_NONE:
			CarGo();
			break;
		default: break;
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
		tick_200ms++;
		if(tick_200ms >= 4)
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
	if(sys_status == 1)
	{
		VoidRun();
	}else
	{
		CarStop();
	}        
}



