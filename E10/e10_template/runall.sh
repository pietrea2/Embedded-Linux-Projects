#!/bin/bash


if [ $# -lt 1 ]
then
        echo "Usage : ./runall.sh 1   <input image file name (no path)> [-d] [-v]"
        echo "          optional -d produces debug output for each stage"
        echo "          optional -v draws the input and output images on a video-out display"
        echo ""
        echo "        ./runall.sh 2|3 <input image file name (no path)>"
        echo "        ./runall.sh clean"
        echo "        ./runall.sh submit"
        exit
fi

if [ "$1" = "clean" ]
then
   echo "-- Cleaning Up! --"
   rmmod -f video
   cd part1 && make clean && cd ..
   cd part2 && make clean && cd ..
   cd part3 && make clean && cd ..
   /home/root/misc/program_fpga ./DE1_SoC_Computer.rbf
   exit
fi

if [ "$1" = "submit" ]
then
   echo "--- Creating submission archive.  Please upload to Quercus! ---"
   rmmod -f video
   /home/root/misc/program_fpga ./DE1_SoC_Computer.rbf
   cd part1 && make clean && cd ..
   cd part2 && make clean && cd ..
   cd part3 && make clean && cd ..
   tar -cjvf e10_$(date "+%Y.%m.%d-%H.%M.%S").tar.bz2 include part1 part2 part3 physical.c README runall.sh
   echo "---   Created submission archive.  Please upload to Quercus!   ---"
   exit
fi

if [[ $# -lt 2 ]]
then
        echo "Usage : ./runall.sh 1   <input image file name (no path)> [-d] [-v]"
        echo "          optional -d produces debug output for each stage"
        echo "          optional -v draws the input and output images on a video-out display"
        echo ""
        echo "        ./runall.sh 2|3 <input image file name (no path)>"
        echo "        ./runall.sh clean"
        echo "        ./runall.sh submit"
        exit
fi

FLAGS=""

if [[ $# -eq 3 ]]
then
    FLAGS=$3
fi

if [[ $# -eq 4 ]]
then
    FLAGS="$3 $4"
fi

case "$1" in

1)  echo "-- Setting up Part 1 --"
    ## configure FPGA manually if using vga emulator
    ##/home/root/misc/program_fpga ./DE1_SoC_Computer.rbf
    rmmod -f video
    insmod /home/root/Linux_Libraries/drivers/video.ko
    cd part1 && make clean && make && cd ..
    rm -Rf output_images
    mkdir output_images
    cd output_images
    echo "-- Running Part 1 --"
    echo "-- Output files appear in output_images directory --"
    echo "running command: ../part1/part1 $FLAGS ../input_images/$2"
    eval "../part1/part1 $FLAGS ../input_images/$2"
    ;;

2)  echo "-- Setting up Part 2 --"
    rmmod -f video
    /home/root/misc/program_fpga ./Edge_Detector_System.rbf
    cd part2 && make clean && make && cd ..
    rm -Rf output_images
    mkdir output_images
    cd output_images
    echo "-- Running Part 2 --"
    eval "../part2/part2 ../input_images/$2"
    ;;

3)  echo "-- Setting up Part 3 --"
    rmmod -f video
    /home/root/misc/program_fpga ./Edge_Detector_System.rbf
    cd part3 && make clean && make && cd ..
    rm -Rf output_images
    mkdir output_images
    cd output_images
    echo "-- Running Part 3 --"
    eval "../part3/part3 ../input_images/$2"
    ;;

*) echo "Usage : ./runall.sh 1 < input image file name (no path) > [-d] [-v]"
   echo "          Optional -d produces debug output for each stage"
   echo "          Optional -v draws the input and output images on a video-out display"
   echo ""
   echo "        ./runall.sh 2|3 < input image file name (no path) >"
   echo "        ./runall.sh clean"
   echo "        ./runall.sh submit"
   ;;

esac
