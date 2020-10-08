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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <string.h>
#include <algorithm>
#include <memory>
#include <unistd.h>
#include "/home/pi/mlx90640-library-master/headers/MLX90640_API.h"

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

// flag for indicating if SIGINT is triggered
static volatile sig_atomic_t key_inter=0;

// function handler for is SIGINT is detected
void keyboard_interrupt(int)
{
    ++key_inter;
}

int MLX90640_GetDiffData(uint8_t slaveAddr, uint16_t *diffData, uint16_t *refData)
{
    static uint16_t ee[832];
    uint16_t frameArr[834];
    uint16_t* frameData = &frameArr;
    uint16_t dataReady = 1;
    uint16_t controlRegister1;
    uint16_t statusRegister;
    int error = 1;
    uint8_t cnt = 0;

    auto t_start = std::chrono::system_clock::now();
    dataReady = 0;
    while(dataReady == 0)
    {
        error = MLX90640_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;

	auto t_end = std::chrono::system_clock::now();
	auto t_elapsed = std::chrono::duration_cast<std::chrono::seconds>(t_end - t_start);
	if (t_elapsed.count() > 5) {
		printf("frameData timeout error waiting for dataReady \n");
		return -1;
	}
    } 

    while(dataReady != 0 && cnt < 5)
    {
        error = MLX90640_I2CWrite(slaveAddr, 0x8000, 0x0030);
        if(error == -1)
        {
            return error;
        }

        error = MLX90640_I2CRead(slaveAddr, 0x0400, 832, frameData); 
        if(error != 0)
        {
            printf("frameData read error \n");
            return error;
        }

        error = MLX90640_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }
        dataReady = statusRegister & 0x0008;
        cnt = cnt + 1;
    }

    if(cnt > 4)
    {
        fprintf(stderr, "cnt > 4 error \n");
        // return -8;
    }
    //printf("count: %d \n", cnt); 
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    frameData[832] = controlRegister1;
    frameData[833] = statusRegister & 0x0001;

    if(error != 0)
    {
        return error;
    }
	// interpolate outliers
	MLX90640_InterpolateOutliers(frameArr, ee)
    // find difference between ee and refData
    // updates diffData
    for(int i=0;i<832;++i)
	diffData[i] = (uint16_t)std::abs(frameData - refData);
	
    return diffData[831];    
}

int main(int argc, char *argv[]){
    static uint16_t eeMLX90640[832], old_frame[832];
    uint16_t frame[834];
    uint16_t diff[832];
    static int fps = FPS;
    int i=0;
    int ret;
    static int timelim = 0;
    static int timedel = 0;
    static long frame_time_micros = FRAME_TIME_MICROS;
    char *p;
    // flag for indicating if an error occurred
    // used with try catch so the file can be closed afterwards
    bool errorFlag=false;
    // variables for getting
    // handle argument parsing for given frame rate
    if(argc > 1){
	// convert argument to long number
        fps = strtol(argv[1], &p, 0);
	// if error occured, print error message
        if (errno !=0 || *p != '\0') {
            std::cout << "Invalid framerate" << std::endl;
            return 1;
        }

        if(argc>2){
            timelim = strtol(argv[2],&p,0);
            if(errno!=0 || *p != '\0'){
                std::cout <<  "Invalid time limit. Reverting to no limit" << std::endl;
                timelim=0;
            }
            else
            {
                if(timelim<0){
                    std::cout << "Invalid time limit Time limit cannot be negative" << std::endl;
                    return 2;
                }
            }
	    /*
	    if(argc>3){
		timedel = strtol(argv[3],&p,0);
		if(errno!=0 || *p != '\0'){
			std::cout << "Invalid delay specified. Reverting to no delay" << std::endl;
			timedel = 0;
		}
		else{
			if(timedel<0){
				std::cout << "Invalid delay, delay cannot be negative. Reverting to no delay" << std::endl;
				return 3;
			}
		   }
	    }*/
        }
        frame_time_micros = 1000000/fps;
    }
    std::cout << "FPS: " << fps
              << ", Time limit: " << ((timelim==0) ? "no limit":std::to_string(timelim)+" secs") 
              << ", Time delay: " << ((timedel==0) ? "no delay":std::to_string(timedel)+" secs")
	      << std::endl;
    if(fps==64){
        frame_time_micros = 2000;
        std::cout << "Increasing time offset to " << frame_time_micros << std::endl;
    }
    // register signal handler for keyboard interrupt
    // on interrupt a global variable is decremented
    signal(SIGINT,keyboard_interrupt);
    std::cout << "Registered signal handler" << std::endl;
	
    std::cout << "initialising camera..." << std::endl;
    MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
    MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
    // set refresh rate of the camera
    switch(fps){
        case 1:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b001);
		break;
	case 2:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b010);
		break;
	case 4:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b011);
		break;
	case 8:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b100);
		break;
	case 16:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b101);
		break;
	case 32:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
		break;
	case 64:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b111);
		break;
	default:
		fprintf(stderr, "Unsupported framerate: %d\n", fps);
		return 1;
    }
    MLX90640_SetChessMode(MLX_I2C_ADDR);
    // parameters structure for camera
    paramsMLX90640 mlx90640;
    MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
    // set resolution
    MLX90640_SetResolution(MLX_I2C_ADDR, 0x03);
    // get parameters
    MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
    // main reading loop
    while(1){
	auto start = std::chrono::system_clock::now();
	// check for difference between the new and old frame
	MLX90640_GetDiffData(MLX_I2C_ADDR,diff,old_frame);

	// update timer variables
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	// get amount of time elapsed and cast to seconds
	auto prog_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end-start_prog);
	// check keyboard interrupt flag
	// if not set, break from loop
	if(key_inter==1){
		std::cout << "Keyboard Interrupt!" << std::endl;
		return -1;
	}
	// checking if target elapsed time has passed
	// timelim of 0 means no limit
	if((timelim>0)&&(prog_elapsed.count()>timelim))
	{
		std::cout << "Time limit reached!" << std::endl;
		return 0;
	}
	// force thread to sleep to match fps
	std::this_thread::sleep_for(std::chrono::microseconds(frame_time - elapsed));
	// copy array to update array
	std::copy(std::begin(eeMLX90640),std::end(eeMLX90640),std::begin(old_frame));
    }
}
