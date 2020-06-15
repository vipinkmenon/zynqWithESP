#Use the programme as
#python myclient.py ip_Address_of_Server_in_xx.xx.xx.xx_format

import socket
import sys
from time import sleep

#argv is a list of command-line parameters
#Elements in this list is of String type
#First parameter (argv[0]) is the name of your programme itself
#In this programme argv[1] will be the IP of ther server
ip_address = put your server's IP here #sys.argv[1] 
port_number = 80
#Go to an infinite loop
#s.settimeout(10)
while 1:
    try:
        s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)#create a socket
        s.connect((ip_address,port_number))
        s.settimeout(1)
        message = "switch=?"
        s.sendall(message.encode())
        data = s.recv(1024)#Expecting maximum 1024 bytes
        print("Server:",data.decode())
        s.close()
        sleep(0.2)
        if not "invalid" in data.decode():
            s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)#create a socket
            s.connect((ip_address,port_number))
            s.settimeout(1)
            message = "led="+data[-8:].decode()
            s.sendall(message.encode())
            data = s.recv(1024)#Expecting maximum 1024 bytes
            print("Server:",data.decode())
            s.close()
            sleep(0.2)
    except Exception as e: #If any exception happens during send the data
        print(str(e))
        s.close()
        #print("Error Connection lost")
        #break