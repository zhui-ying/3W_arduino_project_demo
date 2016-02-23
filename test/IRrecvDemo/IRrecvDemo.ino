/*
* IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
* An IR detector/demodulator must be connected to the input RECV_PIN.
* Version 0.1 July, 2009
* Copyright 2009 Ken Shirriff
* http://arcfn.com
*/

#include <IRremote.h>

#define COMM_STOP  'I'//停止
#define COMM_UP    'A'//前进
#define COMM_DOWN  'B'//后退
#define COMM_RIGHT 'D'//右转
#define COMM_LEFT  'C'//左转

int RECV_PIN = 4;
char ctrl_comm;
IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    unsigned int value_temp;
    value_temp = (results.value & 0xffff);//取后16位
    if(value_temp != 0xffff)//去除重复码
    {
      if((value_temp & 0xff00)>>8 == ( ~(value_temp & 0xff) & 0xff))//校验
      {
        unsigned int ctrl_comm_temp = (value_temp & 0xff00)>>8 & 0xff;
        switch(ctrl_comm_temp)//指令转换
        {
        case 0x02: ctrl_comm = COMM_STOP; break;
        case 0x62: ctrl_comm = COMM_UP; break;
        case 0xA8: ctrl_comm = COMM_DOWN; break;
        case 0xC2: ctrl_comm = COMM_RIGHT; break;
        case 0x22: ctrl_comm = COMM_LEFT; break;
        default: break;
        }
    Serial.print("code:");
        Serial.println(results.value, HEX);
    Serial.println(ctrl_comm, HEX);
      }
    }
    
    
    irrecv.resume(); // Receive the next value
  }
}



