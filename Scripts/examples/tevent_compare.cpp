#include "teventcamera.h"

// example of custom comparison function
// compares the current pixel (c) and previous pixel (p)
// if statements are because the pixels are unsigned variables
bool thresh(uint16_t c, uint16_t p)
{
	if(c>p)
	{
		return (c-p)>10;
	}
	else if(c<p)
	{
		return (p-c)>10;
	}
	else
	{
		return false;
	}
}

int main()
{
	ThermalEventCamera cam(32); // create camera
	int tlim = 60; // time limit in seconds
	// update comparison function
	cam.setCompare(thresh);
	// check comparison flag
	std::cout << "comparison function set: " << cam.getCompareFlag() << std::endl;
	std::cin.get();
	// get start time
	auto start = std::chrono::system_clock::now();
	// start threaded update
	cam.start();
	// loop until time limit or thread error
	while(cam.isUpdateAlive(0))
	{	// print frame as colors
		cam.printSigns();
		// get elapsed time
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start);
		// if the elapsed time exceeds the limit, break from loop
		if(elapsed.count()>tlim){
			std::cout << "stopping due to time limit" << std::endl;
			cam.stop();
			return 0;
		}
	}
}
