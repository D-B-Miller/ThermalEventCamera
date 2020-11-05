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
#include <linux/if_link.h>
#include <string.h>
#include <algorithm>
#include <memory>
#include <unistd.h>
#include <map>
#include <math.h>
#include "headers/MLX90640_API.h"
#include "headers/MLX90640_I2C_Driver.h"

#ifndef __TEVENTCAMERA__
#define __TEVENTCAMERA__

// address of thermal camera on Pi
#define MLX_I2C_ADDR 0x33

#define IMAGE_SCALE 5

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_NONE    "\x1b[30m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define FMT_STRING "\u2588\u2588"

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
	std::chrono::time_point<std::chrono::system_clock> time; // timestamp the change was logged

	// blank constructor
	EventData(){
		this->time  = std::chrono::system_clock::now(); // set timestamp
	}
	// constructor passing
	EventData(unsigned short sig){
		this->time  = std::chrono::system_clock::now(); // set timestamp
		this->sign = sig; // set sign change
	}
};

// class for treating an MLX90640 thermal camera as an Event Camera
class ThermalEventCamera {
	public:
		signed short out[834] {0}; // output array of sign changes
		float emissivity = 1; // emissivity used in temperature conversions

		ThermalEventCamera(); // constructor that sets to 32 fps
		ThermalEventCamera(int fps); // constructor with fps argument
		~ThermalEventCamera(); // deconstructor
		// set print colors
		void setNegColor(const char* neg);
		void setPosColor(const char* pos);
		void setZeroColor(const char* neut);
		// get print colors
		const char* getNegColor(){return (const char*)this->ansi_neg_color;};
		const char* getPosColor(){return (const char*)this->ansi_pos_color;};
		const char* getZeroColor(){return (const char*)this->ansi_zero_color;};
		void read(); // read from the I2C buff
		void update(); // update the output matrix
		void start(); // start threaded reading
		void stop(); // stop threaded reading
		int getFps(); // function to get refresh FPS
		void setFps(int); // function to update refresh FPS
		void printSigns(); // print signs matrix as colors in the console
		void printFrame(); // print current frame as colors in the console
		void printSignsRaw(); // print signs matrix raw values
	private:
		int wrapperRead(); // function passed to readThread. Loops ThermalEventCamera::read
		int wrapperUpdate(); // function passed to updateThread. Loops update
		// colors used in printSigns
		char* ansi_neg_color = (char*)(ANSI_COLOR_RED FMT_STRING ANSI_COLOR_RESET);
		char* ansi_pos_color = (char*)(ANSI_COLOR_CYAN FMT_STRING ANSI_COLOR_RESET);
		char* ansi_zero_color = (char*)(ANSI_COLOR_NONE FMT_STRING ANSI_COLOR_RESET);
		bool queueReady = false; // flag set by read thread to inidcate that a frame has been processed
		uint16_t frame[834]; // raw data read from I2C bus
		uint16_t last_frame[834]; // last frame read
		uint16_t eeMLX90640[832];
		paramsMLX90640 mlx90640; // camera parameters
		std::map<size_t,EventData> events; // map containing the change data
		int fps; // fps set for camera device
		std::future<int> readThread; // thread for asynchronous reading
		std::future<int> updateThread; // thread for updating the output
		bool stopThread=false; // flag to stop the thread from running
};
#endif
