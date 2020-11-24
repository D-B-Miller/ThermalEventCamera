swig -python teventcamera.i
g++ -lpthread -fPIC -std=c++17 -Wall -O2 \
	-I/home/pi/mlx90640-library-master \
	-I/usr/include/python3.7m \
	-L/home/pi/mlx90640-library-master \
	-c teventcamera.cpp /home/pi/mlx90640-library-master/libMLX90640_API.a
ld -shared teventcamera.o -o _teventcamera.so
