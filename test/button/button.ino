/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：按键实验
* 实验说明 ：
* 实验平台 ：arduino UNO R3 流行5号智能小车
* 连接方式 ：KEY_H 接 13 ， KEY_L 接 A0
* 注    意 ：注意查看arduino的电路图，一个高有效，一个低有效
* 作    者 ：航太电子产品研发部    QQ ：1909197536
* 店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/

//用户KEY
int KEY_L = 14;
int KEY_H = 13;
#define KEY_L_BIT digitalRead(KEY_L)
#define KEY_H_BIT digitalRead(KEY_H)

#define KEY_L_DOWN 0
#define KEY_H_DOWN 1

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

void setup() {
  Serial.begin(9600);
  pinMode(KEY_L, INPUT_PULLUP);
  pinMode(KEY_H, INPUT);
  Serial.println("system begin\r\n");
}

void loop() {
	int key_value;
	key_value = KeyScan();
	if(key_value != -1)
	{
		Serial.print("key value");
		Serial.println(key_value);
	}
 
}

