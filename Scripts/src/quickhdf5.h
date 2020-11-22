#include "H5Cpp.h"

// class for quick building a hdf5 recorder for the ThermalEventCamrea
// as the sizes and data types are known we can define and manage the size and dataslab
class EventRecorder{
    public:
        virtual EventRecorder(const char* fname bool addTime=true);
        virtual ~EventRecorder();
  
        H5::H5File f; // file object
        const auto dtype = H5::PredType::NATIVE_UINT16; // data type of file
	const auto ttype = H5::PredType::NATIVE_ULONG // data type of time series. set to long for extended recording
  
        void writeFrame(uint16_t frame[834]); // write frame to file
        bool hasTimeSeries(){return this->hasTime;}; // return state of hasTime flag
  
    private:
        bool hasTime = false; // flag to indicate if a time dataset is/has been added
	/*data dimensions*/
        hsize_t dimsext[2] = {834,1}; // current size of the dataset as we extend it
        hsize_t dims[2] = {834,1}; // starter dimensions
        hsize_t chunk_dims[2] = {834,1}; // dunk dims
        hsize_t offset[2] = {0,0}; // offset for hyperslab
	hsize_t maxdims[2] = {834,H5S_UNLIMITED};
	H5::DataSpace dataspace(2,dims,maxdims); // dataspace for data
	/*time series dimensions*/
	hsize_t tdimsext[2] = {1,1}; // current size of the dataset as we extend it
        hsize_t tdims[2] = {1,1}; // starter dimensions
        hsize_t tchunk_dims[2] = {1,1}; // dunk dims
        hsize_t toffset[2] = {0,0}; // offset for hyperslab
	hsize_t tmaxdims[2] = {1,H5S_UNLIMITED};
	H5::DataSpace timespace(2,tdims,tmaxdims}
      	// dataspace for hyperslab
      	H5::DataSpace slab;
      	// parameters
      	H5::DSetCreatPropList dparams,tparams;
};
