#!/bin/bash


if [ $# -lt 1 ]
then
        echo "Usage : ./runall.sh 1 | 2 | 3 | 4 | clean | submit"
        exit
fi

case "$1" in

1)  echo "--- Setting up Part 1 ---"
    rmmod -f stopwatch
    cd part1 && make clean && make
    echo ""
    echo "---    Installing part 1 stopwatch driver     ---"
    insmod stopwatch.ko
    echo ""
    echo "--- Printing time: cat /dev/stopwatch ---"
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 1
    echo ""
    echo "--- To uninstall stopwatch driver: ./runall.sh clean ---"
    ;;
2)  echo "---          Setting up Part 2              ---"
    rmmod -f stopwatch
    cd part2 && make clean && make
    echo ""
    echo "---     Installing part 2 stopwatch driver     ---"
    insmod stopwatch.ko
    echo ""
    echo "--- Printing time: cat /dev/stopwatch ---"
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    echo ""
    echo "--- Pausing: echo stop > /dev/stopwatch ---"
    echo stop > /dev/stopwatch
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    echo ""
    echo "--- Resuming: echo run > /dev/stopwatch ---"
    echo run > /dev/stopwatch
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    echo ""
    echo "--- Turning on HEX display: echo disp > /dev/stopwatch ---"
    echo disp > /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    echo ""
    echo "--- Turning off HEX display: echo nodisp > /dev/stopwatch ---"
    echo nodisp > /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    cat /dev/stopwatch
    sleep 2
    echo ""
    echo "--- Turning on HEX display: echo disp > /dev/stopwatch ---"
    echo disp > /dev/stopwatch
    sleep 3
    echo ""
    echo "--- To uninstall stopwatch driver: ./runall.sh clean  ---"
    ;;
3)  echo "---          Setting up Part 3                  ---"
    rmmod -f stopwatch
    rmmod -f SW
    rmmod -f KEY
    rmmod -f LEDR
    cd part2 && make clean && make
    echo ""
    echo "---     Installing Part 2 stopwatch driver      ---"
    insmod stopwatch.ko
    echo ""
    echo "---     Installing built in KEY, SW, and LEDR drivers                   ---"
    echo "---     (if you prefer to use your own drivers, modify this script as   ---"
    echo "---     needed and include your compiled drivers with your submission)  ---"
    insmod /home/root/Linux_Libraries/drivers/SW.ko
    insmod /home/root/Linux_Libraries/drivers/KEY.ko
    insmod /home/root/Linux_Libraries/drivers/LEDR.ko
    cd ../part3 && make clean && make
    echo ""
    echo "---     Running Part 3 (Ctrl-C to quit)          ---"
    exec ./part3
    ;;
4)  echo "---            Setting up Part 4                 ---"
    rmmod -f stopwatch
    rmmod -f SW
    rmmod -f KEY
    rmmod -f LEDR
    cd part2 && make clean && make
    echo ""
    echo "---     Installing Part 2 stopwatch driver      ---"
    insmod stopwatch.ko
    echo ""
    echo "---     Installing built in KEY, SW, and LEDR drivers                   ---"
    echo "---     (if you prefer to use your own drivers, modify this script as   ---"
    echo "---     needed and include your compiled drivers with your submission)  ---"
    insmod /home/root/Linux_Libraries/drivers/SW.ko
    insmod /home/root/Linux_Libraries/drivers/KEY.ko
    insmod /home/root/Linux_Libraries/drivers/LEDR.ko
    cd ../part4 && make clean && make
    echo ""
    echo "---     Running Part 4 (Ctrl-C to quit)          ---"
    exec ./part4
    ;;
"clean") echo "---   Cleaning Up!   ---"
    rmmod -f stopwatch
    rmmod -f SW
    rmmod -f KEY
    rmmod -f LEDR
    cd part1 && make clean && cd ../
    cd part2 && make clean && cd ../
    cd part3 && make clean && cd ../
    cd part4 && make clean && cd ../
   ;;
"submit") echo "---   Creating submission archive.  Please upload to Quercus!   ---"
   rmmod -f stopwatch
   rmmod -f SW
   rmmod -f KEY
   rmmod -f LEDR
   cd part1 && make clean && cd ../
   cd part2 && make clean && cd ../
   cd part3 && make clean && cd ../
   cd part4 && make clean && cd ../
   tar -cjvf e4_$(date "+%Y.%m.%d-%H.%M.%S").tar.bz2 part1 part2 part3 part4 include README runall.sh
   ;;

*) echo "Usage : ./runall.sh 1 | 2 | 3 | 4 | clean | submit"
   ;;

esac

