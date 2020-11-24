#include "teventcamera.h"
#include "quickhdf5.h"

int main(){
	// create camera
	ThermalEventCamera cam(32);
	// create eventrecorder
	EventRecorder rec("test.hdf5");
	// array to hold frame
	uint16_t frame[834] = {0};

	// get frame
	cam.read(); // get frame
	cam.getFrame(frame); // get a copy of it
	// write frame to file
	// handles extending the file
	rec.writeFrame(frame);
	// close file
	// happens in deconstructor but makes sure
	rec.close();
}
