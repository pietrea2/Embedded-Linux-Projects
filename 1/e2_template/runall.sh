#!/bin/bash


if [ $# -lt 1 ]
then
        echo "Usage : ./runall.sh 1 | 2 | 3 | clean | submit"
        exit
fi

case "$1" in

1)  echo "---        Setting up Part 1        ---"
    cd part1 && make clean && make
    echo ""
    echo "--- Running Part 1 (Ctrl-C to quit) ---"
    exec ./part1
    ;;
2)  echo "---          Setting up Part 2              ---"
    rmmod -f timer
    cd part2 && make clean && make
    echo ""
    echo "---     Installing part 2 timer module     ---"
    insmod timer.ko
    echo ""
    echo "--- Press KEY0 to print the time to the console ---"
    echo ""
    echo "---   To uninstall driver: ./runall.sh clean    ---"
    ;;
3)  echo "---          Setting up Part 3                  ---"
    rmmod -f timer
    rmmod -f stopwatch
    cd part3 && make clean && make
    echo ""
    echo "---      Installing part 3 stopwatch driver      ---"
    insmod stopwatch.ko
    echo ""
    echo "---  Press KEY0 to start/pause stopwatch                                  ---"
    echo "---  Press KEY1 to print the stopwatch time to the console                ---"
    echo "---  Press KEY1 to set the DD field to the value of the switches (max 99) ---"
    echo "---  Press KEY2 to set the SS field to the value of the switches (max 59) ---"
    echo "---  Press KEY3 to set the MM field to the value of the switches (max 59) ---"
    echo "---   To uninstall driver: ./runall.sh clean     ---"
    ;;
"clean") echo "---   Cleaning Up!   ---"
   rmmod -f timer
   rmmod -f stopwatch
   cd part1 && make clean && cd ../
   cd part2 && make clean && cd ../
   cd part3 && make clean && cd ../
   ;;
"submit") echo "---   Creating submission archive.  Please upload to Quercus!   ---"
   rmmod -f timer
   rmmod -f stopwatch
   cd part1 && make clean && cd ../
   cd part2 && make clean && cd ../
   cd part3 && make clean && cd ../
   tar -cjvf e2_$(date "+%Y.%m.%d-%H.%M.%S").tar.bz2 part1 part2 part3 include README runall.sh
   ;;

*) echo "Usage : ./runall.sh 1 | 2 | 3 | clean | submit"
   ;;

esac

