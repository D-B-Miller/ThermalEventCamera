# ThermalEventCamera

**NOTE: This is an early version of the software and is likely to change in the future**

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
 - [examples](Scripts/examples) : Scripts showing how to use the code in src
 
## Build Instructions
There is a [Makefile](Makefile) for this repository.

To make everything (currently just builds the examples)
```
make all
```

To make the examples
```
make examples
```

To delete built binaries and .o files
```
make clean
```

**NOTE: To build tevent_example, run [build_teventexample](build_teventexample.sh). The compilation instructions aren't currently in the Makefile**

## Scripts
 - [teventcamera.h](Scripts/src/teventcamera.h), [teventcamera.cpp](Scripts.teventcamera.cpp) : Threaded class for reading i2C bus of the camera
 - [eventthermalcamera.cpp](Scripts/src/eventthermalcamera.cpp) : Naive attempt at reading and finding the element wise difference between the current frame and the previous one.
 - [teventraw.h](Scripts/src/teventraw.h), [teventraw.cpp](Scripts/teventraw.cpp) : Threaded class to reads from camera in a raw fashion not using the i2c bus.
 - [thermalraw.py](Scripts/src/thermalraw.py) : Python version of teventraw.h for reading camera in a raw fashion.
 - [threadsafequeue.h](Scripts/src/threadsafequeue.h) : Thread-safe queue used in [teventcamera.h](Scripts/src/teventcamera.h) to hold pixel change events. Based off recipe from [here](https://codetrips.com/2020/07/26/modern-c-writing-a-thread-safe-queue/)
 
## Theory
The image below shows the standard MLX90640 reading loop using the library functions. Each labelled block represents a collection of functions constituting a major step in the process. The arrows are the information that flows between each step.
 
![](pics/mlx90640-basic-read-loop.png)

In the class ThermalEventCamera, one thread continuously reads the I2C bus, processes the raw data and pushes updates of any pixel changes to a thread-safe queue and another reads the queue and updates a easier to use output matrix of just the signs. The class is defined [here](Scripts/src/threadsafequeue.h).

![](pics/lib-read-update-loop.png)

The queue is a collection of a custom structure called EventData defined in the thermalevent camera header [here](Scripts/src/teventcamera.h#L54). 

```c++
// structure for event data on each pixel
struct EventData{
	private:
		std::chrono::time_point<std::chrono::system_clock> time; // timestamp the change was logged
	public:
		signed short sign = 0; // sign of change, +1 for positive, -1 for negative
		int idx = 0; // array idx of change, currently 1D idx
		EventData(){}; // blank constructor, not time is not set
		// constructor passing idx and sign
		EventData(int ii,unsigned short sig){
			this->time  = std::chrono::system_clock::now(); // set timestamp
			this->sign = sig; // set sign change
			this->idx = ii;// set index
		}
		// method to retrieve the timestamp of edit
		std::chrono::time_point<std::chrono::system_clock> time() const {return this->time};
};
```
The member *sign* is the direction of the change and it set to either -1 or +1 for the negative or positive change respectively. The *idx* member is the 1D index of where this change happened in the read data array. It is 1D as the read array is non-rectangular at 834 bytes so cannot be references as a 2D array. The signs matrix updated by the class is accessible through the *out* member. 

The class contains three methods for displaying this signs matrix for debug purposes. The first method [printFrame](Scripts/src/threadsafequeue.h#L331) prints the most recent data as colours in the terminal. This is based off the [test](https://github.com/pimoroni/mlx90640-library/blob/master/examples/src/test.cpp) in the MLX90640 library. It prints the colors based on the estimated temperature values. The emissivity used in the conversion is by default set to 1 but can be changed through the public attribute *emissivity*. The second method [printSigns](Scripts/src/threadsafequeue.h#L210) prints the *out* matrix of sign values as colors in the terminal. The user can set which colors are used for +1 and -1 using the [setPosColor](Scripts/src/threadsafequeue.h#L288) and [setNegColor](Scripts/src/threadsafequeue.h#L244) methods respectively. Currently supported colors are the following for ease of use:

- red (default negative)
- yellow
- none
- green
- cyan (default positive)
- blue 
- magneta

The third method [printSignsRaw](Scripts/src/threadsafequeue.h#L230) prints the raw values of the signs matrix. The idea is the user can see the raw data without having to deal with colors in case there is an issue with the colors or something.

**NOTE: As the frame data and signs matrix are non-rectangular, the print methods iterate over the respective arrays as a 26 x 32 matrix (832 entries) so it can be printed as an "image" in the console**
