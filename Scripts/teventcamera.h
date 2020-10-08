#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
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
#include <thread>

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
	unsigned short sign = 0;
	static std::chrono::time_point<std::chrono::system_clock> time;
	unsigned int loc = 0;
	uint16_t* ptr;
};

// class for treating an MLX90640 thermal camera as an Event Camera
class ThermalEventCamera {
	public:
		ThermalEventCamera(); // blank constructur, def 16 fps
		ThermalEventCamera(int fps); // constructor with fps argument
		~ThermalEventCamera();
		
		int read(); // read from the I2C buff
		void threadRead(); // function passed to readThread. Loops ThermalEventCamera::read
		int start(); // start threaded reading
		int stop(); // stop threaded reading
		int fps(); // function to get set refresh FPS
	private:
		uint16_t last_frame[832];
		uint16_t frame[834];
		EventData[832] data;
		int fps; // fps set for camera device
		paramsMLX90640 mlx90640; // camera parameters
		static uint16_t eeMLX90640[832];
		std::thread readThread; // thread for asynchronous reading
		bool stopThread=false; // flag to stop the thread from running
};