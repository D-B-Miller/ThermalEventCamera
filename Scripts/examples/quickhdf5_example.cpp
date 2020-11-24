#include "teventcamera.h"
#include "quickhdf5.h"

int main(){
	// create camera
	ThermalEventCamera cam(32);
	// create quick file
	EventRecorder rec("test.hdf5");
}
