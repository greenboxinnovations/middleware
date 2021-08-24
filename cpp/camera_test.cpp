#include "VlcCap.h"

#include <unistd.h> // for usleep
#include <iostream>


#include <opencv2/opencv.hpp>

#include <thread>
#include <atomic>

using namespace std;


#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"

// date string stuff
time_t rawtime;
struct tm * timeinfo;
char buffer [80];



string dateString() {
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
	string str = oss.str();

	// std::cout << str << std::endl;
	return str;
}

cv::Mat writeDatePrimary(Mat frame){

	string date = dateString();
	// just some valid rectangle arguments
	int x = 0;
	int y = 0;
	int width = 580;
	int height = 90;
	// our rectangle...
	cv::Rect rect(x, y, width, height);			
	// essentially do the same thing
	cv::rectangle(frame, rect, cv::Scalar(0, 0, 0), CV_FILLED);


	cv::putText(frame, //target image
		date, //text
		//cv::Point(10, clickedFrame.rows / 2), //top-left position
		cv::Point(10, 60), //top-left position
		cv::FONT_HERSHEY_DUPLEX,
		1.5,
		CV_RGB(255, 255, 255), //font color
		2.0);

	return frame;
}


int main(){

	Mat frame;
	Mat frame2;
	Mat s;
	VlcCap cap;
	VlcCap cap2;
	Size S = Size(1920, 1090);

	Size S2 = Size(640, 480);
	


	cap.open("rtsp://192.168.0.123:554/Streaming/Channels/1/?transportmode=unicast");
	cap2.open("rtsp://192.168.0.124:554/Streaming/Channels/2/?transportmode=unicast");
	// cap.open("/home/velocity/Desktop/vlc_stuff/1.mkv");

	// VideoWriter writer = VideoWriter("out.avi", CV_FOURCC('M','J','P','G'), 20, S);
	VideoWriter writer = VideoWriter("final.avi", VideoWriter::fourcc('H','2','6','4'), 10, S);

	const string WINDOW = "window";

	namedWindow(WINDOW, WINDOW_NORMAL);
	cv::resizeWindow(WINDOW, 640, 480);
	usleep(1000);
	if(cap.isOpened()){

		while(1){
			cap.read(frame);
			cap2.read(frame2);

			frame2.copyTo(frame(cv::Rect(1280,0,640, 480)));

			s = writeDatePrimary(frame);

			// cout << "Width : " << frame.size().width << endl;
			// cout << "Height: " << frame.size().height << endl;

			// cv::aruco::detectMarkers(frame, dictionary, corners, ids);

			// // if at least one marker detected 
			// if (ids.size() > 0) {
// 
			// 	aruco::drawDetectedMarkers(frame, corners, ids);
			// 	for (auto const& id : ids) {
			// 		cout << id << endl;
			// 	}								
			// }
			writer.write(s);


			imshow(WINDOW, s);



			char character = waitKey(10);
			switch(character){
				case 27:
					writer.release();
					destroyAllWindows();
					return 0;
					break;
			}

		}
	}


	return 0;
}