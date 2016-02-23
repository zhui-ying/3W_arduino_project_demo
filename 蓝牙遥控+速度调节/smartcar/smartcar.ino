/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：小车蓝牙遥控调整实验
* 实验说明 ：按下KEY_L一次，左边轮子的脉宽增加2%，按下KEY_H一次，右边轮子的脉宽增加2%
*             用于调整两个轮子的转速
* 实验平台 ：流星7号、arduino UNO R3 
* 连接方式 ：请参考interface.h文件
* 注    意 ：指令必须连续发送才会使小车动作，若停止发送指令，则小车会停止
* 作    者 ：航太电子产品研发部    QQ ：1909197536
* 店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/

#include <MsTimer2.h> 
#include "interface.h"

//变量定义
int timer_count;
unsigned int speed_count;//占空比计数器 50次一周期
char front_left_speed_duty;
char front_right_speed_duty;
char behind_left_speed_duty;
char behind_right_speed_duty;
unsigned char tick_5ms = 0;//5ms计数器，作为主函数的基本周期
unsigned char tick_1ms = 0;//1ms计数器，作为电机的基本计数器
unsigned char tick_200ms = 0;
unsigned char switch_flag=0;

char ctrl_comm;//控制指令
unsigned char continue_time = 0;
char ir_rec_flag;
char bt_rec_flag;
char speed_duty_l = 30;//设置左右两轮的速度
char speed_duty_r = 30;

//函数声明
void CarMove();
void CarGo();
void CarBack();
void CarLeft();
void CarRight();
void CarStop();

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
	front_left_speed_duty=speed_duty_l;
	front_right_speed_duty=speed_duty_r;
}

//后退
void CarBack()
{
	front_left_speed_duty=-speed_duty_l;
	front_right_speed_duty=-speed_duty_r;
}

//向左
void CarLeft()
{
	front_left_speed_duty=0;
	front_right_speed_duty=speed_duty_r;
}

//向右
void CarRight()
{
	front_left_speed_duty=speed_duty_l;
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
	MsTimer2::set(1, flash);        // 中断设置函数，每 1ms 进入一次中断
	MsTimer2::start();                //开始计时
	CarStop();
}

void loop() {  
	//指令接收部分	
	//蓝牙遥控部分
	  // 读取串口发送的信息:
    if (Serial.available() > 0)
	{
		char inChar;
		inChar = Serial.read();
	    if(ctrl_comm != inChar || continue_time == 1)
		{
			bt_rec_flag = 1;
			ctrl_comm = inChar;
		}
        continue_time = 40;//更新持续时间
    }       
	
	//按键处理
	int key_value;
	key_value = KeyScan();
	if(key_value == KEY_L) speed_duty_l++;
	if(key_value == KEY_H) speed_duty_r++;

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
		continue_time--;//200ms 无接收指令就停车
		if(continue_time == 0)
		{
			continue_time = 1;
			CarStop();
		}
		//do something
		if(bt_rec_flag == 1)//接收到红外信号
		{
			bt_rec_flag = 0;
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



