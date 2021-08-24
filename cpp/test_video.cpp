#include <opencv2/opencv.hpp>

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

std::atomic<bool> toggle(0);



const string CAM1_IP = "rtsp://192.168.0.129:554/Streaming/Channels/2/?transportmode=unicast";
const string CAM2_IP = "rtsp://192.168.0.128:554/Streaming/Channels/2/?transportmode=unicast";
const string CAM3_IP = "rtsp://192.168.0.127:554/Streaming/Channels/1/?transportmode=unicast";
const string CAM4_IP = "rtsp://192.168.0.133/12";
const string CAM5_IP = "rtsp://192.168.0.132/12";


// const string C1WINDOW = "cam-ONE";
// const string C2WINDOW = "cam-TWO";
const string C3WINDOW = "GREENBOXINNOVATIONS";

sql::Driver *driver;
const string HOST = "tcp://127.0.0.1:3306";
const string USER = "root";
const string PASSWORD = "toor";
const string DB = "pump_master";



struct vidHandle {
	string trans_string;
	int cam_no;
	bool isRecording;
};


class ThreadSafeVector {
private:
	std::mutex mu_;
	std::vector<vidHandle> myVector;

public:
	void add(const vidHandle vh) {
		std::lock_guard<std::mutex> lock(mu_);
		// std::cout << std::this_thread::get_id() << std::endl;
		myVector.push_back(vh);
	}

	void remove(const string t_string) {
		std::lock_guard<std::mutex> lock(mu_);
		
		std::vector<vidHandle>::iterator it;
		for (it = myVector.begin(); it != myVector.end(); /*++it*/) {
			// if(it->first == trans_string){
			// 	myVector.erase(it);
			// }

			if(it->trans_string == t_string){
				it = myVector.erase(it);	
			}
			else{
				++it;	
			}
		}		
	}


	int size() {
		std::lock_guard<std::mutex> lock(mu_);
		// std::cout << std::this_thread::get_id() << std::endl;
		return myVector.size();
	}

	// pass a trans_string
	// receive isRecording boolean
	bool read(const string t_string) {
		std::lock_guard<std::mutex> lock(mu_);
		// std::cout << std::this_thread::get_id() << std::endl;

		std::vector<vidHandle>::iterator it;
		for (it = myVector.begin(); it != myVector.end(); ++it) {
			if(it->trans_string == t_string){
				return it->isRecording;
			}
		}
		return false;
	}

	// check if exists
	// used for deleting video
	bool exists(const string t_string) {
		std::lock_guard<std::mutex> lock(mu_);
		// std::cout << std::this_thread::get_id() << std::endl;

		std::vector<vidHandle>::iterator it;
		for (it = myVector.begin(); it != myVector.end(); ++it) {
			if(it->trans_string == t_string){
				return true;
			}
		}
		return false;
	}

	void change(const string t_string, const bool change) {
		std::lock_guard<std::mutex> lock(mu_);
		// std::cout << std::this_thread::get_id() << std::endl;

		std::vector<vidHandle>::iterator it;
		for (it = myVector.begin(); it != myVector.end(); ++it) {
			if(it->trans_string == t_string){
				it->isRecording = change;
				return;
			}
		}
		return;
	}

	// looks for entries with cam_no
	// removes them if found
	void removeCamNo(const int cam) {
		std::lock_guard<std::mutex> lock(mu_);
		// std::cout << std::this_thread::get_id() << std::endl;

		std::vector<vidHandle>::iterator it;
		for (it = myVector.begin(); it != myVector.end();/* ++it*/) {
			// same as erase
			// but cant use because of mutex
			// no need for multiple mutexes
			// instead copy code here
			if(it->cam_no == cam){				
				it = myVector.erase(it);							
			}
			else{
				++it;	
			}
		}
		return;
	}

	
	void printVec() {
		std::lock_guard<std::mutex> lock(mu_);
		std::vector<vidHandle>::iterator it;
		for (it = myVector.begin(); it != myVector.end(); ++it) {
			cout << it->trans_string <<endl;			
		}		
	}
};



// date string stuff
time_t rawtime;
struct tm * timeinfo;
char buffer [80];


// test stuff
const int intervalMillis = 1000 * 5 * 60;


std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}


void videoClose(string file_name, string file_name_mp4){

	string cmd_f = "ffmpeg -i "+file_name+" "+file_name_mp4;
	// string cmd_f = "ffmpeg -i jiggy";

	exec(cmd_f.c_str());
	if( remove(file_name.c_str()) != 0 ){
		perror( "Error deleting file" );
	}
	else{
		cout << "File successfully deleted" << endl;				
	}
}

void videoDelete(string file_name){
	if( remove(file_name.c_str()) != 0 ){
		perror( "Error deleting file" );
	}
	else{
		cout << "File successfully deleted" << endl;
	}
}


string dateString() {
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
	string str = oss.str();

	// std::cout << str << std::endl;
	return str;
}

cv::Mat writeDateSecondary(Mat frame){

	string date = dateString();
	// just some valid rectangle arguments
	int x = 0;
	int y = 0;
	int width = 200;
	int height = 33;
	// our rectangle...
	cv::Rect rect(x, y, width, height);			
	// essentially do the same thing
	// cv::rectangle(frame, rect, cv::Scalar(0, 0, 0), CV_FILLED);
	cv::rectangle(frame, rect, cv::Scalar(0, 0, 0), FILLED);


	cv::putText(frame, //target image
		date, //text
		//cv::Point(10, clickedFrame.rows / 2), //top-left position
		cv::Point(5, 20), //top-left position
		cv::FONT_HERSHEY_DUPLEX,
		0.5,
		CV_RGB(255, 255, 255), //font color
		0.5);
	return frame;
}


// write to log file
void write_text_to_log_file( const std::string &text )
{
    std::ofstream log_file(
        "/opt/lampp/htdocs/pump_master/logs/cpp_test.log", std::ios_base::out | std::ios_base::app );
    log_file << text << std::endl;
}
// consider merging both functions
// for log timestamp
std::string getCurrentDateTime( std::string s ){
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if(s=="now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if(s=="date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
};


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
	cv::rectangle(frame, rect, cv::Scalar(0, 0, 0), FILLED);


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



void setCamStatus(string cam_no) {

	try {
		// housekeeping
		driver = get_driver_instance();
		unique_ptr<sql::Connection> con(driver->connect(HOST.c_str(), USER.c_str(), PASSWORD.c_str()));
		con->setSchema(DB.c_str());
		unique_ptr<sql::Statement> stmt(con->createStatement());

		string update_query = "UPDATE `cameras` SET `status`= 0 WHERE `cam_no` = " + cam_no;
		stmt->executeUpdate(update_query.c_str());
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}


// latest file second
void updateTransTime(const string file1,const string file2, const string trans_string){

	struct stat t_stat1;	
    stat(file1.c_str(), &t_stat1);    
    struct tm * timeinfo1 = localtime(&t_stat1.st_ctime); // or gmtime() depending on what you want
    

    // cout << "Current Day, Date and Time is = " 
    //      << asctime(timeinfo1)<< endl;

    struct stat t_stat2;
    stat(file2.c_str(), &t_stat2);
    struct tm * timeinfo2 = localtime(&t_stat2.st_ctime); // or gmtime() depending on what you want


    // cout << "Current Day, Date and Time is = " 
    //      << asctime(timeinfo2)<< endl;

    double pre_sec = std::difftime(t_stat2.st_ctime, t_stat1.st_ctime);

    int total_seconds = (int)pre_sec;

    // std::cout << "Wall time passed: "
    //           << std::difftime(t_stat2.st_ctime, t_stat1.st_ctime) << " s.\n";

    
    int  hours, minutes;
	minutes = total_seconds / 60;
	hours = minutes / 60;
	int seconds = int(total_seconds%60);	
	
	char s[25];
	sprintf(s, "%02d:%02d:%02d", hours, minutes, seconds);
	cout << s << endl;
	string update_string(s);

	try {
		// housekeeping
		driver = get_driver_instance();
		unique_ptr<sql::Connection> con(driver->connect(HOST.c_str(), USER.c_str(), PASSWORD.c_str()));
		con->setSchema(DB.c_str());
		unique_ptr<sql::Statement> stmt(con->createStatement());

		// string update_query = "UPDATE `versions` SET `name`= '"+update_string+"' WHERE `ver_id` = 1";
		string update_query = "UPDATE `transactions` SET `trans_time`= '"+update_string+"' WHERE `trans_string` = '"+trans_string+"';";
		
		stmt->executeUpdate(update_query.c_str());
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

int videoThread(const int cam_no, const string trans_string, ThreadSafeVector &tsv){
	
	Mat big_frame;
	Mat small_frame;
	Mat date_frame;
	Mat resized;	
	Size S2 = Size(640, 480);	
	int skip = 0;

	// make a date string
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime(buffer,80,"%Y-%m-%d",timeinfo);
	std::string date(buffer);				
	// make directory if not exists
	// string cmd = "mkdir -m 777 ./uploads/"+date;
	string cmd = "mkdir -p -m 777 /opt/lampp/htdocs/pump_master/videos/"+date;	
	exec("clear");
	exec(cmd.c_str());

	// make file name
	string file_name = "/opt/lampp/htdocs/pump_master/videos/"+date+"/"+trans_string+".avi";
	string file_name_mp4 = "/opt/lampp/htdocs/pump_master/videos/"+date+"/"+trans_string+".mp4";

	// VideoWriter writer = VideoWriter(file_name, CV_FOURCC('X','2','6','4'), 25, S2);
	// VideoWriter writer = VideoWriter(file_name, VideoWriter::fourcc('H','2','6','4'), 25, S2);
	
	// VideoWriter writer = VideoWriter(file_name_mp4, VideoWriter::fourcc('X','2','6','4'), 25, S2);

	// VideoWriter writer = VideoWriter(file_name_mp4, VideoWriter::fourcc('a','v','c','1'), 25, S2);
	VideoWriter writer = VideoWriter(file_name, VideoWriter::fourcc('M','J','P','G'), 25, S2);

	// dont let video record more than 20 min
	auto start = chrono::steady_clock::now();

	while(tsv.read(trans_string)){

		auto now = chrono::steady_clock::now();

		if(chrono::duration_cast<chrono::seconds>(now - start).count() > 1200) {
			tsv.change(trans_string, false);
		}


		if(cam_no == 1){
			displayFrame1.copyTo(small_frame);				
			displayFrame3.copyTo(big_frame);
		}
		else if(cam_no == 2){
			displayFrame2.copyTo(small_frame);
			displayFrame3.copyTo(big_frame);
		}
		else if(cam_no == 4){
			displayFrame4.copyTo(small_frame);
			displayFrame3.copyTo(big_frame);
		}
		else if(cam_no == 5){
			displayFrame5.copyTo(small_frame);
			displayFrame3.copyTo(big_frame);
		}
		

		

		skip++;
		if(skip == 9){
			skip = 0;
			small_frame.copyTo(big_frame(cv::Rect(1280,(1080-small_frame.rows),small_frame.cols,small_frame.rows)));			
			date_frame = writeDatePrimary(big_frame);
			cv::resize(date_frame, resized, S2);
			writer.write(resized);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}

	writer.release();
	std::this_thread::sleep_for(std::chrono::seconds(3));

	// isRecording is false
	if(tsv.exists(trans_string)){
		// process video
		videoClose(file_name,file_name_mp4);
		tsv.remove(trans_string);
	}
	else{
		// delete video
		videoDelete(file_name_mp4);
	}	
	return 0;
}



void getCamStatus(ThreadSafeVector &tsv) {

	try {
		// housekeeping
		driver = get_driver_instance();
		unique_ptr<sql::Connection> con(driver->connect(HOST.c_str(), USER.c_str(), PASSWORD.c_str()));
		con->setSchema(DB.c_str());
		unique_ptr<sql::Statement> stmt(con->createStatement());

		string query = "SELECT * FROM `cameras` WHERE `status` = 1";
		unique_ptr<sql::ResultSet> res(stmt->executeQuery(query.c_str()));
		

		if (res->rowsCount() != 0) {
			while (res->next()) {

				// cout << res->getString("cam_no") << endl;
				// cout << res->getString("type") << endl;
				// cout << res->getString("trans_string") << endl;


				try{
					// make a date string
					time (&rawtime);
					timeinfo = localtime (&rawtime);
					strftime(buffer,80,"%Y-%m-%d",timeinfo);
					std::string date(buffer);				


					// make directory if not exists
					// string cmd = "mkdir -m 777 ./uploads/"+date;
					string cmd = "mkdir -p -m 777 /opt/lampp/htdocs/pump_master/uploads/"+date;
					
					system("clear");
					system(cmd.c_str());


					// make file names
					// string file_name = "uploads/"+date+"/"+res->getString("trans_string") + "_" +res->getString("type")+".jpeg";
					// string file_name2 = "uploads/"+date+"/"+res->getString("trans_string") + "_" + res->getString("type")+"_top.jpeg";
					string file_name = "/opt/lampp/htdocs/pump_master/uploads/"+date+"/"+res->getString("trans_string") + "_" +res->getString("type")+".jpeg";
					string file_name2 = "/opt/lampp/htdocs/pump_master/uploads/"+date+"/"+res->getString("trans_string") + "_" + res->getString("type")+"_top.jpeg";


					int cam_no = std::stoi(res->getString("cam_no"));				
					string t_string = res->getString("trans_string");
					string t_type = res->getString("type");

					// select camera
					if (res->getString("cam_no") == "1")
					{						
						Mat d = writeDateSecondary(displayFrame1);
						imwrite(file_name, d );
						Mat s = writeDatePrimary(displayFrame3);
						imwrite(file_name2, s );
					}
					else if (res->getString("cam_no") == "2") {
						Mat d = writeDateSecondary(displayFrame2);
						imwrite(file_name, d );
						Mat s = writeDatePrimary(displayFrame3);
						imwrite(file_name2, s );
					}
					else if (res->getString("cam_no") == "4") {
						Mat d = writeDateSecondary(displayFrame4);
						imwrite(file_name, d );
						Mat s = writeDatePrimary(displayFrame3);
						imwrite(file_name2, s );
					}
					else if (res->getString("cam_no") == "5") {
						Mat d = writeDateSecondary(displayFrame5);
						imwrite(file_name, d );
						Mat s = writeDatePrimary(displayFrame3);
						imwrite(file_name2, s );
					}
					

					// video routing
					if(t_type == "start"){
						tsv.removeCamNo(cam_no);
						vidHandle vh = {t_string, cam_no, true};
						tsv.add(vh);
						thread vidT(videoThread, cam_no, t_string, std::ref(tsv));
						vidT.detach();
					}
					else if(t_type == "stop"){
						tsv.change(t_string, false);
						string photo_start = "/opt/lampp/htdocs/pump_master/uploads/"+date+"/"+ t_string + "_start.jpeg";
						string photo_stop = "/opt/lampp/htdocs/pump_master/uploads/"+date+"/"+ t_string + "_stop.jpeg";
						updateTransTime(photo_start, photo_stop, t_string);
					}

					// reset status in cameras
					setCamStatus(res->getString("cam_no"));

				}
				catch( const std::exception &e) {
					std::cerr << e.what();
				}

	
			}
		}
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}



void camThread(const string IP) {

	Mat pre_frame;
	Mat frame;
	VlcCap cap;
	cap.open(IP.c_str());
	// VideoCapture video(IP);

	// open and check video	
	if (!cap.isOpened()) {
		cout << "Error acquiring video" << endl;
		return;
	}
	while (1) {

		// read frame		
		try{

			if(toggle){
				toggle = false;
				std::string now = getCurrentDateTime("now");
				write_text_to_log_file(now + " restart");
				cap.release();
				// cap = VlcCap();
				cap.open(IP.c_str());
			}


			if(cap.read(pre_frame)){
				if (!pre_frame.empty()) {
					cvtColor(pre_frame, frame, COLOR_RGB2BGR);
					

					if(IP == CAM1_IP){
						frame.copyTo(displayFrame1);
						first1 = true;
					}
					else if(IP == CAM2_IP){
						frame.copyTo(displayFrame2);
						first2 = true;			
					}
					else if(IP == CAM3_IP){
						frame.copyTo(displayFrame3);
						first3 = true;			
					}
					else if(IP == CAM4_IP){
						frame.copyTo(displayFrame4);
						first4 = true;			
					}
					else if(IP == CAM5_IP){
						frame.copyTo(displayFrame5);
						first5 = true;
					}
				}else{
				std::string now = getCurrentDateTime("now");
				write_text_to_log_file(now + " cap.frame empty");	
				}	
			}else{
				std::string now = getCurrentDateTime("now");
				write_text_to_log_file(now + " cap.read false");	
			}	
		}
		catch( const std::exception &e) {
			std::cerr << e.what();
			std::string now = getCurrentDateTime("now");
			write_text_to_log_file(now + " " + e.what());
		}		
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
}




int main(int argc, char** argv) {

	cout << "ESC on window to exit" << endl;
	// namedWindow(C1WINDOW,WINDOW_NORMAL);
	// namedWindow(C2WINDOW,WINDOW_NORMAL);
	namedWindow(C3WINDOW,WINDOW_NORMAL);

	// cv::resizeWindow(C1WINDOW, 640, 480);
	// cv::resizeWindow(C2WINDOW, 640, 480);
	cv::resizeWindow(C3WINDOW, 640, 480);
	

	// // horizontal size = d1.cols + d2.cols
	// int h_size = 0;
	// // vertical size = d1.ros + d3.rows
	// int v_size = 0;

	cv::Mat comboFrame(cv::Size(1280, 834), CV_8UC3);

	


	cout << "Main start" << endl;

	thread t1(camThread, CAM1_IP);
	t1.detach();

	// thread t2(camThread, CAM2_IP);
	// t2.detach();	

	// thread t3(camThread, CAM3_IP);
	// t3.detach();

	// thread t4(camThread, CAM4_IP);
	// t4.detach();	

	// thread t5(camThread, CAM5_IP);
	// t5.detach();

	
	ThreadSafeVector tsv;

	
	while (1) {

		// if (first1 && first2 && first3 && first4 && first5) {
		// if (first4 && first5) {
		if (first1) {


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

			// displayFrame1.copyTo(comboFrame(cv::Rect(0,0,displayFrame1.cols,displayFrame1.rows)));
			// displayFrame2.copyTo(comboFrame(cv::Rect(640,0,displayFrame2.cols,displayFrame2.rows)));
			// displayFrame4.copyTo(comboFrame(cv::Rect(0,480,displayFrame4.cols,displayFrame4.rows)));
			// displayFrame5.copyTo(comboFrame(cv::Rect(640,480,displayFrame5.cols,displayFrame5.rows)));


			// imshow(C1WINDOW, displayFrame4);
			// imshow(C2WINDOW, displayFrame5);
			imshow(C3WINDOW, displayFrame1);

			//getCamStatus(std::ref(tsv));
		}

		char character = waitKey(10);
		switch (character)
		{
		case 27:			
			destroyAllWindows();
			return 0;
			break;

		case 32:
			cout << "space key" << endl;
			toggle = !toggle;
			break;


		default:
			break;
		}
	}

	return 0;
}