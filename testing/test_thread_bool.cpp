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

std::atomic<bool> network_switch(1);
int i = 0;


struct vidHandle {
	string trans_string;
	int cam_no;
	bool isRecording;
};

#include <shared_mutex>

class ThreadSafeVector {
private:
	std::mutex mu_;	
	std::vector<vidHandle> myVector;
	bool network_change = false;

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
	void set_network_bool(const string tag) {		
		// std::unique_lock<std::mutex> lock(mu_);
		if (mu_.try_lock()) {   // only increase if currently not locked:
			cout << "execute once" << tag << endl;
			mu_.unlock();
		}
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

void test_func(string tag, ThreadSafeVector &tsv){


    while(1){    
		tsv.set_network_bool(tag);
		std::this_thread::sleep_for(std::chrono::seconds(3));	
    }	
}


int main(int argc, char** argv) {

	ThreadSafeVector tsv;	
	// tsv.set_network_bool(true);

	thread t1(test_func, "1", std::ref(tsv));
	t1.detach();

	thread t2(test_func, "2", std::ref(tsv));
	t2.detach();

	thread t3(test_func, "3", std::ref(tsv));
	t3.detach();

	thread t4(test_func, "4", std::ref(tsv));
	t4.detach();

	thread t5(test_func, "5", std::ref(tsv));
	t5.detach();
	
	while(1){
		std::this_thread::sleep_for(std::chrono::seconds(5));					
	}
	
	
	return 0;
}