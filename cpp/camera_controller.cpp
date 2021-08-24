#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>

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
cv::Mat displayFrame6;
cv::Mat displayFrameOH1;
cv::Mat displayFrameOH2;
cv::Mat displayFrameOH3;

std::atomic<bool> first1(0);
std::atomic<bool> first2(0);
std::atomic<bool> first3(0);
std::atomic<bool> first4(0);
std::atomic<bool> first5(0);
std::atomic<bool> first6(0);
std::atomic<bool> first_OH_1(0);
std::atomic<bool> first_OH_2(0);
std::atomic<bool> first_OH_3(0);

std::atomic<bool> active_transaction(0);


// RTSP
const string CAM1_IP = "rtsp://admin:admin123@192.168.1.221:8554/live0.264";
const string CAM2_IP = "rtsp://admin:admin123@192.168.1.220:8554/live0.264";
const string CAM3_IP = "rtsp://admin:admin123@192.168.1.219:8554/live0.264";
const string CAM4_IP = "rtsp://admin:admin123@192.168.1.218:8554/live0.264";
const string CAM5_IP = "rtsp://admin:admin123@192.168.1.217:8554/live0.264";
const string CAM6_IP = "rtsp://admin:admin123@192.168.1.216:8554/live0.264";

const string OH_CAM1_IP = "rtsp://admin:admin@192.168.1.229:port/cam/realmonitor?channel=1&subtype=0";
const string OH_CAM2_IP = "rtsp://admin:admin@192.168.1.224:port/cam/realmonitor?channel=1&subtype=0";
const string OH_CAM3_IP = "rtsp://admin:admin@192.168.1.227:port/cam/realmonitor?channel=1&subtype=0";


// PING
const string CAM1_IP_SHORT = "192.168.1.221";
const string CAM2_IP_SHORT = "192.168.1.220";
const string CAM3_IP_SHORT = "192.168.1.219";
const string CAM4_IP_SHORT = "192.168.1.218";
const string CAM5_IP_SHORT = "192.168.1.217";
const string CAM6_IP_SHORT = "192.168.1.216";

const string OH_CAM1_IP_SHORT = "192.168.1.229";
const string OH_CAM2_IP_SHORT = "192.168.1.224";
const string OH_CAM3_IP_SHORT = "192.168.1.227";


const string C3WINDOW = "FuelCam Camera Controller";

sql::Driver *driver;
const string HOST = "tcp://127.0.0.1:3306";
const string USER = "root";
const string PASSWORD = "FuelCam@123#toor";
const string DB = "fuelcam_middleware";


struct vidHandle {
	string trans_string;
	int cam_no;
	bool isRecording;
};


class ThreadSafeVector {
private:
	std::mutex mu_;
	std::vector<vidHandle> myVector;
	bool network_lock;

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


	// change bool val
	void set_network_bool(const bool change) {
		std::lock_guard<std::mutex> lock(mu_);
		network_lock = change;
	}

	bool get_network_bool() {
		std::lock_guard<std::mutex> lock(mu_);
		return network_lock;
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

// write to log file
void write_text_to_log_file( const std::string &text )
{
    std::ofstream log_file(
        "/opt/lampp/htdocs/middleware/logs/cpp.log", std::ios_base::out | std::ios_base::app );
    log_file << text << std::endl;
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

void setCamStatusTimeOut(string cam_no) {

	try {
		// housekeeping
		driver = get_driver_instance();
		unique_ptr<sql::Connection> con(driver->connect(HOST.c_str(), USER.c_str(), PASSWORD.c_str()));
		con->setSchema(DB.c_str());
		unique_ptr<sql::Statement> stmt(con->createStatement());

		string update_query = "UPDATE `cameras` SET `status`= 1, `type` = 'stop' WHERE `cam_no` = " + cam_no;
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
	string cmd = "mkdir -p -m 777 /opt/lampp/htdocs/middleware/videos/"+date;	
	exec("clear");
	exec(cmd.c_str());

	// make file name
	// string file_name = "/opt/lampp/htdocs/pump_master/videos/"+date+"/"+trans_string+".avi";
	string file_name_mp4 = "/opt/lampp/htdocs/middleware/videos/"+date+"/"+trans_string+".mp4";

	// VideoWriter writer = VideoWriter(file_name, CV_FOURCC('X','2','6','4'), 25, S2);
	// VideoWriter writer = VideoWriter(file_name, VideoWriter::fourcc('H','2','6','4'), 25, S2);
	
	// VideoWriter writer = VideoWriter(file_name_mp4, VideoWriter::fourcc('X','2','6','4'), 25, S2);

	//VideoWriter writer = VideoWriter(file_name_mp4, VideoWriter::fourcc('a','v','c','1'), 25, S2);
	// VideoWriter writer = VideoWriter(file_name, VideoWriter::fourcc('M','J','P','G'), 25, S2);
	VideoWriter writer = VideoWriter(file_name_mp4, VideoWriter::fourcc('a','v','c','1'), 25, S2);       

	// dont let video record more than 20 min
	auto start = chrono::steady_clock::now();

	while(tsv.read(trans_string)){

		auto now = chrono::steady_clock::now();

		if(chrono::duration_cast<chrono::seconds>(now - start).count() > 1200) {
			tsv.change(trans_string, false);
			string s = std::to_string(cam_no);
			setCamStatusTimeOut(s);
		}


		if(cam_no == 1){
			displayFrame1.copyTo(small_frame);				
			displayFrameOH1.copyTo(big_frame);
		}
		else if(cam_no == 2){
			displayFrame2.copyTo(small_frame);
			displayFrameOH1.copyTo(big_frame);
		}
		else if(cam_no == 3){
			displayFrame3.copyTo(small_frame);
			displayFrameOH2.copyTo(big_frame);
		}
		else if(cam_no == 4){
			displayFrame4.copyTo(small_frame);
			displayFrameOH2.copyTo(big_frame);
		}
		else if(cam_no == 5){
			displayFrame5.copyTo(small_frame);
			displayFrameOH3.copyTo(big_frame);
		}
		else if(cam_no == 6){
			displayFrame6.copyTo(small_frame);
			displayFrameOH3.copyTo(big_frame);
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

	string cmd4 = "chmod 777 "+file_name_mp4;
	system(cmd4.c_str());

	// isRecording is false
	if(tsv.exists(trans_string)){
		// process video
		//videoClose(file_name,file_name_mp4);
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

		// string query = "SELECT * FROM `cameras` WHERE `status` = 1";
		string query = "SELECT * FROM `cameras` WHERE 1";
		unique_ptr<sql::ResultSet> res(stmt->executeQuery(query.c_str()));
		
		int start_counter = 0;
		

		if (res->rowsCount() != 0) {
			while (res->next()) {

				// cout << res->getString("cam_no") << endl;
				// cout << res->getString("type") << endl;
				// cout << res->getString("trans_string") << endl;
				int status = std::stoi(res->getString("status"));
				string tttype = res->getString("type");
				if(tttype == "start"){
					start_counter = start_counter + 1;
				}				

				if(status == 1){

					try{
						// make a date string
						time (&rawtime);
						timeinfo = localtime (&rawtime);
						strftime(buffer,80,"%Y-%m-%d",timeinfo);
						std::string date(buffer);				


						// make directory if not exists
						// string cmd = "mkdir -m 777 ./uploads/"+date;
						string cmd = "mkdir -p -m 777 /opt/lampp/htdocs/middleware/uploads/"+date;
						
						// system("clear");
						system(cmd.c_str());

						//string cmd2 = "mkdir -p -m 777 /opt/lampp/htdocs/middleware/ocr/"+date;
						
						// system("clear");
						//system(cmd2.c_str());


						// make file names
						// string file_name = "uploads/"+date+"/"+res->getString("trans_string") + "_" +res->getString("type")+".jpeg";
						// string file_name2 = "uploads/"+date+"/"+res->getString("trans_string") + "_" + res->getString("type")+"_top.jpeg";
						string file_name = "/opt/lampp/htdocs/middleware/uploads/"+date+"/"+res->getString("trans_string") + "_" +res->getString("type")+".jpeg";
						string file_name2 = "/opt/lampp/htdocs/middleware/uploads/"+date+"/"+res->getString("trans_string") + "_" + res->getString("type")+"_top.jpeg";

						// string file_name = "/home/mhks/Desktop/"+res->getString("trans_string") + "_" +res->getString("type")+".jpeg";
						// string file_name2 = "/home/mhks/Desktop/"+res->getString("trans_string") + "_" + res->getString("type")+"_top.jpeg";

						//string file_name_ocr = "/opt/lampp/htdocs/middleware/ocr/"+date+"/"+res->getString("trans_string") + "_" + res->getString("type")+"_top.jpeg";


						int cam_no = std::stoi(res->getString("cam_no"));				
						string t_string = res->getString("trans_string");
						string t_type = res->getString("type");

						// select camera
						if (res->getString("cam_no") == "1")
						{						
							Mat d = writeDateSecondary(displayFrame1);
							imwrite(file_name, d );
							Mat s = writeDatePrimary(displayFrameOH1);
							imwrite(file_name2, s );							
						}
						else if (res->getString("cam_no") == "2") {
							Mat d = writeDateSecondary(displayFrame2);
							imwrite(file_name, d );
							Mat s = writeDatePrimary(displayFrameOH1);
							imwrite(file_name2, s );							
						}
						else if (res->getString("cam_no") == "3") {
							Mat d = writeDateSecondary(displayFrame3);
							imwrite(file_name, d );
							Mat s = writeDatePrimary(displayFrameOH2);
							imwrite(file_name2, s );							
						}
						else if (res->getString("cam_no") == "4") {
							Mat d = writeDateSecondary(displayFrame4);
							imwrite(file_name, d );
							Mat s = writeDatePrimary(displayFrameOH2);
							imwrite(file_name2, s );
						}
						else if (res->getString("cam_no") == "5") {
							Mat d = writeDateSecondary(displayFrame5);
							imwrite(file_name, d );
							Mat s = writeDatePrimary(displayFrameOH3);
							imwrite(file_name2, s );
						}
						else if (res->getString("cam_no") == "6") {
							Mat d = writeDateSecondary(displayFrame6);
							imwrite(file_name, d );
							Mat s = writeDatePrimary(displayFrameOH3);
							imwrite(file_name2, s );
						}

						string cmd2 = "chmod 777 "+file_name;
						system(cmd2.c_str());
						string cmd3 = "chmod 777 "+file_name2;
						system(cmd3.c_str());


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
							string photo_start = "/opt/lampp/htdocs/middleware/uploads/"+date+"/"+ t_string + "_start.jpeg";
							string photo_stop = "/opt/lampp/htdocs/middleware/uploads/"+date+"/"+ t_string + "_stop.jpeg";
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

			if(start_counter > 0){
				active_transaction = true;
			}else{
				active_transaction = false;
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
	// VideoCapture cap(IP);
    // cap.set(cv::CAP_PROP_BUFFERSIZE, 5);


	// VideoCapture video(IP);

	// open and check video	
	if (!cap.isOpened()) {
		cout << "Error acquiring video" << endl;
		return;
	}
	while (1) {
		// shut down thread if camera is down
		if((IP == CAM1_IP) && (first1 == false)){return;}
		if((IP == CAM2_IP) && (first2 == false)){return;}
		if((IP == CAM3_IP) && (first3 == false)){return;}
		if((IP == CAM4_IP) && (first4 == false)){return;}
		if((IP == CAM5_IP) && (first5 == false)){return;}
		if((IP == CAM6_IP) && (first6 == false)){return;}

		if((IP == OH_CAM1_IP) && (first_OH_1 == false)){return;}
		if((IP == OH_CAM2_IP) && (first_OH_2 == false)){return;}
		if((IP == OH_CAM3_IP) && (first_OH_3 == false)){return;}

		// read frame		
		try{
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
					else if(IP == CAM6_IP){
						frame.copyTo(displayFrame6);
						first6 = true;
					}
					else if(IP == OH_CAM1_IP){
						frame.copyTo(displayFrameOH1);
						first_OH_1 = true;
					}
					else if(IP == OH_CAM2_IP){
						frame.copyTo(displayFrameOH2);
						first_OH_2 = true;
					}
					else if(IP == OH_CAM3_IP){
						frame.copyTo(displayFrameOH3);
						first_OH_3 = true;
					}
				}
				else{
					std::string now = getCurrentDateTime("now");
					write_text_to_log_file(now + " cap.frame empty");	
				}	
			}
			else{
				// std::string now = getCurrentDateTime("now");
				// write_text_to_log_file(now + " cap.read false");	
				std::string now = getCurrentDateTime("now");
				write_text_to_log_file(now + "VlcCap restart");

				cap.release();				
				cap.open(IP.c_str()); //retry vlcCap open
			}	
		}
		catch( const std::exception &e) {
			std::cerr << e.what();
			std::string now = getCurrentDateTime("now");
			write_text_to_log_file(now + " " + e.what());
		}
		
		// cap >> frame;
		// cvtColor(pre_frame, frame, COLOR_RGB2BGR);

		
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
}


void ping_cameras() {

	vector<string> ip_list = {	"192.168.1.216",
								"192.168.1.217",
								"192.168.1.218",
								"192.168.1.219",
								"192.168.1.220",
								"192.168.1.221",
								"192.168.1.229",
								"192.168.1.227",
								"192.168.1.224"
							};

	while(1){

		for(auto ip : ip_list) {

			string cmd = "ping -c 1 " + ip + " > /dev/null 2>&1";			
			int res = system(cmd.c_str());

			if (res==0){
				// cout << ip << " up"<< endl;
				// shut down thread if camera is down
				if((ip == CAM1_IP_SHORT) && (first1 == false)){

					cout << ip << " thread_started"<< endl;

					first1 = true;
					displayFrame1 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					thread t1(camThread, CAM1_IP);
					t1.detach();
				}
					

				if((ip == CAM2_IP_SHORT) && (first2 == false)){
					first2 = true;
					displayFrame2 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					thread t2(camThread, CAM2_IP);
					t2.detach();	
				}
										
				if((ip == CAM3_IP_SHORT) && (first3 == false)){
					first3 = true;
					displayFrame3 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					thread t3(camThread, CAM3_IP);
					t3.detach();
				}
										
				if((ip == CAM4_IP_SHORT) && (first4 == false)){
					first4 = true;
					displayFrame4 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					thread t4(camThread, CAM4_IP);
					t4.detach();	
				}
										
				if((ip == CAM5_IP_SHORT) && (first5 == false)){
					first5 = true;
					displayFrame5 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					thread t5(camThread, CAM5_IP);
					t5.detach();
				}

				if((ip == CAM6_IP_SHORT) && (first6 == false)){
					first6 = true;
					displayFrame6 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					thread t6(camThread, CAM6_IP);
					t6.detach();
				}

				// OH series
				if((ip == OH_CAM1_IP_SHORT) && (first_OH_1 == false)){
					first_OH_1 = true;
					displayFrameOH1 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);
					thread tOH1(camThread, OH_CAM1_IP);
					tOH1.detach();
				}

				if((ip == OH_CAM2_IP_SHORT) && (first_OH_2 == false)){
					first_OH_2 = true;
					displayFrameOH2 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);
					thread tOH2(camThread, OH_CAM2_IP);
					tOH2.detach();
				}

				if((ip == OH_CAM3_IP_SHORT) && (first_OH_3 == false)){
					first_OH_3 = true;
					displayFrameOH3 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);
					thread tOH3(camThread, OH_CAM3_IP);
					tOH3.detach();
				}
			}
			else{
				// cout << ip << " down"<< endl;
				if(ip == CAM1_IP_SHORT){
					displayFrame1 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					first1 = false;
				}					
				else if(ip == CAM2_IP_SHORT){
					displayFrame2 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					first2 = false;
				}					
				else if(ip == CAM3_IP_SHORT){
					displayFrame3 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					first3 = false;
				}					
				else if(ip == CAM4_IP_SHORT){
					displayFrame4 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					first4 = false;
				}					
				else if(ip == CAM5_IP_SHORT){
					displayFrame5 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					first5 = false;
				}
				else if(ip == CAM6_IP_SHORT){
					displayFrame6 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
					first6 = false;
				}
				// OH
				else if(ip == OH_CAM1_IP){
					displayFrameOH1 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);
					first_OH_1 = false;
				}
				else if(ip == OH_CAM2_IP){
					displayFrameOH2 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);
					first_OH_2 = false;
				}
				else if(ip == OH_CAM3_IP){
					displayFrameOH3 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);
					first_OH_3 = false;
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(10));
	}		
}


int main(int argc, char** argv) {

	first1 = false;
	first2 = false;
	first3 = false;
	first4 = false;
	first5 = false;
	first6 = false;
	first_OH_1 = false;
	first_OH_2 = false;
	first_OH_3 = false;

	displayFrame1 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
	displayFrame2 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
	displayFrame3 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
	displayFrame4 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
	displayFrame5 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
	displayFrame6 = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
	displayFrameOH1 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);
	displayFrameOH2 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);
	displayFrameOH3 = cv::Mat::zeros(cv::Size(1920, 1090), CV_8UC3);



	cout << "ESC on window to exit" << endl;
	
	namedWindow(C3WINDOW,WINDOW_NORMAL);	
	cv::resizeWindow(C3WINDOW, 1920, 1080);

	//1920 1090
	//480  960


	//cv::Mat comboFrame(cv::Size(1920, 2050), CV_8UC3);
	// cv::Mat comboFrame(cv::Size(2560, 1090), CV_8UC3); // single
	cv::Mat comboFrame(cv::Size(2560, 3270), CV_8UC3);

	// // ISLAND 1
	// thread t1(camThread, CAM1_IP);
	// t1.detach();
	// thread t2(camThread, CAM2_IP);
	// t2.detach();	
	// thread tOH1(camThread, OH_CAM1_IP);
	// tOH1.detach();


	// // // ISLAND 2
	// thread t3(camThread, CAM3_IP);
	// t3.detach();
	// thread t4(camThread, CAM4_IP);
	// t4.detach();	
	// thread tOH2(camThread, OH_CAM2_IP);
	// tOH2.detach();

	// // ISLAND 3
	// thread t5(camThread, CAM5_IP);
	// t5.detach();
	// thread t6(camThread, CAM6_IP);
	// t6.detach();	
	// thread tOH3(camThread, OH_CAM3_IP);
	// tOH3.detach();

	thread pc(ping_cameras);
	pc.detach();


	ThreadSafeVector tsv;


	string checkExit;
	while (1) {

		// island 1
		displayFrameOH1.copyTo(comboFrame(cv::Rect(0,0,displayFrameOH1.cols,displayFrameOH1.rows)));
		displayFrame1.copyTo(comboFrame(cv::Rect(1920,0,displayFrame1.cols,displayFrame1.rows)));
		displayFrame2.copyTo(comboFrame(cv::Rect(1920,480,displayFrame2.cols,displayFrame2.rows)));

		// island 2
		displayFrameOH2.copyTo(comboFrame(cv::Rect(0,1090,displayFrameOH2.cols,displayFrameOH2.rows)));
		displayFrame3.copyTo(comboFrame(cv::Rect(1920,1090,displayFrame3.cols,displayFrame3.rows)));
		displayFrame4.copyTo(comboFrame(cv::Rect(1920,1570,displayFrame4.cols,displayFrame4.rows)));

		// island 3
		displayFrameOH3.copyTo(comboFrame(cv::Rect(0,2180,displayFrameOH3.cols,displayFrameOH3.rows)));
		displayFrame5.copyTo(comboFrame(cv::Rect(1920,2180,displayFrame5.cols,displayFrame5.rows)));
		displayFrame6.copyTo(comboFrame(cv::Rect(1920,2660,displayFrame6.cols,displayFrame6.rows)));


		// displayFrameOH2.copyTo(comboFrame(cv::Rect(0,0,displayFrameOH2.cols,displayFrameOH2.rows)));
		// displayFrame3.copyTo(comboFrame(cv::Rect(1920,0,displayFrame3.cols,displayFrame3.rows)));
		// displayFrame4.copyTo(comboFrame(cv::Rect(1920,480,displayFrame4.cols,displayFrame4.rows)));

		// displayFrameOH3.copyTo(comboFrame(cv::Rect(0,0,displayFrameOH3.cols,displayFrameOH3.rows)));
		// displayFrame5.copyTo(comboFrame(cv::Rect(1920,0,displayFrame5.cols,displayFrame5.rows)));
		// displayFrame6.copyTo(comboFrame(cv::Rect(1920,480,displayFrame6.cols,displayFrame6.rows)));



		if(active_transaction){
			cv::putText(comboFrame, //target image
						"ACTIVE", //text
						cv::Point(10, 40), //top-left position
						cv::FONT_HERSHEY_DUPLEX,
						1.5,
						CV_RGB(255, 0, 0), //font color
						2);	
		}


		// imshow(C1WINDOW, displayFrame4);
		// imshow(C2WINDOW, displayFrame5);
		imshow(C3WINDOW, comboFrame);

		getCamStatus(std::ref(tsv));		

		char character = waitKey(10);
		switch (character)
		{
		case 27:{
			// write to log file on exit
			std::string now = getCurrentDateTime("now");
    		write_text_to_log_file(now + " PROGRAM EXIT");

    		string png_exit = "/opt/lampp/htdocs/middleware/logs/"+now+".jpeg";

    		imwrite(png_exit, comboFrame );

			destroyAllWindows();
			return 0;
			break;
		}

		case 113:{
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