#include <string>
#include "xuartps.h"

//Return codes from WiFi initialization
enum returnCodes{WIFI_SUCCESS,UART_FAILED,ESP_Not_Detected,No_Station,NO_WIFI,NO_IP,MUX_ERROR,SERVER_ERROR};

typedef class espWiFi{
	private:
		 XUartPs myUart;
		 XUartPs_Config *myUartConfig;
		 int initESP(std::string,std::string);
	 	 void espSendCommand(std::string command);
	 	 u8 espBlockReceiveData();
	 	 void espSendData(char channelNum,std::string sendData);
	 	 std::string espReceiveData();
	 	 void uartBlockTransmitByte(u8 sendData);
	 	 u8 uartBlockReceiveByte();
	 	 std::string getString(char x);
	 	 char currentChannel;
	public:
	 	 //espWiFi(); //Default constructor
	 	 int initWifi(std::string,std::string);
	 	 int initWifi(int,std::string,std::string);
	 	 std::string getPacket();
	 	 std::string getPacket(std::string);
	 	 int sendPacket(std::string);
}espWiFi;
