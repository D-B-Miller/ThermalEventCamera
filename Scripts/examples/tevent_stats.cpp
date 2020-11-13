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

uint64_t findMax(uint16_t *arr,size_t n)
{
	uint64_t temp=0;
	for(size_t i=0;i<n;++i){
		if(arr[i]>temp){
			temp=arr[i];
		}
	}
	return temp;
}

float findMean(uint16_t *arr,size_t n)
{
	uint64_t sum = 0;
	for(size_t i=0;i<n;++i){
		sum += arr[i];
	}
	return (float)sum/(float)n;
}

float findStdDev(uint16_t *arr,size_t n,float mean)
{
	float var = 0;
	for(size_t i=0;i<n;++i){
		auto sub = (float)arr[i]-mean;
		var += (sub*sub);
	}
	var /= (float)n;
	return sqrt(var);
}

int main(){
	ThermalEventCamera cam(32);
	int tlim = 120;
	std::ofstream statfile;
	statfile.open("Scripts/bin/event_stats.csv",std::ofstream::out |std::ofstream::trunc);
	statfile << "min,max,mean,std\n";
	// copy of frame to find stats on
	uint16_t ff[834] = {0};
	// get start time of program
	auto start = std::chrono::system_clock::now();
	cam.start();
	while(cam.isUpdateAlive(0))
	{
		cam.getFrame(ff);
		statfile << findMin(ff,834) << ',' << findMax(ff,834) << ',';
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
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	statfile.close();
	return 1;
}
