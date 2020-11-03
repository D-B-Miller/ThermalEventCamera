#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <algorithm>
#include <memory>
#include <unistd.h>
#include "/home/pi/mlx90640-library-master/headers/MLX90640_API.h"
#include "/home/pi/mlx90640-library-master/headers/MLX90640_I2C_Driver.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_NONE    "\x1b[30m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//#define FMT_STRING "%+06.2f "
#define FMT_STRING "\u2588\u2588"


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
    uint16_t* frameData = frameArr;
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
    MLX90640_InterpolateOutliers(frameArr, ee);
    // find difference between ee and refData
    // updates diffData
    for(int i=0;i<832;++i)
	diffData[i] = (uint16_t)std::abs(frameData - refData);
    return diffData[831];
}

uint16_t MLX90640_GetDiffFrame(uint8_t slaveAddr, uint16_t *diffData, uint16_t *refData)
{
	uint16_t frame[834];
	// get new frame
	MLX90640_GetFrameData(slaveAddr,frame);
	std::cout << "data max " << (uint16_t)*std::max_element(std::begin(frame),std::end(frame))
		<< " data min " << (uint16_t)*std::min_element(std::begin(frame),std::end(frame)) << std::endl;
	// find difference
	for(int i=0;i<834;++i)
		diffData[i] = frame[i] - refData[i];
	// update old frame to new data
	//std::copy(std::begin(frame),std::end(frame),std::begin(refData));
	std::copy(std::begin(frame),std::end(frame),refData);
	return diffData[833];
}

void printColors(uint16_t *diffData,float eTa,paramsMLX90640 *pp)
{
	static float mlx90640To[768];
	// convert difference data to temperature
	MLX90640_CalculateTo(diffData,pp,1,eTa,mlx90640To);
	// correct bad pixels
	MLX90640_BadPixelsCorrection(pp->brokenPixels, mlx90640To, 1, pp);
        MLX90640_BadPixelsCorrection(pp->outlierPixels, mlx90640To, 1, pp);
	// print as colors
	for(int x = 0; x < 32; x++){
            for(int y = 0; y < 24; y++){
                //std::cout << image[32 * y + x] << ",";
                float val = mlx90640To[32 * (23-y) + x];
                if(val > 99.99) val = 99.99;
                if(val > 32.0){
                    printf(ANSI_COLOR_MAGENTA FMT_STRING ANSI_COLOR_RESET, val);
                }
                else if(val > 29.0){
                    printf(ANSI_COLOR_RED FMT_STRING ANSI_COLOR_RESET, val);
                }
                else if (val > 26.0){
                    printf(ANSI_COLOR_YELLOW FMT_STRING ANSI_COLOR_YELLOW, val);
                }
                else if ( val > 20.0 ){
                    printf(ANSI_COLOR_NONE FMT_STRING ANSI_COLOR_RESET, val);
                }
                else if (val > 17.0) {
                    printf(ANSI_COLOR_GREEN FMT_STRING ANSI_COLOR_RESET, val);
                }
                else if (val > 10.0) {
                    printf(ANSI_COLOR_CYAN FMT_STRING ANSI_COLOR_RESET, val);
                }
                else {
                    printf(ANSI_COLOR_BLUE FMT_STRING ANSI_COLOR_RESET, val);
                }
            }
            std::cout << std::endl;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(20));
        printf("\x1b[33A");
}

int main(int argc, char *argv[]){
    static uint16_t eeMLX90640[832]; // used in getting parameters
    static uint16_t old_frame[832]; // previous frame, used in calculating difference
    uint16_t diff[832]; // difference between new data and old_frame
    // diff variables for GetDiffFrame
    uint16_t oframe[834];
    uint16_t diffFrame[834];
    float eTa = 0.0;

    int ret = 0;
    uint16_t uret = 0;
    // register signal handler for keyboard interrupt
    // on interrupt a global variable is decremented
    signal(SIGINT,keyboard_interrupt);
    std::cout << "Registered signal handler" << std::endl;
    std::cout << "initialising camera..." << std::endl;
    MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
    MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
    // set refresh rate of the camera to 32 fps
    // 64 fps generates some issues
    MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
    MLX90640_SetChessMode(MLX_I2C_ADDR);
    // parameters structure for camera
    paramsMLX90640 mlx90640;
    MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
    // set resolution
    MLX90640_SetResolution(MLX_I2C_ADDR, 0x03);
    // get parameters
    MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
    // main reading loop
    std::cout << "starting main loop" << std::endl;
    while(1){
		// check for difference between the new and old frame
		//ret = MLX90640_GetDiffData(MLX_I2C_ADDR,diff,old_frame);
		//std::cout << ret << std::endl;
		//std::cout << "getting diff data" << std::endl;
		uret = MLX90640_GetDiffFrame(MLX_I2C_ADDR,diffFrame,oframe);
		//std::cout << uret << std::endl;
		//std::cout << "max " << *std::max_element(std::begin(diffFrame),std::end(diffFrame))
		//	 << " min " << *std::min_element(std::begin(diffFrame),std::end(diffFrame)) << std::endl;
		//eTa = MLX90640_GetTa(oframe, &mlx90640);

		//printColors(diffFrame,eTa,&mlx90640);
		// check keyboard interrupt flag
		// if not set, break from loop
		if(key_inter==1){
			std::cout << "Keyboard Interrupt!" << std::endl;
			return -1;
		}
    }
}
