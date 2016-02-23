/********************************* 深圳市航太电子有限公司 *******************************
  实 验 名 ：小车wifi遥控实验
  实验说明 ：使用手机连接wifi模块后，通过手机发送指令来控制小车
             需要把手机端当做服务器，打开后获取IP和端口号，然后把信息写到
			 HOST_NAME 和 HOST_PORT 编译后重新烧录
  实验平台 ：流星7号、arduino UNO R3
  连接方式 ：请参考interface.h文件
  注    意 ：指令必须连续发送才会使小车动作，若停止发送指令，则小车会停止
			 这里初次连接时使用的是smartlink方式，具体操作请看教程
			 为了方便调试，用到了软件串口mySerial
  作    者 ：航太电子产品研发部    QQ ：1909197536
  店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/
  
#include <MsTimer2.h>
#include "interface.h"
#include "ESP8266.h"
#include <SoftwareSerial.h>

#define HOST_NAME   "192.168.31.195"
#define HOST_PORT   (8080)

#define DBG(x)  mySerial.println(x)

SoftwareSerial mySerial(3, 2); /* RX:D3, TX:D2 */

ESP8266 wifi(Serial, 115200); //刚开始默认波特率为115200
uint8_t buffer[8] = {0};
//变量定义
int timer_count;

unsigned int speed_count;//占空比计数器 50次一周期
char front_left_speed_duty;
char front_right_speed_duty;
char behind_left_speed_duty;
char behind_right_speed_duty;
unsigned char tick_5ms = 0;//5ms计数器，作为主函数的基本周期
unsigned char tick_1ms = 0;//1ms计数器，作为电机的基本计数器
unsigned char tick_500ms = 0;
unsigned int tick_3s = 0;//3s定时器，作为心跳包发送周期
unsigned char switch_flag = 0;

char ctrl_comm;//控制指令
unsigned char continue_time = 0;
char wifi_rec_flag;

//函数声明
void CarMove();
void CarGo();
void CarBack();
void CarLeft();
void CarRight();
void CarStop();
void AutoLink(void);
bool WifiInit(void);

#if 1

//根据占空比驱动电机转动
void CarMove()
{
  //左前轮
  if (front_left_speed_duty > 0) //向前
  {
    if (speed_count < front_left_speed_duty)
    {
      FRONT_LEFT_GO;
    } else
    {
      FRONT_LEFT_STOP;
    }
  }
  else if (front_left_speed_duty < 0) //向后
  {
    if (speed_count < (-1)*front_left_speed_duty)
    {
      FRONT_LEFT_BACK;
    } else
    {
      FRONT_LEFT_STOP;
    }
  }
  else                //停止
  {
    FRONT_LEFT_STOP;
  }

  //右前轮
  if (front_right_speed_duty > 0) //向前
  {
    if (speed_count < front_right_speed_duty)
    {
      FRONT_RIGHT_GO;
    } else                //停止
    {
      FRONT_RIGHT_STOP;
    }
  }
  else if (front_right_speed_duty < 0) //向后
  {
    if (speed_count < (-1)*front_right_speed_duty)
    {
      FRONT_RIGHT_BACK;
    } else                //停止
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
  front_left_speed_duty = SPEED_DUTY;
  front_right_speed_duty = SPEED_DUTY;
}

//后退
void CarBack()
{
  front_left_speed_duty = -SPEED_DUTY;
  front_right_speed_duty = -SPEED_DUTY;
}

//向左
void CarLeft()
{
  front_left_speed_duty = 0;
  front_right_speed_duty = SPEED_DUTY;
}

//向右
void CarRight()
{
  front_left_speed_duty = SPEED_DUTY;
  front_right_speed_duty = 0;
}

//停止
void CarStop()
{
  front_left_speed_duty = 0;
  front_right_speed_duty = 0;
}

#endif

/*******************************************************************************
  函 数 名 ：AutoLink
  函数功能 ：自动连接，前10s自动连接，若连接失败则进入smartlink模式30s，若依然失败
             则再次回到自动连接，直到连接成功
  输    入 ：无
  输    出 ：无
*******************************************************************************/
void AutoLink(void)
{
  int8_t status = STATUS_LOSTIP;
  while (status != STATUS_GETIP)
  {
    uint32_t start_time = millis();
    DBG("start auto link");
    //10s自动连接时间
    while ((millis() - start_time < 10000) && status != STATUS_GETIP)
    {
      status = wifi.getSystemStatus();
      delay(500);
    }

    //连接失败进入smartlink模式 30s
    if (status != STATUS_GETIP)
    {
      String link_msg;
      DBG("start smartlink");
      wifi.stopSmartLink();

      if (true == wifi.smartLink((uint8_t)AIR_LINK, link_msg))
      {
        DBG(link_msg);
        start_time = millis();//等待获取IP
        while ((millis() - start_time < 5000) && status != STATUS_GETIP)
        {
          status = wifi.getSystemStatus();
          delay(500);
        }
      } else
      {
        wifi.stopSmartLink();
        delay(500);
        DBG("link AP fail");
    wifi.restart();
      }
    }
  }
  DBG("link AP OK");
}

bool WifiInit(void)
{
  DBG("start link");
  wifi.restart();
  wifi.setOprToStationSoftAP();
  AutoLink();
  wifi.disableMUX();
creattcp0:
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) {//连接主机
    DBG("create tcp ok\r\n");
  } else {
    DBG("create tcp err\r\n");
    delay(2000);
    goto creattcp0;
  }
  delay(1000);
  if (wifi.getSystemStatus() == STATUS_GETLINK)
    return true;
  else
    return false;
}

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
 // pinMode(Echo, INPUT);
 // pinMode(Trig, OUTPUT);
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
  if (speed_count >= 50) //50ms周期 //modfied by LC 2015.09.12 9:56
  {
    speed_count = 0;
  }
  CarMove();
}

void setup() {
  mySerial.begin(9600);
  DBG("system begin");
  IOInit();
  LED_ON;
  
  DBG("Set baud");
	while(1)
	{
		Serial.begin(115200);
		wifi.SetBaud(19200);
		Serial.begin(9600);//更改波特率到9600
		if(wifi.SetBaud(19200))//在新波特率下检查是否成功
		{
			break;//成功就退出
		}	
	}
	
  while(!WifiInit());
  LED_OFF;
  MsTimer2::set(1, flash);   // 中断设置函数，每 1ms 进入一次中断
  MsTimer2::start();   //开始计时
  CarStop();
}

void loop() {
  //指令接收部分
    if(Serial.available() > 5)
    {
      uint32_t len = wifi.recv(buffer, sizeof(buffer), 50);
      if (len > 0)
      {
      char inChar;
      inChar = buffer[0];
      if (ctrl_comm != inChar || continue_time == 1)
      {
        wifi_rec_flag = 1;
        ctrl_comm = inChar;
      }
      continue_time = 60;//更新持续时间
	  tick_3s = 0;//心跳包周期
      }     
    }

  //执行部分
  if (tick_5ms >= 5)
  {
    tick_5ms = 0;
    tick_500ms++;
	tick_3s++;
    if (tick_500ms >= 100)
    {
      tick_500ms = 0;
      if (switch_flag)
      {
        LED_ON;
        switch_flag = 0;
      } else
      {
        LED_OFF;
        switch_flag = 1;
      }
    }
	
	if(tick_3s >= 600)
	{
		tick_3s = 0;
		//定时检查连接是否正常，如果不正常,wifi复位重新连接
		if (wifi.getSystemStatus() != STATUS_GETLINK)
		{
		  DBG("server unlink\r\n");
		  while(!WifiInit());
		}
		delay(10);
		wifi.rx_empty();		
	}
	
    //continue_time--;//300ms 无接收指令就停车//去掉延时检测 modfied by LC 2016.01.07 17:02
    if (continue_time == 0)
    {
      continue_time = 1;
      CarStop();
    }
    //do something
    if (wifi_rec_flag == 1) //接收到红外信号
    {
      wifi_rec_flag = 0;
      switch (ctrl_comm)
      {
        case COMM_UP:    CarGo(); break;
        case COMM_DOWN:  CarBack(); break;
        case COMM_LEFT:  CarLeft(); break;
        case COMM_RIGHT: CarRight(); break;
        case COMM_STOP:  CarStop(); break;
        default : break;
      }
    }
  }
}


