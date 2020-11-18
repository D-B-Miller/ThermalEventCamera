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
	if(this->readThread.valid())
	{
		status = this->readThread.wait_for(span);
		if(status == std::future_status::timeout){
			std::cerr << "warning: timeout waiting for read thread to finish" << std::endl;
		}
	}
	// wait for update thread to finish
	if(this->updateThread.valid())
	{
		status = this->updateThread.wait_for(span);
		if(status == std::future_status::timeout){
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

// gets a copy of the last read data
// copies values into matrix provided by user
void ThermalEventCamera::getFrame(uint16_t (&cf)[834])
{
	std::mutex mx;
	// lock access to frame in case threaded interface is being used
	std::unique_lock<std::mutex> lck(mx);
	// copy values across
	std::copy(std::begin(this->frame),std::end(this->frame),std::begin(cf));
	// release lock on data
	lck.unlock();
}

// apply MLX90640_InterpolateOutliers to current state of frame
// and copy values to matrix provided by user
void ThermalEventCamera::interpOutliers(uint16_t (&cf)[832])
{
	std::mutex mx;
	std::unique_lock<std::mutex> lck(mx);
	// interpolate outliers
	// uses matrix setup by *_ExtractParameters call in constructor
	MLX90640_InterpolateOutliers(this->frame,this->eeMLX90640);
	// free mutex so the thread can keep running
	lck.unlock();
	// copy matrix to pointer
	std::copy(std::begin(this->eeMLX90640),std::end(this->eeMLX90640),std::begin(cf));
}

// start the threaded I2C read
void ThermalEventCamera::start()
{
	this->stopFlag = false; // set stop flag to false
	// if the fps has not been set. the camera prob hasn't been initialised
	if(this->fps == 0){
		std::cerr << "Frame rate has not been set! Cannot start threads!" << std::endl;
		return;
	}
	// create asynchronous thread to read from I2C bus and populate behaviour
	this->readThread = std::async(std::launch::async,&ThermalEventCamera::wrapperRead,this);
	this->updateThread = std::async(std::launch::async,&ThermalEventCamera::wrapperUpdate,this);
}

// stop the threaded I2C read
void ThermalEventCamera::stop()
{
	this->stopFlag = true;
}

// get flag indicating if a custom comparison function has been set
bool ThermalEventCamera::getCompareFlag()
{
	return this->cmpSet;
}

// set pixel comparison function for comparing a pixel from current
// and past frames
void ThermalEventCamera::setCompare(CompareFunc f)
{
	// update comparison function
	this->compare = f;
	// an uninitialised CompareFunc when compared to 0 returns true
	// if the function is initialised set flag
	if(f!=0){
		this->cmpSet = true;
	}
	else{	// if the user has passed a blank comparison function
		// clear flag
		this->cmpSet = false;
	}
}

// single read of I2C buff and find element wise
void ThermalEventCamera::read()
{
	// get frame data
	MLX90640_GetFrameData(MLX_I2C_ADDR,this->frame);
	// check for changes against last frame
	for(int i=0;i<834;++i)
	{
		bool cmp = false;
		// if the compare function is set
		// use it to compare the target pixel from current and pase frame
		if(this->cmpSet)
		{
			cmp = this->compare(this->frame[i],this->last_frame[i]);
		}
		else
		{	// if the compare function has not been set
			// calculate log of frame intensity adding +1 to avoid zero-error
			cmp = 65535*log(this->frame[i]+1)!=65535*log(this->last_frame[i]+1);
		}
		// if the pixels are different according to the comparison fn
		// generate EventData and add to queue
		if(cmp)
		{
			this->events.push(EventData(i,this->frame[i]>this->last_frame[i]? 1 : -1));
		}
	}
	// update last_frame with curent frame
	std::copy(std::begin(this->frame),std::end(this->frame),std::begin(this->last_frame));
}

// threaded read of I2C buff
// repeated calls of read so long as stopThreadis false
int ThermalEventCamera::wrapperRead()
{
	// get lock on std cout
	// to be used when exception handling is set
	std::unique_lock<std::mutex> lck(this->print_mutex);
	lck.unlock();
	while(!this->stopFlag)
	{
		this->read();
	}
	lck.lock();
	return 0;
}

// update the output matrix
// clear the current matrix, query the events map for any changes and process any
void ThermalEventCamera::update()
{
	if(this->clearSigns){
		std::fill(std::begin(this->out),std::end(this->out),0);
	}
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
				this->lts = ev.time();
			}
		}
	}
}

// threaded updated based on current data
// runs so long as stopThread is True
int ThermalEventCamera::wrapperUpdate()
{
	std::unique_lock<std::mutex> lck(this->print_mutex);
	lck.unlock();
	while(!this->stopFlag){
		this->update();
	}
	lck.lock();
	return 0;
}

// function for posting the signs as colours in the console
// based off the test example in the mlx90640 lib
// prints the array with the exception of the last two entries so it's a rectangular matrix
void ThermalEventCamera::printSigns(bool flip)
{	// rows and column of printed image
	int c=32,r=26;
	// if the flip flag is set
	// the rows and cols are switched
	if(flip){
		c = 26;
		r = 32;
	}
	for(int x=0;x<c;++x){
		for(int y=0;y<r;++y){
			// get value of signs matrix
			signed short val = this->out[32*(25-y) + x];
			// print color based on value
			if (val==1){
				std::cout << this->ansi_pos_color;
			}
			else if(val == -1){
				std::cout << this->ansi_neg_color;
			}
			else if(val == 0){
				std::cout << ' ';
			}
		}
		std::cout << std::endl;
	}
	printf("\x1b[33A");
}

// function for printing the out matrix as raw values
// used as debugging
void ThermalEventCamera::printSignsRaw(bool flip)
{	// rows and column of printed image
	int c=32,r=26;
	// if the flip flag is set
	// the rows and cols are switched
	if(flip){
		c = 26;
		r = 32;
	}
	for(int x=0;x<c;++x)
	{
		for(int y=0;y<r;++y)
		{
			std::cout << this->out[32*(25-y)+x];
		}
		std::cout << std::endl;
	}
	printf("\x1b[33A");
}

// set the color used with printing negative sign to console
void ThermalEventCamera::setNegColor(const char* neg)
{
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
void ThermalEventCamera::setPosColor(const char* pos)
{
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
void ThermalEventCamera::printFrame(bool flip)
{
	// rows and column of printed image
	int c=32,r=24;
	// if the flip flag is set
	// the rows and cols are switched
	if(flip){
		c = 24;
		r = 32;
	}
	static float mlx90640To[768]; // converted temperature values
	float eTa = MLX90640_GetTa(this->frame, &this->mlx90640); // estimated environmental temperatures
	// convert data to temperature
        MLX90640_CalculateTo(this->frame, &this->mlx90640, this->emissivity, eTa, mlx90640To);
	// fix bad temperature values
        MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, 1, &this->mlx90640);
        MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, 1, &this->mlx90640);
	// iterate over matrix and print values as colors
	for(int x = 0; x < c; x++){
            for(int y = 0; y < r; y++){
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
// user provides waiting time in millisecodns for wait_for call
bool ThermalEventCamera::isReadAlive(int t)
{
	return this->readThread.wait_for(std::chrono::milliseconds(t)) != std::future_status::ready;
}

// function for checking if the update thread is alive
// user provides waiting time in milliseconds for wait_for call
bool ThermalEventCamera::isUpdateAlive(int t)
{
	return this->updateThread.wait_for(std::chrono::milliseconds(t)) != std::future_status::ready;
}
