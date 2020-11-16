rm Scripts/examples/tevent_hdf5.o
rm Scripts/bin/tevent_hdf5

h5c++ -std=c++17 -I/home/pi/mlx90640-library-master -IScripts/src/ -c -o Scripts/examples/tevent_hdf5.o Scripts/examples/tevent_hdf5.cpp
h5c++ -std=c++17 -lpthread -IScripts/src/ -I/home/pi/mlx90640-library-master/ -L/home/pi/mlx90640-library-master -o Scripts/bin/tevent_hdf5 Scripts/src/teventcamera.cpp Scripts/examples/tevent_hdf5.o /home/pi/mlx90640-library-master/libMLX90640_API.a
