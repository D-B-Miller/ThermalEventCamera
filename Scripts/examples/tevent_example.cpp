#include "teventcamera.h"
#include <iostream>
#include <string.h>

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
		cc->printSigns(); // print signs data
		//cc->printSignsRaw();
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
	cc->start();
	while(cc->isReadAlive(0)){
		// print frame as colors
		cc->printFrame();
		// get elapsed time
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start);
		// if the elapsed time exceeds the limit, break from loop
		if(elapsed.count()>tlim){
			std::cout << "stopping due to time limit" << std::endl;
			cc->stop();
			return;
		}
	}
	std::cout << "stopping due to read thread stopping with res: " << cc->isReadAlive(0) << std::endl;
}

void thread_update(ThermalEventCamera *cc, int tlim)
{
	if(tlim<=0){
		std::cerr << "Time limit has to be >=0!" << std::endl;
		return;
	}
	// get start time of program
	auto start = std::chrono::system_clock::now();
	cc->start();
	while(cc->isUpdateAlive(0))
	{
		// print frame as colors
		cc->printSigns();
		// get elapsed time
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start);
		// if the elapsed time exceeds the limit, break from loop
		if(elapsed.count()>tlim){
			std::cout << "stopping due to time limit" << std::endl;
			cc->stop();
			return;
		}
	}
	std::cout << "stopping due to update thread stopping with res: " << cc->isUpdateAlive(0) << std::endl;
}

int main(int argc,char* argv[]){
	// initialise camera at 32 fps
	std::cout << "starting camera..." << std::endl;
	ThermalEventCamera cam(32);
	// parse comand line arguments
	if(argc>1){
		if(std::strcmp(argv[1],"--man_read")){
			manual_read(&cam);
		}
		else if(std::strcmp(argv[1],"--man_update")){
			manual_update(&cam);
		}
		else if(std::strcmp(argv[1],"--td_read")){
			thread_read(&cam,120);
		}
		else if(std::strcmp(argv[1],"--td_update")){
			thread_update(&cam,120);
		}
	}
	else{ // if no flag was specified, run thread_update
		thread_update(&cam,120);
	}
	std::cout << "exiting" << std::endl;
}
