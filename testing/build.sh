# #!/bin/bash
if [ $# -eq 0 ]
	then
		echo "No arguments supplied"
	else
		if [ -z "$2" ]
			then
				a=$(basename $1 .cpp)
				# g++ -std=c++0x $1 -o $a `pkg-config --cflags --libs opencv ` -lpthread  -lmysqlcppconn
				# ADDED VLC AND VLC CAP HELPER CLASS IN COMPILER
				g++ -g -std=c++11 $1 -o $a `pkg-config --cflags --libs opencv4 libvlc` -lpthread  -lmysqlcppconn 
				# g++ -g -std=c++11 $1 -o $a `pkg-config --cflags --libs opencv4 libvlc` -lpthread  -lmysqlcppconn 
			else
				# g++ -std=c++0x $1 -o $2 `pkg-config --cflags --libs opencv` -lpthread  -lmysqlcppconn
				g++ -g -std=c++11 $1 -o $2 `pkg-config --cflags --libs opencv4 libvlc` -lpthread  -lmysqlcppconn
		fi 	
  	# g++ -std=c++0x $1 -o app `pkg-config --cflags --libs opencv` -lpthread  -lmysqlcppconn
fi

echo Done!
