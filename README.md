# ThermalEventCamera

Event cameras are an emerging technology similar to picture cameras which rather than reading a frame of data at a fixed rate, pixel-wise changes are read asynchronously with the time, location and sign of the change.

This repo is an attempt at converting a MLX90640 thermal camera into an event camera, asynchronously reading the data over I2C bus and updating an array describing the changes. This won't be true asynchronous as there is a max refresh rate on the I2C bus so the focus will be setting up the process of responding to changes only.

## Requirements
 - [mlx90640 library](https://github.com/pimoroni/mlx90640-library)

## Survey Paper
 - Gallego, G., Delbruck, T., Orchard, G., Bartolozzi, C., Taba, B., Censi, A., Leutenegger, S., Davison, A., Conradt, J., Daniilidis, K., Scaramuzza, D.,
[Event-based Vision: A Survey](http://rpg.ifi.uzh.ch/docs/EventVisionSurvey.pdf),
IEEE Trans. Pattern Anal. Machine Intell. (TPAMI), 2020

## Structure
 - [bin](Scripts/bin) : All compiled scripts/ binaries and executables
 - [src](Scripts/src) : All source scripts
 - [build](Scripts/build) : All building scripts

## Scripts
 - [teventcamera.h](Scripts/src/teventcamera.h), [teventcamera.cpp](Scripts.teventcamera.cpp) : Threaded class for reading i2C bus of the camera
 - [eventthermalcamera.cpp](Scripts/src/eventthermalcamera.cpp) : Naive attempt at reading and finding the element wise difference between the current frame and the previous one.
 - [teventraw.h](Scripts/src/teventraw.h), [teventraw.cpp](Scripts/teventraw.cpp) : Threaded class to reads from camera in a raw fashion not using the i2c bus.
 - [teventraw.py](Scripts/src/teventraw.py) : Python version of teventraw.h for reading camera in a raw fashion.
 
## Theory
The image below shows the standard MLX90640 reading loop using the library functions. Each labelled block represents a collection of functions constituting a major step in the process. The arrows are the information that flows between each step.
 
![](pics/mlx90640-basic-read-loop.png)

In the class ThermalEventCamera, one thread continuously reads the I2C bus, processes the raw data and updates the hash map of pixel changes. The map is a std::map of integers and a custom structure called EventData. The integer (int) is the index in the data array. The custom structure EventData is the sign of the change (-1,0,1) and the timestamp (std::chrono::systemclock) of when it is logged. This constitutes the event data recorded by Event Cameras. This is converted to a more user friendly array by the update thread. This iterates over the map and updates the output array with the sign of the change. This output array (out) is what can be processed further can is available publically.

**NOTE: This is an early version of teh software and is likely to change in the future**

![](pics/lib-read-update-loop.png)
