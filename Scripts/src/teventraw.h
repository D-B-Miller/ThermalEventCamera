#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <chrono.h>
#include <thread>
#include <future>

#include <linux/if_link.h>

// structure for event data on each pixel
struct EventData{
	signed short sign = 0; // sign of change, +1 for positive, -1 for negative and 0 for no change
	static std::chrono::time_point<std::chrono::system_clock> time; // timestamp the change was logged
	
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

class ThermalEventRaw{
	private:
		int fd; // file descriptor
		uint16_t out[632]; // output data
		uint16_t last_out[632]; // last frame
		std::future<void> f; // future for reading thread
		bool stopFlag = false; // flag for stopping read thread
		
	public:
		EventData events[632]; // output array of EventData
		signed short signs[632]; // output array of just 
		std::map<int,EventData> events; // map containing the change data
		
		ThermalEventRaw(); // blank constructor
		~ThermalEventRaw(); // deconstructor
		
		int openDev(); // open device
		void closeDev(); // close device
		int update(); // read from device and update output
		void updateLoop(); // function passed to thread
		void start(); // reset stop flag and start thread
		void stop(); // set stop flag
};