#!/bin/bash


if [ $# -lt 1 ]
then
        echo "Usage : ./runall.sh 1|2|3|4|5|clean|submit"
        exit
fi

case "$1" in

1)  echo "---         Setting up Part 1         ---"
    rmmod -f SW
    rmmod -f KEY
    cd part1 && make clean && make
    echo "---  Running Part 1 (Press <Enter> to quit)  ---"
    echo "Running in 3..."
    sleep 1
    echo "2..."
    sleep 1
    echo "1..."
    sleep 1
    exec ./part1
    ;;
2)  echo "---         Setting up Part 2         ---"
    rmmod -f SW
    rmmod -f KEY
    cd part2 && make clean && make
    echo "---  Running Part 2 (Press <Enter> to quit)  ---"
    echo "Running in 3..."
    sleep 1
    echo "2..."
    sleep 1
    echo "1..."
    sleep 1
    exec ./part2
    ;;
3)  echo "---         Setting up Part 3         ---"
    rmmod -f SW
    rmmod -f KEY
    cd part3 && make clean && make
    echo "---  Running Part 3 (Press Ctrl-C to quit)  ---"
    echo "Running in 3..."
    sleep 1
    echo "2..."
    sleep 1
    echo "1..."
    sleep 1
    exec ./part3
    ;;
4)  echo "---         Setting up Part 4         ---"
    rmmod -f SW
    rmmod -f KEY
    cd part4 && make clean && make
    echo "---  Running Part 4 (Ctrl-C to quit)  ---"
    echo "Running in 3..."
    sleep 1
    echo "2..."
    sleep 1
    echo "1..."
    sleep 1
    exec ./part4
    ;;
5)  echo "---        Setting up Part 5          ---"
    rmmod -f SW
    rmmod -f KEY
    insmod /home/root/Linux_Libraries/drivers/KEY.ko
    insmod /home/root/Linux_Libraries/drivers/SW.ko
    cd part5 && make clean && make
    echo "---  Running Part 5 (Ctrl-C to quit)           ---"
    echo "---  Press KEY0 to increase speed              ---"
    echo "---  Press KEY1 to decrease speed              ---"
    echo "---  Press KEY2 to increase number of objects  ---"
    echo "---  Press KEY3 to decrease number of objects  ---"
    echo "---  Turn off all switches to show lines       ---"
    echo "---  Turn on any switch to hide lines          ---"
    echo "Running in 3..."
    sleep 1
    echo "2..."
    sleep 1
    echo "1..."
    sleep 1
    exec ./part5
    ;;
"clean") echo "-- Cleaning Up! --"
   rmmod SW
   rmmod KEY
   cd part1  && make clean && cd ../
   cd part2  && make clean && cd ../
   cd part3  && make clean && cd ../
   cd part4  && make clean && cd ../
   cd part5  && make clean && cd ../
   ;;
"submit") echo "---   Creating submission archive.  Please upload to Quercus!   ---"
   rmmod -f SW
   rmmod -f KEY
   cd part1 && make clean && cd ../
   cd part2 && make clean && cd ../
   cd part3 && make clean && cd ../
   cd part4 && make clean && cd ../
   cd part5 && make clean && cd ../
   tar -cjvf e5_$(date "+%Y.%m.%d-%H.%M.%S").tar.bz2 part1 part2 part3 part4 part5 include README runall.sh
   ;;
*) echo "Usage : ./runall.sh 1|2|3|4|5|clean|submit"
   ;;

esac

