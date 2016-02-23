/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：超声波+舵机自动避障实验
* 实验说明 ：通过通过超声波采集前方及左右的距离来控制小车自动避障
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
#include <Servo.h> 

//变量定义
int timer_count;
unsigned int speed_count;//占空比计数器 50次一周期
char front_left_speed_duty;
char front_right_speed_duty;
unsigned char tick_5ms = 0;//5ms计数器，作为主函数的基本周期
unsigned char tick_1ms = 0;//1ms计数器，作为电机的基本计数器
unsigned char tick_200ms = 0;
unsigned char switch_flag=0;
unsigned char distance_cm = 20;
Servo myservo;

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
	front_left_speed_duty=-SPEED_DUTY-10;//迅速刹车，防止装上
	front_right_speed_duty=-SPEED_DUTY-10;
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

void GetDistance()
{
        long IntervalTime=0; //定义一个时间变量
		digitalWrite(Trig, 1);//置高电平
		delayMicroseconds(15);//延时15us
		digitalWrite(Trig, 0);//设为低电平
		IntervalTime=pulseIn(Echo, HIGH);//用自带的函数采样反馈的高电平的宽度，单位us
		float S=IntervalTime/58.00; //使用浮点计算出距离，单位cm
		if(S != 0)//去掉采集不成功的情况
		{
			distance_cm = (unsigned char)S;
		}	
}

//获取超声波距离
void Distance()
{
	static unsigned char count = 0;
	count++;
	if(count >= 20)//每100ms 采集一次
	{
		GetDistance();
	}
}

void DuojiMid()
{
	myservo.write(90);              // 输入对应的角度值，舵机会转到此位置
	delay(300);//延时300ms
}

void DuojiRight()
{
	myservo.write(22);              // 输入对应的角度值，舵机会转到此位置
	delay(300);//延时300ms
}

void DuojiLeft()
{
	myservo.write(158);              // 输入对应的角度值，舵机会转到此位置
	delay(300);//延时300ms
}

///获取三个方向的距离,进来前舵机方向为向前
void GetAllDistance(unsigned int *dis_left,unsigned int *dis_right,unsigned int *dis_direct)
{
	CarStop();
	delay(100);
	GetDistance();
	*dis_direct = distance_cm;
	
	DuojiRight();
	delay(200);
	GetDistance();//获取右边距离
	*dis_right = distance_cm;
	
	DuojiMid();
	DuojiLeft();
	delay(200);
	GetDistance();//获取左边距离
	*dis_left = distance_cm;
	
	DuojiMid();//归位
}

void VoidRun()
{
	 if(distance_cm < 10)//前方有障碍物
	 {
		unsigned int dis_left;//左边距离
		unsigned int dis_right;//右边距离
		unsigned int dis_direct;//中间距离
		if(distance_cm < 8)
		{
			CarBack();
			delay(400);
		}
		
		while(1)
		{
			GetAllDistance(&dis_left,&dis_right,&dis_direct);
			if(dis_direct > 30)//前方距离已经有30cm以上了就不需要管左右了，直接前进
			{
				CarGo();
				delay(500);
				return;				
			}
			if(dis_direct < 8)
			{
				CarBack();
				delay(500);
				continue;
			}
			else if(dis_direct >= dis_left && dis_direct >= dis_right)//前方距离最远
			{
				CarGo();
				return;
			}
			else if(dis_left <= dis_right)//右转
			{
				CarRight();
				delay(700);
			}
			else if(dis_right < dis_left)
			{
				CarLeft();
				delay(700);
			}
		}
	}
	else
	{
		CarGo();
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
	myservo.attach(DUOJI_IO);  // attaches the servo on pin 5 to the servo object
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
	
	if(tick_5ms >= 5)
	{
		tick_5ms = 0;
		if(sys_status == 1)
		{
			Distance();
			VoidRun();
		}else
		{
			CarStop();
		}  
	}  
	      
}



