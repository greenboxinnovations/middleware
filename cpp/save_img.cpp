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





int main(int argc, char** argv) {

	Mat img(500,1000, CV_8UC3, Scalar(0,0,100));

	string file_name = "/home/mhks/Desktop/template/res.jpeg";
	string file_name1 = "/opt/lampp/htdocs/middleware/uploads/2021-08-20/res1.jpeg";
	imwrite(file_name, img );
	imwrite(file_name1, img );

	return 0;
}