/*
 * server.c
 *
 *  Created on: Jun 11, 2020
 *      Author: VIPIN
 */


#include "xparameters.h"
#include "xuartps.h"
#include <sleep.h>
#include <xil_io.h>
#include "wifi.h"
#include "xparameters.h"
#include <iostream>

using namespace std;

std::string ssid= "HUAWEI-dpNd";
std::string pwd = "BjgWf4Jf";

void loop(espWiFi*);
int stringToBin(std::string in);
std::string intToBinString(int in);

int main(){
    int status;
    espWiFi myWiFi;
    do{
        status = myWiFi.initWifi(921600,ssid,pwd);
        if(status == WIFI_SUCCESS)
            xil_printf("Successfully started HTTP Server\n\r");
        else{
            xil_printf("Failed retrying...\n\r");
        }
    }while(status != WIFI_SUCCESS);
    loop(&myWiFi);
    return 0;
}



void loop(espWiFi* myWiFi) {
  int val;
  string data;
  u32 found;
  while(1){
    data=myWiFi->getPacket();
    found = data.find("led=");
    if(found != string::npos){
        myWiFi->sendPacket("Turned on LED!!");
        val=stringToBin(data.substr(found+4,8));
        Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR,val);
    }
    found = data.find("switch");
    if(found != string::npos){
        val = Xil_In32(XPAR_AXI_GPIO_0_BASEADDR);
        myWiFi->sendPacket("Switch Pos = "+intToBinString(val));
    }
  }
}

int stringToBin(std::string in){
    int num=0;
    for(u32 i=0;i < in.length(); i++){
        num = 2*num + (in[i]-'0');
    }
    return num;
}

std::string intToBinString(int in){
    std::string Out="";
    int bit;
    for(u32 i=0;i < 8; i++){
        bit = in%2;
        Out = to_string(bit)+Out;
        in = in/2;
    }
    return Out;
}

