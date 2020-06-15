/*
 * wifi.cc
 *
 *  Created on: Jun 13, 2020
 *      Author: VIPIN
 */
#include "wifi.h"
#include <sleep.h>


using namespace std;

int espWiFi::initESP(string ssid,string pwd){
	string recvData;
	u32 found;
	int i;
	xil_printf("Initializing Connection.");
	espSendCommand("AT\r\n");
	recvData = espReceiveData();
	sleep(1);
	found = recvData.find("OK");
	if (found == string::npos){
		xil_printf("Could not detect ESP\n\r");
		return ESP_Not_Detected;
	}
	xil_printf("..");
	espSendCommand("AT+CWMODE=1\r\n");//Send the AT command to configure the ESP as a station (server/client)
	sleep(1);
	recvData = espReceiveData();
	found = recvData.find("OK");
	if(found == string::npos){
		xil_printf("Error cannot configure as a station\n\r");
		return No_Station;
	}
	xil_printf("....");
	espSendCommand("AT+CWJAP_CUR="+string("\"")+ssid+string("\"")+","+string("\"")+pwd+string("\"")+"\r\n");
	xil_printf("\n\rConnecting to WiFi");
	for(int i=0;i<10000;i++){
		usleep(100);
		xil_printf("..");
		recvData = espReceiveData();
		found = recvData.find("OK");
		if(found != string::npos){
			break;
		}
	}
	if(found == string::npos){
		xil_printf("Failed to connect to WIFI\n\r");
		return NO_WIFI;
	}
	xil_printf("\n\r");
	espSendCommand("AT+CIFSR\r\n"); //Send AT command to get the IP address of ESP
	sleep(1);
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
		return NO_IP;
	}
	espSendCommand("AT+CIPMUX=1\r\n");
	sleep(1);
	recvData = espReceiveData();
	found = recvData.find("OK");
	if(found == string::npos){
		xil_printf("Error cannot configure MUX\n\r");
		return MUX_ERROR;
	}
	espSendCommand("AT+CIPSERVER=1,80\r\n");
	sleep(1);
	recvData = espReceiveData();
	found = recvData.find("OK");
	if(found == string::npos){
		xil_printf("Failed to start server\n\r");
		return SERVER_ERROR;
	}
	return WIFI_SUCCESS;
}

int espWiFi::initWifi(string ssid,string pwd){
	u32 status;
	myUartConfig = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
	status = XUartPs_CfgInitialize(&myUart, myUartConfig, myUartConfig->BaseAddress);
	if(status != XST_SUCCESS){
		print("Uart initialization failed...\n\r");
		return UART_FAILED;
	}
	status = initESP(ssid,pwd);
	return status;
}


int espWiFi::initWifi(int baudRate,string ssid,string pwd){
	u32 status;
	string recvData;
	myUartConfig = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
	status = XUartPs_CfgInitialize(&myUart, myUartConfig, myUartConfig->BaseAddress);
	if(status != XST_SUCCESS){
		print("Uart initialization failed...\n\r");
		return UART_FAILED;
	}

	espSendCommand("AT+UART_CUR="+to_string(baudRate)+",8,1,0,0\r\n");

	recvData = espReceiveData(); //read and discard all responses from ESP

	status = XUartPs_SetBaudRate(&myUart,baudRate);//Change the baudrate for PS UART


	if(status != XST_SUCCESS){
		print("Baud Rate Error....\n\r");
		return UART_FAILED;
	}

	status = initESP(ssid,pwd);
	return status;
}



void espWiFi::espSendCommand(string command){
	u32 numBytes = 0;
	while(numBytes != command.length()){
		//xil_printf("%c",command[numBytes]);
		uartBlockTransmitByte(command[numBytes]);
		numBytes++;
	}
}

void espWiFi::espSendData(char channelNum,string sendData){
	u32 i=0;
	u32 dataSize;
	string datSize;
	std::string recvData;
	u32 found1;
	u32 found2;
	dataSize = sendData.length();
	datSize = to_string(dataSize);
	espSendCommand("AT+CIPSEND="+getString(channelNum)+","+datSize+"\r\n");
	do{
		recvData = espReceiveData();
		found1 = recvData.find("OK");
	}while(found1==string::npos); //Wait until the send command is completed
	while(i != dataSize){
		uartBlockTransmitByte(sendData[i]);
		i++;
	}
	do{
		recvData = espReceiveData();
		found1 = recvData.find("OK");
		found2 = recvData.find("CLOSED");
	}while(found1==string::npos && found2==string::npos); //Wait either for client to close the connection or all data to be sent
	if(found2==string::npos){ //If client didn't close the connection, close it
		espSendCommand("AT+CIPCLOSE="+getString(channelNum)+"\r\n");
		do{
			recvData = espReceiveData();
			found1 = recvData.find("CLOSED");
		}while(found1==string::npos);
	}
}

string espWiFi::espReceiveData(){
	u8 receivedByte=1;
	string receivedData="";
	while(receivedByte != 0){
		receivedByte = uartBlockReceiveByte();
		receivedData += receivedByte;
		//xil_printf("%c",receivedByte);
	}
	return receivedData;
}

void espWiFi::uartBlockTransmitByte(u8 sendData){
	u32 transmittedBytes = 0;
	while(!transmittedBytes){
		transmittedBytes =  XUartPs_Send(&myUart,&sendData,1);
	}
	//xil_printf("%d\n\r",sendData);
}

u8 espWiFi::uartBlockReceiveByte(){
	u32 receivedBytes = 0;
	u8 receivedData;
	u32 timeOut = 0;
	while(!receivedBytes){
		receivedBytes =  XUartPs_Recv(&myUart,&receivedData,1);
		timeOut++;
		if(timeOut==50000){
			return 0;
		}
	}
	return receivedData;
}

u8 espWiFi::espBlockReceiveData(){
	u8 receivedByte=0;
	u8 receivedData;
	while(!receivedByte){
		receivedByte =  XUartPs_Recv(&myUart,&receivedData,1);
	}
	return receivedData;
}

string espWiFi::getString(char x)
{
    string s(1, x);
    return s;
}

string espWiFi::getPacket(){
	string recvData="";
	u32 found;
	while(1){
		recvData = espReceiveData();
		found = recvData.find("IPD,");
		if(found != string::npos){
			this->currentChannel = recvData[found+4];
			return recvData;
		}
	}
}


string espWiFi::getPacket(std::string filter){
	std::string recvData;
	u32 found;
	while(1){
		recvData = espReceiveData();
		found = recvData.find("IPD,");
		if(found != string::npos){
			this->currentChannel = recvData[found+4];
			if(recvData.find(filter))
				return recvData;
		}
	}
}

int espWiFi::sendPacket(std::string sendData){
	espSendData(this->currentChannel,sendData);
	return 0;
}
