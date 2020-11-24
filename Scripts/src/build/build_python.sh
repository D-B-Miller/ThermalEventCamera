swig -python teventcamera.i
g++ -lpthread -std=c++17 -Wall -O2 -I/home/pi/mlx90640-library-master/ -I../ -I/home/pi/local/include/python3.5 -L/home/pi/mlx90640-library-master -c ../teventcamera.cpp
