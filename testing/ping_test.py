from tkinter import *

import subprocess
# Here, we are creating our class, Window, and inheriting from the Frame
# class. Frame is a class from the tkinter module. (see Lib/tkinter/__init__)

import os
import time

import requests
from requests.packages.urllib3.util.retry import Retry
from requests.adapters import HTTPAdapter
 
from urllib3.util.retry import Retry
import requests
from requests.adapters import HTTPAdapter
import json

# logging ping results
import datetime
import logging


class Window(Frame):

    # Define settings upon initialization. Here you can specify
    def __init__(self, master=None):
        
        # parameters that you want to send through the Frame class. 
        Frame.__init__(self, master)   

        #reference to the master widget, which is the tk window                 
        self.master = master

        #with that, we want to then run init_window, which doesn't yet exist
        self.init_window()

    #Creation of init_window
    def init_window(self):

        # changing the title of our master widget      
        self.master.title("GUI")

        # allowing the widget to take the full space of the root window
        self.pack(fill=BOTH, expand=1)

        # creating a button instance
        # quitButton = Button(self, text="Exit",command=self.client_exit)
        # startButton = Button(self, text="Exit",command=self.start_client)
        # quitButton = Button(self, text="Exit",command=self.client_exit)
        # stopButton = Button(self, text="Stop",command=self.kill_program)

        # placing the button on my window
        # quitButton.place(x=0, y=0)
        # stopButton.place(x=100, y=100)


       

    # def client_exit(self):
    #     exit()

    def kill_program(self):
        result = subprocess.run('/opt/lampp/htdocs/pump_master/program.sh kill',shell=True, stdout=subprocess.PIPE)
        print(result.stdout.decode('utf-8'))
        # exit()

def ping_camera():
    try:
        global isCamUp
        # wait time before msg is sent after ping loss
        # msg_diff = 1*60
        msg_diff = 10
        # wait time before msg is resent after fist msg
        # msg_interval = 5*60
        msg_interval = 45

        counter = 0

        ping_list = ["192.168.0.128", "192.168.0.129", "192.168.0.127", "192.168.0.133", "192.168.0.132"]

        for hostname in ping_list:
            response = os.system("ping -c 4 " + hostname)

            # file names
            file_name = "/opt/lampp/htdocs/pump_master/"+str(hostname) + ".txt"
            file_msg_name = "/opt/lampp/htdocs/pump_master/"+str(hostname) + "_msg.txt"

            # no response
            # CAM is down
            if response != 0:

                pass

            # cam is up
            # delete files if exist
            else:
                counter += 1
                
        if counter==5:
            isCamUp = 1
        else:
            isCamUp = 0

        print("counter "+str(counter))
        print("ISCAMPUP "+str(isCamUp))
        root.after(10000, ping_camera)
    except Exception as e:
        # raise e
        print(e)
    



def disable_event():
    pass
    # root.destroy()
    # exit()

# root window created. Here, that would be the only window, but
# you can later have windows within windows.
root = Tk()

root.geometry("400x300+300+300")

#creation of an instance
app = Window(root)

# time.sleep(10)


# loops here

root.after(3000, ping_camera)
# root.after(5000, check_program_status)


# root.after(10000, networkSelector)

# sync_check()
# root.after(10000, sync_check)
# root.after(12000, send_photos)
# root.after(15000, send_videos)

# root.protocol("WM_DELETE_WINDOW", disable_event)

#mainloop 
root.mainloop()