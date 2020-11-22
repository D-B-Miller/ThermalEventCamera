#include "H5Cpp.h"
#include <chrono>

// class for quick building a hdf5 recorder for the ThermalEventCamrea
// as the sizes and data types are known we can define and manage the size and dataslab
class EventRecorder{
    public:
        virtual EventRecorder(const char* fname,int nfps = 32); // create recorder with filename and fps of recording
        virtual ~EventRecorder(); // deconstructor of recording. ensures that the file is closed
  
        H5::H5File f; // file object
        const auto dtype = H5::PredType::NATIVE_UINT16; // data type of file
	const auto ttype = H5::PredType::NATIVE_ULONG // data type of time series. set to long for extended recording
  
        void writeFrame(uint16_t frame[834]); // write frame to file
  	std::chrono::time_point getStartTime(){return this->rec_start;}; // get start time of recording
	std::chrono::time_point getEndTime(){return this->rec_end;}; // get end time of recording
	void close(); // close file
    private:
	std::chrono::time_point rec_start = std::chrono::system_clock::now(); // start time
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now()); // elapsed time of recording
	std::chrono::time_point rec_end = std::chrono::system_clock::now(); // end time of recording
	int fps=0; // fps of logging
	long frame_time_micros = 0; // delay between recordings
	/*data dimensions*/
        hsize_t dimsext[2] = {834,1}; // current size of the dataset as we extend it
        hsize_t dims[2] = {834,1}; // starter dimensions
        hsize_t chunk_dims[2] = {834,1}; // dunk dims
        hsize_t offset[2] = {0,0}; // offset for hyperslab
	hsize_t maxdims[2] = {834,H5S_UNLIMITED};
	H5::DataSpace dataspace(2,dims,maxdims); // dataspace for data
	H5::DataSet dset; // dataset ref
	/*time series dimensions*/
	hsize_t tdimsext[2] = {1,1}; // current size of the dataset as we extend it
        hsize_t tdims[2] = {1,1}; // starter dimensions
        hsize_t tchunk_dims[2] = {1,1}; // dunk dims
        hsize_t toffset[2] = {0,0}; // offset for hyperslab
	hsize_t tmaxdims[2] = {1,H5S_UNLIMITED};
	H5::DataSpace timespace(2,tdims,tmaxdims);
	H5::DataSet tset; // timeseries reference
      	// dataspace for hyperslab
      	H5::DataSpace slab;
      	// dataset parameters
      	H5::DSetCreatPropList dparams,tparams;
};
