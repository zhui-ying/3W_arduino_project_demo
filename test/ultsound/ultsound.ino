
//������ģ��
int Echo = 2;
int Trig = 3;
unsigned char distance_cm = 0;

//��ȡ����������
void Distance()
{
	static unsigned char count = 0;
	long IntervalTime=0; //����һ��ʱ�����
	digitalWrite(Trig, 1);//�øߵ�ƽ
	delayMicroseconds(15);//��ʱ15us
	digitalWrite(Trig, 0);//��Ϊ�͵�ƽ
	IntervalTime=pulseIn(Echo, HIGH);//���Դ��ĺ������������ĸߵ�ƽ�Ŀ�ȣ���λus
	float S=IntervalTime/58.00; //ʹ�ø����������룬��λcm
	if(S != 0)//ȥ���ɼ����ɹ������
	{
		distance_cm = (unsigned char)S;
	}
}

void setup() {
	pinMode(Echo, INPUT);
	pinMode(Trig, OUTPUT);
	
	Serial.begin(9600);
}

void loop() {
	Distance();
	Serial.print(distance_cm);
  Serial.println(" cm");
	delay(500);
}
