#!/bin/bash


if [ $# -lt 1 ]
then
        echo "Usage : ./runall.sh 1|2|3|4|clean|submit"
        exit
fi

case "$1" in

1)  echo "---                Setting up Part 1               ---"
    cd part1 && make clean && make
    echo "---          Running Part 1 (Ctrl-C to quit)       ---"
    exec ./part1
    ;;
2)  echo "---                Setting up Part 2               ---"
    cd part2 && make clean && make
    rmmod -f accel
    echo "---     Installing Part 2 accelerometer driver     ---"
    insmod accel.ko
    echo "---  Printing accelerometer data: cat /dev/accel   ---"
    cat /dev/accel
    sleep 2
    cat /dev/accel
    sleep 2
    cat /dev/accel
    sleep 2
    echo ""
    echo "---  Running Part 2 user program (Ctrl-C to quit)  ---"
    exec ./part2
    ;;
3)  echo "---                Setting up Part 3               ---"
    cd part3 && make clean && make
    rmmod -f accel
    echo "---     Installing Part 3 accelerometer driver     ---"
    insmod accel.ko
    sleep 2
    echo "---  Printing accelerometer data: cat /dev/accel   ---"
    cat /dev/accel
    sleep 2
    cat /dev/accel
    sleep 2
    cat /dev/accel
    sleep 2
    echo "---  Printing device id (to serial terminal): echo device > /dev/accel  ---"
    echo "device" > /dev/accel
    sleep 2
    echo "---  Running Part 3 user program (Ctrl-C to quit)  ---"
    exec ./part3
    ;;
4)  echo "---              Setting up Part 4                 ---"
    cd part4 && make clean && make
    rmmod -f accel
    echo "---     Installing Part 4 accelerometer driver     ---"
    insmod accel.ko
    echo "---  Printing accelerometer data: cat /dev/accel   ---"
    cat /dev/accel
    sleep 2
    cat /dev/accel
    sleep 2
    cat /dev/accel
    sleep 2
    echo "---  Running Part 4 user program (Ctrl-C to quit)  ---"
    exec ./part4
    ;;
"clean") echo "-- Cleaning Up! --"
    rmmod -f accel
    cd part1  && make clean && cd ../
    cd part2  && make clean && cd ../
    cd part3  && make clean && cd ../
    cd part4  && make clean && cd ../
    ;;
"submit") echo "---   Creating submission archive.  Please upload to Quercus!   ---"
    rmmod -f accel
    cd part1 && make clean && cd ../
    cd part2 && make clean && cd ../
    cd part3 && make clean && cd ../
    cd part4 && make clean && cd ../
    tar -cjvf e7_$(date "+%Y.%m.%d-%H.%M.%S").tar.bz2 part1 part2 part3 part4 include README runall.sh ADXL345.c accel_wrappers.c
    ;;
*)  echo "Usage : ./runall.sh 1|2|3|4|clean|submit"
    ;;

esac

