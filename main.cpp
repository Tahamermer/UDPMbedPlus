/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "EthernetInterface.h"
#include <picojson.h>

int PWMValue1=0,PWMValue2=0,PwmValue3=0,PwmValue4=0;
float PwmVal1,PwmVal2,PwmVal3,PwmVal4,PwmVal;
float analogData[6];

AnalogIn AIN0(p15);
AnalogIn AIN1(p16);
AnalogIn AIN2(p17);
AnalogIn AIN3(p18);
AnalogIn AIN4(p19);
AnalogIn AIN5(p20);

PwmOut  PWM1(p26);
PwmOut  PWM2(p25);
PwmOut  PWM3(p24);
PwmOut  PWM4(p23);

DigitalOut Led1(LED1);
DigitalOut Led2(LED2);
DigitalOut Led3(LED3);

static const char* mbedIP       = "192.168.109.131";       //IP
static const char* mbedMask     = "255.255.255.0";          // Mask
static const char* mbedGateway  = "192.168.109.1";          //Gateway
static const int   udpPort      = 4141;                     //Port
char* destIP = "192.168.109.137";                           //Destination IP


UDPSocket socket;
SocketAddress sockAddr;

EthernetInterface eth;

void readAnalogChannels()
{
    analogData[0] = AIN0.read()*3.3;
    analogData[1] = AIN1.read()*3.3;
    analogData[2] = AIN2.read()*3.3;
    analogData[3] = AIN3.read()*3.3;
    analogData[4] = AIN4.read()*3.3;
    analogData[5] = AIN5.read()*3.3;
}

void receiver()
{
    SocketAddress rd_addr;
    UDPSocket rd_sock(&eth);
    //bind to specific receiving port
    int bind = rd_sock.bind(udpPort);
    //buffer for received data
    char buffer[256];
    picojson::value v;
    double pwm1value;
    double pwm2value;
    double pwm3value;
    double pwm4value;
    while(true) {
        int ret = rd_sock.recvfrom(&rd_addr,buffer, sizeof(buffer));
        if(ret > 0) {
            buffer[ret] = '\0';
            const char *json = buffer;
            string err = picojson::parse(v,json, json + strlen(json));    //parsing json data
            //PWM
            pwm1value = (int)v.get("PWMValue1").get<double>();
            pwm2value = (int)v.get("PWMValue2").get<double>();
            pwm3value = (int)v.get("PWMValue3").get<double>();
            pwm4value = (int)v.get("PWMValue4").get<double>();
            PwmVal1=pwm1value/(float)100;                       //dividing for duty cycle
            PwmVal2=pwm2value/(float)100;
            PwmVal3=pwm3value/(float)100;
            PwmVal4=pwm4value/(float)100;
            PWM1.period_us(1.0f);                            //  microsecond period
            PWM1.write(PwmVal1);                            //  duty cycle
            PWM2.period_us(1.0f);
            PWM2.write(PwmVal2);
            PWM3.period_us(1.0f);
            PWM3.write(PwmVal3);
            PWM4.period_us(1.0f);
            PWM4.write(PwmVal4);
            //
            Led2 = !Led2;
            Thread::wait(50);
        }
    }
}
void transmitter()
{
    char outbuf[256], *put = outbuf;
    SocketAddress td_addr(destIP, udpPort);
    UDPSocket td_sock(&eth);
    while(1) {
        readAnalogChannels();
        picojson::object a;
        string DeviceType ="LPC1768";
        a["Dev:"] = picojson::value(DeviceType);
        a["AD0"] = picojson::value(analogData[0]);
        a["AD1"] = picojson::value(analogData[1]);
        a["AD2"] = picojson::value(analogData[2]);
        a["AD3"] = picojson::value(analogData[3]);
        a["AD4"] = picojson::value(analogData[4]);
        a["AD5"] = picojson::value(analogData[5]);
        string str = picojson::value(a).serialize();
        int response = td_sock.sendto(td_addr, str.c_str(),300);
        Led3 = !Led3;
        Thread::wait(50);
    }
}

int main()
{
    Thread receive;
    Thread transmit;
    // Set-up static IP network for transmitting device
    eth.set_network(mbedIP, mbedMask, mbedGateway);     // my device address
    // Start network
    eth.connect();
    transmit.start(transmitter);
    receive.start(receiver);
    while(true){
          Led1 = !Led1;
    }
}
