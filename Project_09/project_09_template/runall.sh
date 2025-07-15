#!/bin/bash


if [ $# -lt 1 ]
then
        echo "Usage : ./runall.sh 1|2|3|4|clean|submit"
        exit
fi

case "$1" in

1)  echo "---                Setting up Part 1               ---"
    cd part1 && make clean && make
    rmmod -f signal_generator
    echo "---          Running Part 1 (Ctrl-C to quit)       ---"
    exec ./part1
    ;;
2)  echo "---                Setting up Part 2               ---"
    cd part2 && make clean && make
    rmmod -f signal_generator
    echo "---            Installing Part 2 driver            ---"
    insmod signal_generator.ko
    ;;
3)  echo "---                Setting up Part 3               ---"
    cd part2 && make clean && make
    rmmod -f signal_generator
    rmmod -f video
    rmmod -f SW
    insmod /home/root/Linux_Libraries/drivers/video.ko
    insmod /home/root/Linux_Libraries/drivers/SW.ko
    echo "---            Installing Part 2 driver            ---"
    insmod signal_generator.ko
    cd ../part3 && make clean && make
    echo "---  Running Part 3 user program (Ctrl-C to quit)  ---"
    exec ./part3
    ;;
4)  echo "---              Setting up Part 4                 ---"
    cd part2 && make clean && make
    rmmod -f signal_generator
    rmmod -f video
    rmmod -f SW
    rmmod -f KEY
    insmod /home/root/Linux_Libraries/drivers/video.ko
    insmod /home/root/Linux_Libraries/drivers/SW.ko
    insmod /home/root/Linux_Libraries/drivers/KEY.ko
    echo "---            Installing Part 2 driver            ---"
    insmod signal_generator.ko
    cd ../part4 && make clean && make
    echo "---  Running Part 4 user program (Ctrl-C to quit)  ---"
    exec ./part4
    ;;
"clean") echo "-- Cleaning Up! --"
    rmmod -f signal_generator
    rmmod -f video
    rmmod -f SW
    rmmod -f KEY
    cd part1  && make clean && cd ../
    cd part2  && make clean && cd ../
    cd part3  && make clean && cd ../
    cd part4  && make clean && cd ../
    ;;
"submit") echo "---   Creating submission archive.  Please upload to Quercus!   ---"
    rmmod -f signal_generator
    rmmod -f video
    rmmod -f SW
    rmmod -f KEY
    cd part1 && make clean && cd ../
    cd part2 && make clean && cd ../
    cd part3 && make clean && cd ../
    cd part4 && make clean && cd ../
    tar -cjvf e9_$(date "+%Y.%m.%d-%H.%M.%S").tar.bz2 part1 part2 part3 part4 include physical.c README runall.sh
    ;;
*)  echo "Usage : ./runall.sh 1|2|3|4|clean|submit"
    ;;

esac

