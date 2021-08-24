#!/bin/bash
# id=`/opt/lampp/bin/mysql -uroot -ptoor -Dtimer -se "SELECT p_status FROM users WHERE name='admin'"`

# echo $id
# PATH=/home/velocity/bin:/home/velocity/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin
# DISPLAY=:0
#DISPLAY=:0 xterm

# sudo chmod 777 /dev/usb/lp0
# sudo chmod 777 /dev/usb/lp1

# if [ "$id" -eq "1" ]
# 	then
#    		echo "attempting to START program"

# 		p=$(pidof /home/velocity/jiggy/./velocity_9)
# 		if [ -z "$p" ]
# 			then
# 				echo "program not started...Starting now"
# 				#./velocity_9
# 				/home/velocity/jiggy/./velocity_9
# 				#xterm -e /home/velocity/Desktop/jiggy/./velocity_9
# 			else
# 				echo "program is already runnning"
# 		fi

# 	elif [ "$id" -eq "0" ]
# 		then
# 			echo "attempting to STOP program"

# 			#p=$(pidof ./velocity_9)
# 			p=$(pidof /home/velocity/jiggy/./velocity_9)
# 			if [ -z "$p" ]
# 				then
# 					echo "program not running..."					
# 				else
# 					echo "program is runnning at pid "$p
# 					kill -9 $p
# 			fi
#    	else
#    		echo "not equal to 1 or 0"
# fi


# echo $1
case $1 in
	"status") 
		# echo "status"
		p=$(pidof /opt/lampp/htdocs/middleware/cpp/camera_controller)
		if [ -z "$p" ]
			then
				echo "program not running"					
			else
				echo $p				
		fi
		;;
	"start") 
		echo "start"
		/opt/lampp/htdocs/middleware/cpp/camera_controller
		;;
	"kill")
		echo "kill"
		p=$(pidof /opt/lampp/htdocs/middleware/cpp/camera_controller)
		if [ -z "$p" ]
			then
				echo "program not running..."					
			else
				echo "program is runnning at pid "$p
				kill -9 $p
		fi
		;;
esac


# p=$(pidof /home/select-automobiles/Desktop/football_click/untitled)
# if [ -z "$p" ]
# 	then
# 		echo "program not running..."					
# 	else
# 		echo "program is runnning at pid "$p
# 		kill -9 $p
# fi
