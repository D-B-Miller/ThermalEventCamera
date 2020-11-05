#include "teventcamera.h"
#include <iostream>

// function for testing the manual read function
void manual_read(ThermalEventCamera *cc)
{
	while(1){
		cc->read(); // update frame data
		cc->printFrame(); // print framme data as colors
	}
}

void manual_update(ThermalEventCamera *cc)
{
	while(1){
		cc->read();	 // update frame data
		cc->update();	 // update sign data
		cc->printSigns(); // print sings data
	}
}

void thread_read(ThermalEventCamera *cc, int tlim)
{
	if(tlim<=0){
		std::cerr << "Time limit has to be >=0!" << std::endl;
		return;
	}
	// get start time of program
	auto start = std::chrono::system_clock::now();
	while(1){
		// print frame as colors
		cc->printFrame();
		// get elapsed time
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
		// if the elapsed time exceeds the limit, break from loop
		if(elapsed.count()>tlim){
			break;
		}
	}
	// set flag to stop threads
	cc->stop();
}

void thread_update(ThermalEventCamera *cc, int tlim)
{
	if(tlim<=0){
		std::cerr << "Time limit has to be >=0!" << std::endl;
		return;
	}
	// get start time of program
	auto start = std::chrono::system_clock::now();
	while(1){
		// print frame as colors
		cc->printSigns();
		// get elapsed time
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
		// if the elapsed time exceeds the limit, break from loop
		if(elapsed.count()>tlim){
			break;
		}
	}
	// set flag to stop threads
	cc->stop();
}

int main(int argc,char* argv[]){
	// initialise camera at 32 fps
	std::cout << "starting camera..." << std::endl;
	ThermalEventCamera cam(32);
	manual_read(&cam);
	//manual_update(&cam);
	//thread_read(&cam);
	//thread_update(&cam);
	std::cout << "exiting" << std::endl;
}
