g++ -std=c++17 -L ~/mlx90640-library-master -I ~/mlx90640-library-master/ -I Scripts/src/ -c -o Scripts/src/teventcamera.o Scripts/src/teventcamera.cpp
g++ -std=c++17 -lpthread -Wall -O2 -I Scripts/src/ -I ~/mlx90640-library-master/ -L ~/mlx90640-library-master -o Scripts/bin/tevent_example Scripts/src/teventcamera.o Scripts/examples/tevent_example.cpp ~/mlx90640-library-master/libMLX90640_API.a
