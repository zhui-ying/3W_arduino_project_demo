/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 : 红外读取实验
* 实验说明 ：读取车头前5个光电对管的状态
* 实验平台 ：arduino UNO R3 流行5号智能小车
* 连接方式 ：按照接线图连线
* 注    意 ：
* 作    者 ：航太电子产品研发部    QQ ：1909197536
* 店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/

//红外循迹
int SEARCH_M_IO = 9;//中间循迹IO
int SEARCH_R_IO = 8;//右循迹IO
int SEARCH_L_IO = 7;//左循迹IO
#define BLACK_AREA 1
#define WHITE_AREA 0
#define SEARCH_M_BIT digitalRead(SEARCH_M_IO)
#define SEARCH_R_BIT digitalRead(SEARCH_R_IO)
#define SEARCH_L_BIT digitalRead(SEARCH_L_IO)

//循迹避障
int VOID_R_PIN = 10;
int VOID_L_PIN = 6;
#define VOID_R_BIT digitalRead(VOID_R_PIN)
#define VOID_L_BIT digitalRead(VOID_L_PIN)

void IOinit(void)
{
		//循迹
	pinMode(SEARCH_M_IO, INPUT);
	pinMode(SEARCH_R_IO, INPUT);
	pinMode(SEARCH_L_IO, INPUT);
	//避障
	pinMode(VOID_R_PIN, INPUT);
	pinMode(VOID_L_PIN, INPUT);	
}

void setup() {
  Serial.begin(9600);
  IOinit();
  Serial.println("system begin\r\n");
}

void loop() {
	Serial.print("void left:");
	Serial.println(VOID_L_BIT);		
	Serial.print("void right:");
	Serial.println(VOID_R_BIT);

	Serial.print("search left:");
	Serial.println(SEARCH_L_BIT);	
	Serial.print("search Mid:");
	Serial.println(SEARCH_M_BIT);	
	Serial.print("search right:");
	Serial.println(SEARCH_R_BIT);
	Serial.println();
	
	delay(500);
	
}

