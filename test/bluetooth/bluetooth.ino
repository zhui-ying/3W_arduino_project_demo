/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：bluetooth实验
* 实验说明 ：与蓝牙模块连接，并实时返回接收的数据
* 实验平台 ：arduino UNO R3 流行5号智能小车
* 连接方式 ：将蓝牙模块直接插到小车对应的蓝牙插座上
* 注    意 ：
* 作    者 ：航太电子产品研发部    QQ ：1909197536
* 店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/

void setup() {
  Serial.begin(9600);
  Serial.println("system begin\r\n");
}

void loop() {
	if (Serial.available() > 0)
	{
		char inChar;
		inChar = Serial.read();
		Serial.print(inChar);
	}   
}

