#include "teventcamera.h"
//#include "headers/MLX90640_API.h"

ThermalEventCamera::ThermalEventCamera(int fps)
{
	MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
	MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
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
		fprintf(stderr, "Unsupported framerate: %d\n", this->fps);
	}
	MLX90640_SetChessMode(MLX_I2C_ADDR);
	MLX90640_DumpEE(MLX_I2C_ADDR, this->eeMLX90640);
        MLX90640_SetResolution(MLX_I2C_ADDR, 0x03);
	MLX90640_ExtractParameters(this->eeMLX90640, &this->mlx90640);
}

// deconstructor
ThermalEventCamera::~ThermalEventCamera()
{
	// set thread flag stop
	this->stopThread = true;
	// time to wait for threads to finish
	std::chrono::milliseconds span (100);
	// status returned by threads
	std::future_status status;
	// wait for read thread to finish
	if(this->readThread.valid()){
		status = this->readThread.wait_for(span);
		if(status == std::future_status::ready){
			std::cout << "read thread stopped" << std::endl;
		}
		else if(status == std::future_status::timeout){
			std::cerr << "warning: timeout waiting for read thread to finish" << std::endl;
		}
	}
	// wait for update thread to finish
	if(this->updateThread.valid()){
		status = this->updateThread.wait_for(span);
		if(status == std::future_status::read){
			std::cout << "update thread stopped" << std::endl;
		}
		else if(status == std::future_status::timeout){
			std::cerr << "warning: timeout whilst waiting for update thread to stop" << std::endl;
		}
	}
}

// function to get the refresh rate of the device set on creation
int ThermalEventCamera::getFps()
{
	return this->fps;
}

// start the threaded I2C read
int ThermalEventCamera::start(){
	this->stopThread = false;
	// create asynchronous thread to read from I2C bus and populate behaviour
	this->readThread = std::async(std::launch::async,&ThermalEventCamera::threadRead,this);
	this->updateThread = std::async(std::launch::async,&ThermalEventCamera::threadUpdate,this);
	return 0;
}

// stop the threaded I2C read
int ThermalEventCamera::stop(){
	this->stopThread = true;
	return 0;
}

// single read of I2C buff and find element wise
void ThermalEventCamera::read(){
	// element wise difference between pixels
	uint16_t diff = 0;
	// get frame data
	MLX90640_GetFrameData(MLX_I2C_ADDR,this->data);
	// interpolate outliers to create a valid data frame
	MLX90640_InterpolateOutliers(this->data, this->frame);
	// check for changes against last frame
	for(int i=0;i<832;++i)
	{
		diff = this->last_frame[i]-this->frame[i];
		// if difference between pixels is not zero
		// add entry to map where index is the pixel index
		this->events.insert(std::pair<int,EventData>(i,diff>0? 1 : diff<0 ? -1 : 0));
	}
	// update last_frame with curent frame
	std::copy(std::begin(this->frame),std::end(this->frame),std::begin(this->last_frame));
}

// threaded read of I2C buff
// repeated calls of read so long as stopThreadis false
int ThermalEventCamera::threadRead(){
	while(not this->stopThread)
		this->read();
	return 0;
}

// update the output matrix
// clear the current matrix, query the events map for any changes and process any
void ThermalEventCamera::update(){
	// clear output matrix
	//std::fill(std::begin(this->out),std::end(this->out),0);
	// iterate over map, c++ 17
	for(auto const& [key,val] : this->events)
	{	// update non-zero entries of output matrix with sign value
		if(val.sign!=0)
			this->out[key] = val.sign;
	}
}

// threaded updated based on current data
// runs so long as stopThread is True
int ThermalEventCamera::threadUpdate(){
	while(not this->stopThread)
		this->update();
	return 0;
}

