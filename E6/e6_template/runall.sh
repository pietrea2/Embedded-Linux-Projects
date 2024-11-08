#!/bin/bash


if [ $# -lt 1 ]
then
        echo "Usage : ./runall.sh 1|2|3|4|5|6|clean|submit"
        exit
fi

case "$1" in

1)  echo "---           Setting up Part 1             ---"
    cd part1 && make clean && make
    echo "---     Installing Part 1 video driver      ---"
    rmmod -f video
    insmod video.ko
    echo "---  Running Part 1 (press Enter to quit)   ---"
    exec ./part1
    ;;
2)  echo "---           Setting up Part 2             ---"
    cd part2 && make clean && make
    echo "---     Installing Part 2 video driver      ---"
    rmmod -f video
    insmod video.ko
    echo "---  Running Part 2 (press Enter to quit)   ---"
    exec ./part2
    ;;
3)  echo "---           Setting up Part 3             ---"
    cd part3 && make clean && make
    echo "---     Installing Part 3 video driver      ---"
    rmmod -f video
    insmod video.ko
    echo "---    Running Part 3 (Ctrl-C to quit)      ---"
    exec ./part3
    ;;
4)  echo "---           Setting up Part 4             ---"
    cd part4 && make clean && make
    echo "---     Installing Part 4 video driver      ---"
    rmmod -f video
    insmod video.ko
    echo "---    Running Part 4 (Ctrl-C to quit)      ---"
    exec ./part4
    ;;
5)  echo "---           Setting up Part 5             ---"
    cd part4 && make clean && make video && cd ..
    cd part5 && make clean && make
    echo "---     Installing Part 4 video driver      ---"
    rmmod -f video
    rmmod -f SW
    rmmod -f KEY
    insmod ../part4/video.ko
    insmod /home/root/Linux_Libraries/drivers/SW.ko
    insmod /home/root/Linux_Libraries/drivers/KEY.ko
    echo "---  Running Part 5 (Ctrl-C to quit)           ---"
    echo "---  Press KEY0 to increase speed              ---"
    echo "---  Press KEY1 to decrease speed              ---"
    echo "---  Press KEY2 to increase number of objects  ---"
    echo "---  Press KEY3 to decrease number of objects  ---"
    echo "---  Turn off all switches to show lines       ---"
    echo "---  Turn on any switch to hide lines          ---"
    exec ./part5
    ;;
6)  echo "---            Setting up Part 6               ---"
    cd part6 && make clean && make
    rmmod -f video
    rmmod -f SW
    rmmod -f KEY
    insmod /home/root/Linux_Libraries/drivers/SW.ko
    insmod /home/root/Linux_Libraries/drivers/KEY.ko
    insmod video.ko
    echo "---  Running Part 5 (Ctrl-C to quit)           ---"
    echo "---  Press KEY0 to increase speed              ---"
    echo "---  Press KEY1 to decrease speed              ---"
    echo "---  Press KEY2 to increase number of objects  ---"
    echo "---  Press KEY3 to decrease number of objects  ---"
    echo "---  Turn off all switches to show lines       ---"
    echo "---  Turn on any switch to hide lines          ---"
    exec ./part6
    ;;
"clean") echo "---  Cleaning Up!  ---"
   rmmod -f SW
   rmmod -f KEY
   rmmod -f video
   cd part1 && make clean && cd ../
   cd part2 && make clean && cd ../
   cd part3 && make clean && cd ../
   cd part4 && make clean && cd ../
   cd part5 && make clean && cd ../
   cd part6 && make clean && cd ../
   ;;
"submit") echo "---   Creating submission archive.  Please upload to Quercus!   ---"
   rmmod -f SW
   rmmod -f KEY
   rmmod -f video
   cd part1 && make clean && cd ../
   cd part2 && make clean && cd ../
   cd part3 && make clean && cd ../
   cd part4 && make clean && cd ../
   cd part5 && make clean && cd ../
   cd part6 && make clean && cd ../
   tar -cjvf e6_$(date "+%Y.%m.%d-%H.%M.%S").tar.bz2 part1 part2 part3 part4 part5 part6 README runall.sh
   ;;
*) echo "Usage : ./runall.sh 1|2|3|4|5|6|clean|submit"
   ;;

esac

