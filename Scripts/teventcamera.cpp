#include teventcamera.h

ThermalEventCamera::ThermalEventCamera()
{
	MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
	MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
	MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
	MLX90640_SetChessMode(MLX_I2C_ADDR);
	MLX90640_DumpEE(MLX_I2C_ADDR, this->eeMLX90640);
    MLX90640_SetResolution(MLX_I2C_ADDR, 0x03);
	MLX90640_ExtractParameters(eeMLX90640, &this->mlx90640);
	// create thread object
	this->readThread(&ThermalEventCamera::threadRead,this);
	// setup i2c interface
	this->i2c.stop();
}

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
		return 1;
    }
	MLX90640_SetChessMode(MLX_I2C_ADDR);
	MLX90640_DumpEE(MLX_I2C_ADDR, this->eeMLX90640);
    MLX90640_SetResolution(MLX_I2C_ADDR, 0x03);
	MLX90640_ExtractParameters(eeMLX90640, &this->mlx90640);
	// setup i2c interface
	this->i2c.stop();
}

// deconstructor
ThermalEventCamera::~ThermalEventCamera()
{
	// set thread flag stop
	this->stopThread = True;
}

// function to get the refresh rate of the device ste on creation
int ThermalEventCamera::fps()
{
	return this->fps;
}

// start the threaded I2C read
void start(){
	this->stopThread = false;
	this->readThread.join();
}

// stop the threaded I2C read
void stop(){
	this->stopThread = True;
}

// single read of I2C buff and find element wise
void read(){
	// get frame data 
	MLX90640_GetFrameData(MLX_I2C_ADDR,this->frame);
	// interpolate outliers to create a valid data frame
	MLX90640_InterpolateOutliers(this->frame, this->eeMLX90640);
}

// threaded read of I2C buff
// repeated calls of read so long as stopThreadis false
void threadRead(){
	while(not stopThread)
		this->read();
}

