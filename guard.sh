#!/bin/sh
#echo $HOME
cd $HOME
#cd /app/unpay
. ~/.bash_profile
for((;;));
do
	num=`ps -ef|grep 'TransServer'|grep -v 'grep'|grep -v 'tail'|grep -v 'vi'|grep -v 'guard'|wc -l`
	#echo "$num"
	if [ $num = 0 ]
	then
		cd $HOME/TransServer
		echo "`date`:detect process disappear,ReStart......" >> ./guard.log
		setsid ./TransServer > /dev/null 2>&1 &
	fi
	
	sleep 5;
done;
