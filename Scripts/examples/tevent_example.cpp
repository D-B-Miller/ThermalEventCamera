#include "teventcamera.h"
#include <iostream>

int main(int argc,char* argv[]){
	// initialise camera at 32 fps
	std::cout << "starting camera..." << std::endl;
	ThermalEventCamera cam(32);
	std::cout << "starting update and reading thread" << std::endl;
	cam.start();
	std::cout << "pausing to allow the threads to do something" << std::endl;
	for(int i=0;i<5;++i){
		sleep(1);
		std::cout << '.' << std::endl;
	}
	cam.printSigns();
	std::cout << std::endl;
	std::cout << "stopping threads and sleeping for 1s" << std::endl;
	cam.stop();
	sleep(1);
	std::cout << "exiting" << std::endl;
}
