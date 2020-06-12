/*
 * server.c
 *
 *  Created on: Jun 11, 2020
 *      Author: VIPIN
 */


#include "xparameters.h"
#include "xuartps.h"
#include "string"
#include <sleep.h>
#include <xil_io.h>
//#include <iostream>

using namespace std;

string ssid="\"HUAWEI-dpNd\"";
string pwd = "\"BjgWf4Jf\"";

XUartPs myUart;

void uartBlockTransmitByte(XUartPs *myUart,u8 sendData);
u8 uartBlockReceiveByte(XUartPs *myUart);
void espSendData(char channelNum,string sendData);
string espReceiveData();
void espSendCommand(string command);
void loop();
u8 espBlockReceiveData();
string getString(char x);

int main(){
		string command;
		string recvData;
		u32 status;
		u32 found;
		u32 i;
		XUartPs_Config *myUartConfig;
		myUartConfig = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
		status = XUartPs_CfgInitialize(&myUart, myUartConfig, myUartConfig->BaseAddress);
		if(status != XST_SUCCESS)
			print("Uart initialization failed...\n\r");

		xil_printf("Welcome\n\r");

		espSendCommand("AT\r\n");
		recvData = espReceiveData();

		found = recvData.find("OK");
		if (found != string::npos)
			xil_printf("ESP Detected\n\r");
		else{
			xil_printf("Could not detect ESP\n\r");
			return 0;
		}

		espSendCommand("AT+CWMODE=1\r\n");//Send the AT command to configure the ESP as a station (server/client)
		recvData = espReceiveData();
		found = recvData.find("OK");
		if(found != string::npos)
			xil_printf("Successfully configured as a station\n\r");
		else{
			xil_printf("Error cannot configure as a station\n\r");
			return 0;
		}

		espSendCommand("AT+CWJAP_CUR="+ssid+","+pwd+"\r\n");
		sleep(10);
		recvData = espReceiveData();
		found = recvData.find("CONNECTED");
		if(found != string::npos)
			xil_printf("Successfully connected to WIFI\n\r");
		else{
			xil_printf("Failed to connect to WIFI\n\r");
			return 0;
		}
		espSendCommand("AT+CIFSR\r\n"); //Send AT command to get the IP address of ESP
		recvData = espReceiveData();

		found = recvData.find("\"");
		if(found != string::npos){
			xil_printf("IP Address:");
			i = found+1;
			while(recvData[i]=='.' || (recvData[i]>='0' && recvData[i]<='9')){
				xil_printf("%c",recvData[i]);
				i++;
			}
			xil_printf("\n\r");
		}
		else{
			xil_printf("Failed to get IP\n\r");
			return 0;
		}
		espSendCommand("AT+CIPMUX=1\r\n");
		recvData = espReceiveData();
		found = recvData.find("OK");
		if(found != string::npos)
			xil_printf("Successfully configured\n\r");
		else{
			xil_printf("Error cannot configure MUX\n\r");
			return 0;
		}
		espSendCommand("AT+CIPSERVER=1,80\r\n");
		recvData = espReceiveData();
		found = recvData.find("OK");
		if(found != string::npos)
			xil_printf("Started HTTP Server on port 80\n\r");
		else{
			xil_printf("Failed to start server\n\r");
			return 0;
		}
		loop();
}



void loop() {
  string recvData;
  char channelNum;
  string sendData;
  int i=1;
  u32 found;
  while(1){
	  recvData = espReceiveData();//Wait for some data from ESP. ESP will have data when it receives a request over the network from a client
	  //for(u32 i=0;i<recvData.length();i++)
	  //  xil_printf("%c",recvData[i]);
	  found = recvData.find("IPD,");
	  if(found != string::npos){
		  //xil_printf("loc %d\n\r",found);
		  channelNum = recvData[found+4];
		  sendData = to_string(i);
		  espSendData(channelNum,sendData);
		  //espSendCommand("001");
		  i++;
		  //xil_printf("Channel No:%d\n\r",channelNum);
		  //xil_printf("Char %d %c\n\r",recvData[found+1],recvData[found+1]);
	  }
  }
}


void espSendCommand(string command){
	u32 numBytes = 0;
	while(numBytes != command.length()){
		uartBlockTransmitByte(&myUart,command[numBytes]);
		numBytes++;
	}
}

void espSendData(char channelNum,string sendData){
	u32 i=0;
	u32 dataSize;
	string datSize;
	dataSize = sendData.length();
	datSize = to_string(dataSize);
	espSendCommand("AT+CIPSEND="+getString(channelNum)+","+datSize+"\r\n");
	usleep(100000);
	while(i != dataSize){
		uartBlockTransmitByte(&myUart,sendData[i]);
		i++;
	}
	usleep(1000000);
	espSendCommand("AT+CIPCLOSE="+getString(channelNum)+"\r\n");
	usleep(100000);
}

string espReceiveData(){
	u8 receivedByte=1;
	string receivedData="";
	while(receivedByte != 0){
		receivedByte = uartBlockReceiveByte(&myUart);
		receivedData += receivedByte;
		xil_printf("%c",receivedByte);
	}
	return receivedData;
}

void uartBlockTransmitByte(XUartPs *myUart,u8 sendData){
	u32 transmittedBytes = 0;
	while(!transmittedBytes){
		transmittedBytes =  XUartPs_Send(myUart,&sendData,1);
	}
	//xil_printf("%d\n\r",sendData);
}

u8 uartBlockReceiveByte(XUartPs *myUart){
	u32 receivedBytes = 0;
	u8 receivedData;
	u32 timeOut = 0;
	while(!receivedBytes){
		receivedBytes =  XUartPs_Recv(myUart,&receivedData,1);
		timeOut++;
		if(timeOut==50000){
			//xil_printf("Time out!!\n\r");
			return 0;
		}
	}
	//xil_printf("%d\n\r",timeOut);
	return receivedData;
}

u8 espBlockReceiveData(){
	u8 receivedByte=0;
	u8 receivedData;
	while(!receivedByte){
		receivedByte =  XUartPs_Recv(&myUart,&receivedData,1);
		//print("Here");
	}
	return receivedData;
}

string getString(char x)
{
    string s(1, x);
    return s;
}
