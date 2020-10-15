#include <stdint.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <future>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <iomanip>
#include <ctime>
#include <signal.h>
#include <linux/if_link.h>
#include <string.h>
#include <algorithm>
#include <memory>
#include <unistd.h>
#include "/home/pi/mlx90640-library-master/headers/MLX90640_API.h"

// address of thermal camera on Pi
#define MLX_I2C_ADDR 0x33

#define IMAGE_SCALE 5

// Valid frame rates are 1, 2, 4, 8, 16, 32 and 64
// The i2c baudrate is set to 1mhz to support these
#define FPS 16
#define FRAME_TIME_MICROS (1000000/FPS)

// Despite the framerate being ostensibly FPS hz
// The frame is often not ready in time
// This offset is added to the FRAME_TIME_MICROS
// to account for this.
#define OFFSET_MICROS 850

// structure for event data on each pixel
struct EventData{
	signed short sign = 0; // sign of change, +1 for positive, -1 for negative and 0 for no change
	static std::chrono::time_point<std::chrono::system_clock> time; // timestamp the change was logged
	
	// blank constructor
	EventData(){
		this->time  = std::chrono::system_clock::now(); // set timestamp
	}
	// constructor passing 
	EventData(unsigned short sig, unsigned int ll){
		this->time  = std::chrono::system_clock::now(); // set timestamp
		this->sign = sig; // set sign change
	}
};

// class for treating an MLX90640 thermal camera as an Event Camera
class ThermalEventCamera {
	public:
		ThermalEventCamera(); // blank constructur, def 16 fps
		ThermalEventCamera(int fps); // constructor with fps argument
		~ThermalEventCamera();
		
		signed short out[832]; // output array of changes
		
		void read(); // read from the I2C buff
		void update(); // update the output matrix
		int start(); // start threaded reading
		int stop(); // stop threaded reading
		int fps(); // function to get set refresh FPS
	private:
		void threadRead(); // function passed to readThread. Loops ThermalEventCamera::read
		void threadUpdate(); // function passed to updateThread. Loops update
	
		bool queueReady = false; // flag set by read thread to inidcate that a frame has been processed
		static uint16_t data[834]; // raw data read from I2C bus
		static uint16_t last_frame[832]; // last frame read
		static uint16_t frame[832]; // current frame
		paramsMLX90640 mlx90640; // camera parameters
		std::map<int,EventData> events; // map containing the change data
		int fps; // fps set for camera device
		std::future<void> readThread; // thread for asynchronous reading
		std::future<void> updateThread; // thread for updating the output
		bool stopThread=false; // flag to stop the thread from running
};