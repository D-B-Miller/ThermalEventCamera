# ThermalEventCamera

Event cameras are an emerging technology similar to picture cameras which rather than reading a frame of data at a fixed rate, pixel-wise changes are read asynchronously with the time, location and sign of the change.

This repo is an attempt at converting a MLX90640 thermal camera into an event camera, asynchronously reading the data over I2C bus and updating an array describing the changes. This won't be true asynchronous as there is a max refresh rate on the I2C bus so the focus will be setting up the process of responding to changes only.

## Requirements
 - [mlx90640 library](https://github.com/pimoroni/mlx90640-library)

## Survey Paper
 - Gallego, G., Delbruck, T., Orchard, G., Bartolozzi, C., Taba, B., Censi, A., Leutenegger, S., Davison, A., Conradt, J., Daniilidis, K., Scaramuzza, D.,
[Event-based Vision: A Survey](http://rpg.ifi.uzh.ch/docs/EventVisionSurvey.pdf),
IEEE Trans. Pattern Anal. Machine Intell. (TPAMI), 2020

## Scripts
 - [teventcamera.h](Scripts/teventcamera.h), [teventcamera.cpp](Scripts.teventcamera.cpp) : Threaded class for reading i2C bus of the camera
 - [eventthermalcamera.cpp](Scripts/eventthermalcamera.cpp) : Naive attempt at reading and finding the element wise difference between the current frame and the previous one.
 
## Theory
The image below shows the standard MLX90640 reading loop using the library functions. Each labelled block represents a collection of functions constituting a major step in the process. The arrows are the information that flows between each step.
 
![](pics/mlx90640-basic-read-loop.png)

To achieve asynchronous reading, the process of getting the raw arrays is placed into a thread that runs continuously. The non-zero changes are logged in a queue/hash table for further processing.

<insert picture here>
