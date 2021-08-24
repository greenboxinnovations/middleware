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

isCamUp = 1
isStarting = 0

date_ping_file = "/opt/lampp/htdocs/pump_master/logs/start_cameras.log"


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

def writeLog(msg):
    logging.basicConfig(filename=date_ping_file,
                        filemode='a',
                        format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
                        datefmt='%Y-%m-%d %H:%M:%S',                                            
                        level=logging.DEBUG)
    logging.info(msg)

def kill_program_from_out():
    result = subprocess.run('/opt/lampp/htdocs/pump_master/program.sh kill',shell=True, stdout=subprocess.PIPE)
    print(result.stdout.decode('utf-8'))


def check_ping():
    hostname = "192.168.0.1"
    response = os.system("ping -c 1 " + hostname)
    # and then check the response...
    if response == 0:
        print("Router Active")
    else:
        writeLog("Router Error")
        kill_program_from_out()
    root.after(10000, check_ping)

def check_internet():
    hostname = "8.8.8.8"
    response = os.system("ping -c 1 " + hostname)
    # and then check the response...
    if response == 0:
        print("Internet Active")
        return True
    else:
        writeLog("Internet Down")
        return False

def networkSelector():
    result = subprocess.run('/opt/lampp/htdocs/pump_master/network.sh',shell=True, stdout=subprocess.PIPE)
    print(result.stdout.decode('utf-8'))
    root.after(60000, networkSelector)


def check_program_status():
    global isCamUp    
    result = subprocess.run('/opt/lampp/htdocs/pump_master/program.sh status',shell=True,stdout=subprocess.PIPE)

    result_formatted = result.stdout.decode('utf-8').rstrip()
    # print(result.stdout.decode('utf-8'))
    print(result_formatted)
    if(result_formatted == "program not running"):
        if(isCamUp == 1):
            print("start program")
            if(isStarting == 0):
                writeLog("starting program")
                start_program()
                # pass
        else:            
            print("Please Check the cameras")            

    else:
        if(isCamUp == 1):
            print("everything OK")            
            
        else:
            print("kill program")
            writeLog("killing program")            
            kill_program_from_out()        
    root.after(5000, check_program_status) 


def start_program():
    isStarting = 1
    time.sleep(5)
    print("starting program now")
    result = subprocess.run('/opt/lampp/htdocs/pump_master/program.sh start&',shell=True)
    isStarting = 0
    # print(result.stdout.decode('utf-8'))


def send_msg(file_msg_name, hostname):
    print("send msg")



    now = time.strftime("%H:%M", time.localtime(time.time()))
    urgent_msg = "CAMERA " +hostname+ " DOWN:- " + str(now)
    msg_url = "https://www.fast2sms.com/dev/bulk?authorization=CbSpQve5NE&sender_id=SLAUTO&message="+ urgent_msg +"&language=english&route=t&numbers=9762230207,8411815106&flash=0"
    # msg_url = "https://9gag.com"  

    try:

        s = requests.Session()
        retries = Retry(total=5,
                        backoff_factor=0.1,
                        status_forcelist=[ 500, 502, 503, 504 ])
        s.mount('http://', HTTPAdapter(max_retries=retries))
        # r = s.get('https://9gag.com/')
        r = s.get(msg_url)
        print(r.status_code)

        # write to msg_file
        with open(file_msg_name, 'a') as msg_file:
            msg_file.write(str(time.time()))

    except Exception as e:
        # raise
        print(e)
        pass
    else:
        pass
    finally:
        pass


# function required by send_fcm
def retry_session(retries, session=None, backoff_factor=0.3, status_forcelist=(500, 502, 503, 504)):
    session = session or requests.Session()
    retry = Retry(
        total=retries,
        read=retries,
        connect=retries,
        backoff_factor=backoff_factor,
        status_forcelist=status_forcelist,
        )
    adapter = HTTPAdapter(max_retries=retry)
    session.mount('http://', adapter)
    session.mount('https://', adapter)
    return session


def send_fcm(file_msg_name, hostname):

    try:
        now = time.strftime("%H:%M", time.localtime(time.time()))
        urgent_msg = "CAMERA " +hostname+ " DOWN:- " + str(now)

        title = "Camera Down"
        # message = "Different message"

        x = {
            "to": "/topics/weather",
            "data": {
                "title" : title,
                "message" : urgent_msg,
            }
        }

        json_data = json.dumps(x)


        headers={
            "Accept": "*/*",
            "Accept-Encoding": "gzip, deflate",
            "Authorization": "key=AAAASovyhrg:APA91bHMAD2X0uNzJ_ppkY7GS2M7IgVmZbaXJHvECfa86OfOml1KB5FXR5D35tjQ7lIg3Fs0imk9kzlpsI7w1UbFWUHTJjs-cNdYdkvcnZ8JnMY02cg_bBnNo5VWkkkO6TnIeWS0AXGv",
            "Cache-Control": "no-cache",
            "Connection": "keep-alive",
            "Content-Length": str(len(json_data)),
            "Content-Type": "application/json",
            "Host": "fcm.googleapis.com",       
            "cache-control": "no-cache"
        }

        endpoint = "https://fcm.googleapis.com/fcm/send"
        session = retry_session(retries=5)
        # session.post(url=endpoint, data=json.dumps(x), headers=headers)
        session.post(url=endpoint, data=json_data, headers=headers)

        # write to msg_file
        with open(file_msg_name, 'a') as msg_file:
            msg_file.write(str(time.time()))

    except Exception as e:
        print(e)
    


# def ping_camera():
#     global isCamUp
#     hostname = "192.168.0.128"
#     response = os.system("ping -c 1 " + hostname)
#     # and then check the response...
#     if response == 0:
#         print("Network Active")
#         isCamUp = 1
#     else:
#         print("Network Error")        
#         isCamUp = 0
#         # kill_program_from_out()
#     root.after(5000, ping_camera)


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
            response = os.system("ping -c 1 " + hostname)

            # file names
            file_name = "/opt/lampp/htdocs/pump_master/"+str(hostname) + ".txt"
            file_msg_name = "/opt/lampp/htdocs/pump_master/"+str(hostname) + "_msg.txt"

            # no response
            # CAM is down
            if response != 0:

                writeLog(hostname)
                
                # file exists
                if os.path.isfile(file_name):
                    # read and get time diff
                    my_file = open(file_name, "r") 
                    prev_time = float(my_file.read())
                    sec_diff = time.time() - prev_time

                    if sec_diff > msg_diff:
                        # if msg file does not exist make one
                        # and send msg
                        if not os.path.exists(file_msg_name):
                            # send_msg(file_msg_name, hostname)
                            if(check_internet()):
                                send_fcm(file_msg_name, hostname)
                                # pass
                            else:
                                 writeLog("Cannot send notification as router down")
                                # pass
                # does NOT exists
                else:
                    # create here
                    with open(file_name, 'a') as my_file:
                        my_file.write(str(time.time()))

            # cam is up
            # delete files if exist
            else:
                counter += 1
                if os.path.exists(file_name):
                    os.remove(file_name)


            # check msg file
            # if interval is greater than file delete interval
            # delete file   
            if os.path.isfile(file_msg_name):
                # read and get time diff
                my_msg_file = open(file_msg_name, "r") 
                prev_msg_time = float(my_msg_file.read())
                sec_msg_diff = time.time() - prev_msg_time

                if sec_msg_diff > msg_interval:
                    if os.path.exists(file_msg_name):
                        os.remove(file_msg_name)

        if counter==5:
            isCamUp = 1
        else:
            isCamUp = 0

        print("counter "+str(counter))
        print("ISCAMPUP "+str(isCamUp))
        root.after(10000, ping_camera)
    except Exception as e:
        print(e)
    



def send_photos():
    if check_internet():
        result = subprocess.run('/opt/lampp/bin/php /opt/lampp/htdocs/pump_master/send_photos.php',shell=True,stdout=subprocess.PIPE)
        print(result.stdout.decode('utf-8'))
    root.after(6000, send_photos)


def sync_check():
    if check_internet():
        result = subprocess.run('/opt/lampp/bin/php /opt/lampp/htdocs/pump_master/sync_check.php',shell=True,stdout=subprocess.PIPE)
        print(result.stdout.decode('utf-8'))
    root.after(5000, sync_check)

def send_videos():
    if check_internet():
        result = subprocess.run('/opt/lampp/bin/php /opt/lampp/htdocs/pump_master/send_videos.php',shell=True,stdout=subprocess.PIPE)
        print(result.stdout.decode('utf-8'))
    root.after(10000, send_videos)
    #with open("send_videos.txt", "a+") as myfile:
    #    myfile.write(result.stdout.decode('utf-8'))
    #myfile.close()
    


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

time.sleep(10)


# loops here
try:
    root.after(3000, ping_camera)
    root.after(10000, check_ping)
    root.after(5000, check_program_status)


    # root.after(10000, networkSelector)

    # sync_check()
    root.after(10000, sync_check)
    root.after(12000, send_photos)
    root.after(15000, send_videos)

    root.protocol("WM_DELETE_WINDOW", disable_event)

    #mainloop 
    root.mainloop()
except Exception as e:
    writeLog(e)
    # raise e
