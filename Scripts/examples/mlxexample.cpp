#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <map>
#include <algorithm>
#include <assert.h>
#include "headers/MLX90640_API.h"

#define MLX_I2C_ADDR 0x33

struct EventData{
	signed short sign = 0; // sign of change, +1 for positive, -1 for negative and 0 for no change
	std::chrono::time_point<std::chrono::system_clock> time; // timestamp the change was logged

	// blank constructor
	EventData(){
		this->time  = std::chrono::system_clock::now(); // set timestamp
	}
	// constructor passing
	EventData(signed short sig){
		this->time  = std::chrono::system_clock::now(); // set timestamp
		this->sign = sig; // set sign change
	}
};

int16_t getDiff(uint16_t first,uint16_t second)
{
	uint16_t abs_diff = (first > second) ? (first - second): (second-first);
	assert(abs_diff<=INT64_MAX);
	return (first>second)?(int16_t)abs_diff : -(int16_t)abs_diff;
}

int main(){
	int state = 0;
	static uint16_t eeMLX90640[832];
	float emissivity = 1;
	uint16_t frame[834],last[834];
	int16_t diff;
	float eTa;

	std::map<int,EventData> events;

	MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
	MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
	MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
	MLX90640_SetChessMode(MLX_I2C_ADDR);

	paramsMLX90640 mlx90640;
    	MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
    	MLX90640_ExtractParameters(eeMLX90640, &mlx90640);

    	int refresh = MLX90640_GetRefreshRate(MLX_I2C_ADDR);
    	printf("EE Dumped...\n");
	int i=0;
	signed short s = 0;
	while(1){
		state = !state;
		//printf("State: %d \n", state);
		MLX90640_GetFrameData(MLX_I2C_ADDR, frame);
		for(i=0;i<834;++i)
			diff = getDiff(frame[i],last[i]);
			s = diff==0? 0 : ((diff>0)? 1 : -1);
			if(s!=0){
				std::cout << "updating " << i << std::endl;
				events.insert(std::pair<int,EventData>(i,s));
			}
        	std::copy(std::begin(frame),std::end(frame),std::begin(last));
	}
	return 0;
}
