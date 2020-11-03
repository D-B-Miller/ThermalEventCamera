g++ -std=c++17 -I ~/mlx90640-library-master/ -c -o ~/Desktop/ThermalEventCamera/Scripts/src/teventcamera.o ~/Desktop/ThermalEventCamera/Scripts/src/teventcamera.cpp
g++ -std=c++17 -Wall -O2 -I ~/Desktop/ThermalEventCamera/src/ -o /home/pi/Desktop/ThermalEventCamera/Scripts/bin/tevent_example Scripts/src/teventcamera.o Scripts/examples/tevent_example.cpp
