#include "teventcamera.h"
#include <fstream>

uint16_t findMin(uint16_t *arr,size_t n)
{
	uint16_t temp=0;
	for(size_t i=0;i<n;++i){
		if(arr[i]<temp){
			temp = arr[i];
		}
	}
	return temp;
}

uint16_t findMax(uint16_t *arr,size_t n)
{
	uint16_t temp=0;
	for(size_t i=0;i<n;++i){
		if(arr[i]>temp){
			temp=arr[i];
		}
	}
	return temp;
}

float findMean(uint16_t *arr,size_t n)
{
	uint16_t sum = 0;
	for(size_t i=0;i<n;++i){
		sum += arr[i];
	}
	return (float)sum/(float)n;
}

float findStdDev(uint16_t *arr,size_t n,float mean)
{
	float var = 0;
	for(size_t i=0;i<n;++i){
		auto sub = arr[i]-mean;
		var += (sub*sub);
	}
	var /= n;
	return sqrt(var);
}

int main(){
	ThermalEventCamera cam(32);
	int tlim = 180;
	std::ofstream statfile;
	statfile.open("Scripts/bin/event_stats.csv");
	statfile << "min,max,mean,std\n";
	// get start time of program
	auto start = std::chrono::system_clock::now();
	cam.start();
	while(cam.isUpdateAlive(0))
	{
		auto ff = cam.getFrame();
		statfile << findMin(ff,834) << ',' << findMax(ff,834);
		float mean = findMean(ff,834);
		statfile << mean << ',' << findStdDev(ff,834,mean) << '\n';
		// get elapsed time
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start);
		// if the elapsed time exceeds the limit, break from loop
		if(elapsed.count()>tlim){
			std::cout << "stopping due to time limit" << std::endl;
			cam.stop();
			statfile.close();
			return 0;
		}
	}
	statfile.close();
	return 1;
}
