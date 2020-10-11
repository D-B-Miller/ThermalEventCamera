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
void ThermalEventCamera::start(){
	this->stopThread = false;
	// create asynchronous thread to read from I2C bus and populate behaviour
	this->readThread = std::async(std::launch::async,&ThermalEventCamera::threadRead,this);
	this->updateThread = std::async(std::launch::async,&ThermalEventCamera::threadUpdate,this);
}

// stop the threaded I2C read
void ThermalEventCamera::stop(){
	this->stopThread = True;
}

// single read of I2C buff and find element wise
void ThermalEventCamera::read(){
	uint16_t diff = 0;
	// get frame data 
	MLX90640_GetFrameData(MLX_I2C_ADDR,this->data);
	// interpolate outliers to create a valid data frame
	MLX90640_InterpolateOutliers(this->data, this->frame);
	// check for changes against last frame
	for(int i=0;i<832;++i)
	{
		diff = this->old_frame[i]-this->frame[i];
		// if difference between pixels is not zero
		// add entry to map where index is the pixel index
		if(diff!=0)
			this->events.insert(std::pair<int,EventData>(i,diff>0? 1 : -1,i));
	}
	// update last_frame with curent frame
	std::copy(std::begin(this->frame),std::end(this->frame),std::begin(this->last_frame));
}

// threaded read of I2C buff
// repeated calls of read so long as stopThreadis false
void ThermalEventCamera::threadRead(){
	while(not this->stopThread)
		this->read();
}

// update the output matrix
// clear the current matrix, query the events map for any changes and process any
void ThermalEventCamera::update();

// threaded updated based on current data
// runs so long as stopThread is True
void ThermalEventCamera::threadUpdate(){
	while(not this->stopThread)
		this->update();
}

