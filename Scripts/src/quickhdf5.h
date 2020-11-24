#include "H5Cpp.h"
#include <iostream>
#include <chrono>

// class for quick building a hdf5 recorder for the ThermalEventCamrea
// as the sizes and data types are known we can define and manage the size and dataslab
typedef std::chrono::time_point<std::chrono::system_clock> system_tp;
class EventRecorder{
    public:
        EventRecorder(const char* fname); // create recorder with filename and fps of recording
        virtual ~EventRecorder(); // deconstructor of recording. ensures that the file is closed

        H5::H5File f; // file object
        const H5::PredType dtype = H5::PredType::NATIVE_UINT16; // data type of file
	const H5::PredType ttype = H5::PredType::NATIVE_LLONG; // data type of time series. set to long for extended recording

        int writeFrame(uint16_t frame[834]); // write frame to file
  	system_tp getStartTime(){return this->rec_start;}; // get start time of recording
	system_tp getEndTime(){return this->rec_end;}; // get end time of recording
	void close(); // close file
	hsize_t getNumFrames(){return this->dimsext[1];}; // get the total number of frames 
    private:
	system_tp rec_start = std::chrono::system_clock::now(); // start time
	system_tp rec_end = std::chrono::system_clock::now(); // end time of recording
	/*data dimensions*/
        hsize_t dimsext[2] = {834,1}; // current size of the dataset as we extend it
        hsize_t dims[2] = {834,1}; // starter dimensions
        hsize_t chunk_dims[2] = {834,1}; // dunk dims
        hsize_t offset[2] = {0,0}; // offset for hyperslab
	hsize_t maxdims[2] = {834,H5S_UNLIMITED}; // max dims of dataset, set to unlimited to allow extension
	H5::DataSpace dataspace = H5::DataSpace(2,this->dims,this->maxdims); // dataspace for data
	H5::DataSet dset; // dataset ref
	/*time series dimensions*/
	hsize_t tdimsext[1] = {1}; // current size of the dataset as we extend it
        hsize_t tdims[1] = {1}; // starter dimensions
        hsize_t tchunk_dims[1] = {1}; // dunk dims
        hsize_t toffset[2] = {0,0}; // offset for hyperslab
	hsize_t tmaxdims[2] = {H5S_UNLIMITED};
	H5::DataSpace timespace = H5::DataSpace(1,tdims,tmaxdims);
	H5::DataSet tset; // timeseries reference
      	// dataspace for hyperslab
      	H5::DataSpace slab;
      	// dataset parameters
      	H5::DSetCreatPropList dparams,tparams;
};
