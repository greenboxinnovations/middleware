#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <thread>
#include <atomic>
#include <mutex>
#include <time.h>     

#include <chrono>

#include <utility>
#include <vector>

#include <string>
#include <iostream>
#include <fstream>

// for date string
#include <iomanip>
#include <ctime>
#include <sstream>

// log file
#include <fstream>

#include <sys/stat.h>
#include <time.h>
#include <stdio.h>


// Vlc player
#include "VlcCap.h"


// database includes 
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

// system calls
#include <stdlib.h>

using namespace std;
using namespace cv;

cv::Mat displayFrame1;
cv::Mat displayFrame2;
cv::Mat displayFrame3;
cv::Mat displayFrame4;
cv::Mat displayFrame5;

std::atomic<bool> first1(0);
std::atomic<bool> first2(0);
std::atomic<bool> first3(0);
std::atomic<bool> first4(0);
std::atomic<bool> first5(0);

std::atomic<bool> active_transaction(0);



// const string CAM1_IP = "rtsp://192.168.0.129:554/Streaming/Channels/2/?transportmode=unicast";
// const string CAM2_IP = "rtsp://192.168.0.128:554/Streaming/Channels/2/?transportmode=unicast";
// const string CAM3_IP = "rtsp://192.168.0.127:554/Streaming/Channels/1/?transportmode=unicast";
// const string CAM4_IP = "rtsp://192.168.0.133/12";
// const string CAM5_IP = "rtsp://192.168.0.132/12";


const string OH_CAM1_IP = "rtsp://admin:admin@192.168.1.224:port/cam/realmonitor?channel=1&subtype=0";
const string OH_CAM2_IP = "rtsp://admin:admin@192.168.1.227:port/cam/realmonitor?channel=1&subtype=0";
const string OH_CAM3_IP = "rtsp://admin:admin@192.168.1.229:port/cam/realmonitor?channel=1&subtype=0";




// const string C1WINDOW = "cam-ONE";
// const string C2WINDOW = "cam-TWO";
const string C3WINDOW = "GREENBOXINNOVATIONS";

// sql::Driver *driver;
// const string HOST = "tcp://127.0.0.1:3306";
// const string USER = "root";
// const string PASSWORD = "toor";
// const string DB = "pump_master";






void camThread(const string IP) {

	Mat pre_frame;
	Mat frame;
	VlcCap cap;
	cap.open(IP.c_str());
	// VideoCapture cap(IP);
    // cap.set(cv::CAP_PROP_BUFFERSIZE, 5);


	// VideoCapture video(IP);

	// open and check video	
	if (!cap.isOpened()) {
		cout << "Error acquiring video" << endl;
		return;
	}
	while (1) {

		// read frame		
		try{
			if(cap.read(pre_frame)){
				if (!pre_frame.empty()) {
					cvtColor(pre_frame, frame, COLOR_RGB2BGR);
					

					if(IP == OH_CAM1_IP){
						frame.copyTo(displayFrame1);
						first1 = true;
					}
					else if(IP == OH_CAM2_IP){
						frame.copyTo(displayFrame2);
						first2 = true;			
					}
					else if(IP == OH_CAM3_IP){
						frame.copyTo(displayFrame3);
						first3 = true;			
					}					
				}
				else{
					//std::string now = getCurrentDateTime("now");
					//write_text_to_log_file(now + " cap.frame empty");	
				}	
			}
			else{
				// std::string now = getCurrentDateTime("now");
				// write_text_to_log_file(now + " cap.read false");	
				//std::string now = getCurrentDateTime("now");
				//write_text_to_log_file(now + "VlcCap restart");
				cap.release();				
				cap.open(IP.c_str());
			}	
		}
		catch( const std::exception &e) {
			std::cerr << e.what();
			//std::string now = getCurrentDateTime("now");
			//write_text_to_log_file(now + " " + e.what());
		}
		
		// cap >> frame;
		// cvtColor(pre_frame, frame, COLOR_RGB2BGR);

		
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
}




int main(int argc, char** argv) {

	cout << "ESC on window to exit" << endl;	
	namedWindow(C3WINDOW,WINDOW_NORMAL);	
	cv::resizeWindow(C3WINDOW, 1920, 1080);
	

	// // horizontal size = d1.cols + d2.cols
	// int h_size = 0;
	// // vertical size = d1.ros + d3.rows
	// int v_size = 0;

	cv::Mat comboFrame(cv::Size(3840, 2180), CV_8UC3);
	//cv::Mat comboFrame(cv::Size(1920, 2180), CV_8UC3);
	//cv::Mat comboFrame(cv::Size(1920, 1090), CV_8UC3);

	


	cout << "Main start" << endl;

	thread t1(camThread, OH_CAM1_IP);
	t1.detach();

	thread t2(camThread, OH_CAM2_IP);
	t2.detach();	

	thread t3(camThread, OH_CAM3_IP);
	t3.detach();

	string checkExit;
	while (1) {

		// if (first1) {	
		//if (first1 && first2) {		
		if (first1 && first2 && first3) {		


			// if(h_size == 0){
			// 	// horizontal size = d1.cols + d2.cols
			// 	h_size = displayFrame1.cols + displayFrame2.cols;
			// 	// vertical size = d1.ros + d3.rows
			// 	v_size = displayFrame1.rows + displayFrame4.rows;

			// 	cout << h_size << endl;
			// 	cout << v_size << endl;
			// 	cout << displayFrame3.type() <<endl;

			// 	// cv::Mat comboFrame(cv::Size(h_size, v_size), displayFrame3.type());

			// 	// resize(comboFrame,comboFrame,Size(h_size,v_size));
			// }

			// cout << "rows: " << displayFrame1.rows << endl;
			// cout << "cols: " << displayFrame1.cols << endl;

			displayFrame1.copyTo(comboFrame(cv::Rect(0,0,displayFrame1.cols,displayFrame1.rows)));
			displayFrame2.copyTo(comboFrame(cv::Rect(0,1090,displayFrame2.cols,displayFrame2.rows)));
			displayFrame3.copyTo(comboFrame(cv::Rect(1920,0,displayFrame3.cols,displayFrame3.rows)));
			// displayFrame3.copyTo(comboFrame(cv::Rect(640,480,displayFrame3.cols,displayFrame3.rows)));


			imshow(C3WINDOW, comboFrame);
		}

		char character = waitKey(10);
		switch (character)
		{
		case 27: {
			destroyAllWindows();
			return 0;
			break;
		}
		case 32:
			break;


		default:
			break;
		}
	}
	return 0;
}