#include <thermalraw.h>

// blank constructor
ThermalEventRaw::ThermalEventRaw(){
	// open access to device saving file descriptor with ASYNC flag
	this->fd = open("/dev/i2c-1", O_RDONLY | O_ASYNC);
	if (this->fd == -1) {
		std::cerr <<"Failed to open /dev/i2c-1");
		return;
	}
	else{
		// set device to non blocking
		int flags = fcntl(this->fd,F_GETFL,);
		fcntl(this->fd,F_SETFL,flags | O_NONBLOCK);
		// create thread for reading
	}
}

// deconstructor
ThermalEventRaw::~ThermalEventRaw(){
	if(this->fd != -1){
		close(this->fd);
	}
}

// start reading thread
void ThermalEventRaw::start(){
	this->stopFlag = false;
	this->readThread = std::async(std::launch::async,&ThermalEventRaw::readLoop,this);
}

// set flag to stop reading thread
void ThermalEventRaw::stop(){
	this->stopFlag = true;
}


void ThermalEventRaw::updateLoop(){
	while(!this->stopFlag)
		this->read(buf);
}

// method for reopening the device
int ThermalEventRaw::openDev(){
	// close device if open
	this->closeDev();
	// open access to device saving file descriptor with ASYNC flag
	this->fd = open("/dev/i2c-1", O_RDONLY | O_ASYNC);
	if (this->fd == -1) {
		std::cerr <<"Failed to open /dev/i2c-1");
		return 1;
	}
	return 0;
}

// method for closing the device
void ThermalEventRaw::closeDev(){
	if(this->fd != -1){
		close(this->fd);
	}
}

// read data and update output
int ThermalEventRaw::update(){
	char raw[1664];
	ssize_t rsize = 0;   // read number of bytes
	int i=0;			 // counter var
	uint16_t *p = this->out; // get pointer to start 
	// read data into buffer
	rsize = read(this->fd,raw,1664);
	if (errno!=0){
		std::cerr << "Read Error: " << errno << " : " << strerror(errno) << std::endl;
		return 2;
	}
	// convert buffer to output 
	for(int cnt=0;cnt<832;cnt++)
	{
		i = cnt << 1;
		this->*p++ = (uint16_t)raw[i]*256+(uint16_t)[i+1];
	}
	return 0;
}