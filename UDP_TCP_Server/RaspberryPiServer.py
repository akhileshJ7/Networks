#Akhilesh Janaswamy 
# 
# Progress: LED was toggles and controls different PI's 
# LED by clicking on the button on my PI. 
# 
################ 
import socket 
import RPi.GPIO as gpio 
import time 
import _thread 

#Set LED on pI 
outputs = [6,4] 
GPIO.setup(outputs,GPIO.OUT) 
#Set Buttons on PI
inputs = [16] 
GPIO.setup(inputs,GPIO.IN) 

#Start socket
mysocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
mysocket.setsockopt(socket.SOL_SOCKET,socket.SO_BROADCAST,1) 
 
message = ("operation: discover\r\n") 
mysocket.sendto(str.encode(message),("192.168.24.255",4961)) 
 
data, addr = mysocket.recvfrom(1024) 
print('Received: ' + data.decode("utf-8") + ' From: ' +addr[0] + ' Port: ' +str(addr[1])) 
message = ("operation: capabilities\r\nname: janasawamyPi\r\nresources: 2\r\nresource1\r\ntype:lamp 1\r\nresource 2\r\ntype: switch 1\r\n") 
mysocket.sendto(str.encode(message), addr) 
 
#Recieve acknowledgement 
def recieve(): 
    while True: 
        data = tcpsocket.recv(1024) 
        msg = data.decode("utf-8") 
        checkmsg = msg.split() 
        if (checkmsg[1] == 'acknowledgement'): 
            pass 
        else: 
            if checkmsg[7] == 'ON': 
                gpio.output(6,gpio.LOW) 
            if checkmsg[7] == 'OFF': 
                gpio.output(6,gpio.HIGH) 
         
 
#send response from buttons by TCP
def button(): 
    status = 0 
    msg1 = "operation: status change\r\ntype: switch 1\r\nstate: ON\r\n" 
    msg2 = "operation: status change\r\ntype: switch 1\r\nstate: OFF\r\n" 
    while True: 
        if not gpio.input(16): 
            time.sleep(0.1) 
            while not gpio.input(16): 
                pass 
            if status == 0: 
                tcpsocket.sendto(msg1.encode('utf-8'),addr) 
                gpio.output(4,gpio.LOW) 
                status = 1 
            else: 
                tcpsocket.sendto(msg2.encode('utf-8'),addr) 
                status = 0 
                gpio.output(4,gpio.HIGH) 
     
 
 
while True: 
    mysocket.close() 
 
    tcpsocket.connect(addr) 
	#Start threads 
    _thread.start_new_thread( recieve, ()) 
    _thread.start_new_thread( button, ()) 
     
    while True: 
        pass 
     
               