#include "teventcamera.h"

// constructor accepting FPS argument
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
		std::cerr << "Unsupported framerate: " << fps << std::endl;
		this->fps = 0;
	}
	MLX90640_SetChessMode(MLX_I2C_ADDR);
	MLX90640_DumpEE(MLX_I2C_ADDR, this->eeMLX90640);
        MLX90640_SetResolution(MLX_I2C_ADDR, 0x03);
	MLX90640_ExtractParameters(this->eeMLX90640, &this->mlx90640);
}

// basic constructor
// initialised to 32 fps
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
	this->stopFlag = true;
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

// function to update refresh rate
void ThermalEventCamera::setFps(int nfps)
{
	switch(nfps){
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
		std::cerr << "Unsupported framerate: " << nfps << std::endl;
	}
}

// start the threaded I2C read
void ThermalEventCamera::start(){
	this->stopFlag = false; // set stop flag to false
	// if the fps has not been set. the camera prob hasn't been initialised
	if(this->fps == 0){
		std::cerr << "Frame rate has not been set! Cannot start threads!" << std::endl;
		return;
	}
	// create asynchronous thread to read from I2C bus and populate behaviour
	std::cout << "Starting threads" << std::endl;
	this->readThread = std::async(std::launch::async,&ThermalEventCamera::wrapperRead,this);
	this->updateThread = std::async(std::launch::async,&ThermalEventCamera::wrapperUpdate,this);
}

// stop the threaded I2C read
void ThermalEventCamera::stop(){
	this->stopFlag = true;
}

// single read of I2C buff and find element wise
void ThermalEventCamera::read(){
	// get frame data
	MLX90640_GetFrameData(MLX_I2C_ADDR,this->frame);
	// check for changes against last frame
	for(int i=0;i<834;++i)
	{	// calculate log of frame intensity adding +1 to avoid zero-error
		if(65535*log(this->frame[i]+1)!=65535*log(this->last_frame[i]+1))
		{
			//this->events.insert(std::pair<int,EventData>(i,this->frame[i]>this->last_frame[i]? 1 : -1));
			this->events.push(EventData(i,this->frame[i]>this->last_frame[i]? 1 : -1));
		}
	}
	// update last_frame with curent frame
	std::copy(std::begin(this->frame),std::end(this->frame),std::begin(this->last_frame));
}

// threaded read of I2C buff
// repeated calls of read so long as stopThreadis false
int ThermalEventCamera::wrapperRead(){
	std::cout << "entering read loop" << std::endl;
	while(!this->stopFlag)
	{
		this->read();
	}
	return 0;
}

// update the output matrix
// clear the current matrix, query the events map for any changes and process any
void ThermalEventCamera::update(){
	// if there are events in the queue
	if(!this->events.empty())
	{
		EventData ev;
		// get current size of queue and iterate over it
		// getting the entries and updating the signs matrix
		for(unsigned long i=0;i< this->events.size();++i){
			// if successful, update sign
			if(this->events.pop(ev)){
				this->out[ev.idx] = ev.sign;
			}
		}
	}
}

// threaded updated based on current data
// runs so long as stopThread is True
int ThermalEventCamera::wrapperUpdate(){
	while(!this->stopFlag)
	{
		this->update();
	}
	return 0;
}

// function for posting the signs as colours in the console
// based off the test example in the mlx90640 lib
// prints the array with the exception of the last two entries so it's a rectangular matrix
void ThermalEventCamera::printSigns(){
	for(int x=0;x<32;++x){
		for(int y=0;y<26;++y){
			signed short val = this->out[32*(25-y) + x];
			if (val==1){
				std::cout << this->ansi_pos_color;
			}
			else if(val == -1){
				std::cout << this->ansi_neg_color;
			}
		}
		std::cout << std::endl;
	}
	printf("\x1b[33A");
}

// function for printing the out matrix as raw values
void ThermalEventCamera::printSignsRaw()
{
	for(int x=0;x<32;++x)
	{
		for(int y=0;y<26;++y)
		{
			std::cout << this->out[32*(25-y)+x];
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
// convert frame to temperature using set emissivity and print as colors
void ThermalEventCamera::printFrame(){
	static float mlx90640To[768]; // converted temperature values
	float eTa = MLX90640_GetTa(this->frame, &this->mlx90640); // estimated environmental temperatures
	// convert data to temperature
        MLX90640_CalculateTo(this->frame, &this->mlx90640, this->emissivity, eTa, mlx90640To);
	// fix bad temperature values
        MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, 1, &this->mlx90640);
        MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, 1, &this->mlx90640);
	// iterate over matrix and print values as colors
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

// function for checking if the read thread is alive
bool ThermalEventCamera::isReadAlive(int t)
{
	// if there is not a shared state available
	// then it must still be running
	if(!this->readThread.valid())
	{
		return true;
	}
	else
	{
		// if a shared state is available
		// wait for a returned result and check returned status
		return this->readThread.wait_for(std::chrono::milliseconds(t)) == std::future_status::ready;
	}
}

bool ThermalEventCamera::isUpdateAlive(int t)
{
	if(!this->updateThread.valid())
	{
		return true;
	}
	else
	{
		return this->updateThread.wait_for(std::chrono::milliseconds(t)) == std::future_status::ready;
	}
}
