#include "teventcamera.h"

ThermalEventCamera::ThermalEventCamera(int fps)
{
	MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
	MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
	switch(fps){
        case 1:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b001);
		this->fps = 1;
		break;
	case 2:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b010);
		this->fps = 2;
		break;
	case 4:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b011);
		this->fps = 4;
		break;
	case 8:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b100);
		this->fps = 8;
		break;
	case 16:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b101);
		this->fps = 16;
		break;
	case 32:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
		this->fps = 32;
		break;
	case 64:
		MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b111);
		this->fps = 64;
		break;
	default:
		std::cerr << "Unsupported framerate: " << std::endl;
	}
	MLX90640_SetChessMode(MLX_I2C_ADDR);
	MLX90640_DumpEE(MLX_I2C_ADDR, this->eeMLX90640);
        MLX90640_SetResolution(MLX_I2C_ADDR, 0x03);
	MLX90640_ExtractParameters(this->eeMLX90640, &this->mlx90640);
}

ThermalEventCamera::ThermalEventCamera()
{
	MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
	MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
	MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
	this->fps = 32;
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
		if(status == std::future_status::ready){
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
void ThermalEventCamera::start(){
	this->stopThread = false;
	// create asynchronous thread to read from I2C bus and populate behaviour
	this->readThread = std::async(std::launch::async,&ThermalEventCamera::wrapperRead,this);
	this->updateThread = std::async(std::launch::async,&ThermalEventCamera::wrapperUpdate,this);
}

// stop the threaded I2C read
void ThermalEventCamera::stop(){
	this->stopThread = true;
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
		// update events map
		this->events.insert(std::pair<int,EventData>(i,this->frame[i]>this->last_frame[i]? 1 : this->frame[i]<this->last_frame[i] ? -1 : 0));
	}
	// update last_frame with curent frame
	std::copy(std::begin(this->frame),std::end(this->frame),std::begin(this->last_frame));
}

// threaded read of I2C buff
// repeated calls of read so long as stopThreadis false
int ThermalEventCamera::wrapperRead(){
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
int ThermalEventCamera::wrapperUpdate(){
	while(not this->stopThread)
		this->update();
	return 0;
}

// function for posting the signs as colours in the console
// based off the test example in the mlx90640 lib
void ThermalEventCamera::printSigns(){
	for(int x=0;x<32;++x){
		for(int y=0;y<24;++y){
			signed short val = this->out[32*(23-y) + x];
			if (val==1){
				std::cout << this->ansi_pos_color;
			}
			else if(val == -1){
				std::cout << this->ansi_neg_color;
			}
			else{
				std::cout << this->ansi_zero_color;
			}
		}
		std::cout << std::endl;
	}
	printf("\x1b[33A");
}

// set the color used with printing negative sign to console
void ThermalEventCamera::setNegColor(const char* neg){
	char* ll = (char*)neg;
	// convert string to lowercase
	while(*neg){
		*ll = tolower(*neg);
		neg++;
		ll++;
	}
	// compare and update negative string
	if(std::strcmp(ll,"red")==0)
	{
		this->ansi_neg_color = ANSI_COLOR_RED FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"yellow")==0)
	{
		this->ansi_neg_color = ANSI_COLOR_YELLOW FMT_STRING ANSI_COLOR_YELLOW;
	}
	else if(std::strcmp(ll,"none")==0)
	{
		this->ansi_neg_color = ANSI_COLOR_NONE FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"green")==0)
	{
		this->ansi_neg_color = ANSI_COLOR_GREEN FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"cyan")==0)
	{
		this->ansi_neg_color = ANSI_COLOR_CYAN FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"blue")==0)
	{
		this->ansi_neg_color = ANSI_COLOR_BLUE FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"magneta")==0)
	{
		this->ansi_neg_color = ANSI_COLOR_MAGENTA FMT_STRING ANSI_COLOR_RESET;
	}
	else
	{
		std::cerr << "Unsupported color " << ll << std::endl;
	}
}

// set the color used with printing positive sign to console
void ThermalEventCamera::setPosColor(const char* pos){
	char* ll = (char*)pos;
	// convert string to lowercase
	while(*pos){
		*ll = tolower(*pos);
		pos++;
		ll++;
	}
	// compare and update negative string
	if(std::strcmp(ll,"red")==0)
	{
		this->ansi_pos_color = ANSI_COLOR_RED FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"yellow")==0)
	{
		this->ansi_pos_color = ANSI_COLOR_YELLOW FMT_STRING ANSI_COLOR_YELLOW;
	}
	else if(std::strcmp(ll,"none")==0)
	{
		this->ansi_pos_color = ANSI_COLOR_NONE FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"green")==0)
	{
		this->ansi_pos_color = ANSI_COLOR_GREEN FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"cyan")==0)
	{
		this->ansi_pos_color = ANSI_COLOR_CYAN FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"blue")==0)
	{
		this->ansi_pos_color = ANSI_COLOR_BLUE FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"magneta")==0)
	{
		this->ansi_pos_color = ANSI_COLOR_MAGENTA FMT_STRING ANSI_COLOR_RESET;
	}
	else
	{
		std::cerr << "Unsupported color " << ll << std::endl;
	}
}

// set the color used with printing zero\neutral sign to console
void ThermalEventCamera::setZeroColor(const char* zero){
	char* ll = (char*)zero;
	// convert string to lowercase
	while(*zero){
		*ll = tolower(*zero);
		zero++;
		ll++;
	}
	// compare and update negative string
	if(std::strcmp(ll,"red")==0)
	{
		this->ansi_zero_color = ANSI_COLOR_RED FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"yellow")==0)
	{
		this->ansi_zero_color = ANSI_COLOR_YELLOW FMT_STRING ANSI_COLOR_YELLOW;
	}
	else if(std::strcmp(ll,"none")==0)
	{
		this->ansi_zero_color = ANSI_COLOR_NONE FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"green")==0)
	{
		this->ansi_zero_color = ANSI_COLOR_GREEN FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"cyan")==0)
	{
		this->ansi_zero_color = ANSI_COLOR_CYAN FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"blue")==0)
	{
		this->ansi_zero_color = ANSI_COLOR_BLUE FMT_STRING ANSI_COLOR_RESET;
	}
	else if(std::strcmp(ll,"magneta")==0)
	{
		this->ansi_zero_color = ANSI_COLOR_MAGENTA FMT_STRING ANSI_COLOR_RESET;
	}
	else
	{
		std::cerr << "Unsupported color " << ll << std::endl;
	}
}
