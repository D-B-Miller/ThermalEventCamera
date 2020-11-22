#include "H5Cpp.h"

// class for quick building a hdf5 recorder for the ThermalEventCamrea
// as the sizes and data types are known we can define and manage the size and dataslab
class EventRecorder{
    public:
        virtual EventRecorder(const char* fname bool addTime=true);
        virtual ~EventRecorder();
  
        H5::H5File f; // file object
        const auto dtype = H5::PredType::NATIVE_UINT16; // data type of file
  
        void writeFrame(uint16_t frame[834]); // write frame to file
        bool hasTimeSeries(){return this->hasTime;}; // return state of hasTime flag
  
    private:
        bool hasTime = false; // flag to indicate if a time dataset is/has been added
        hsize_t dimsext[2] = {834,1}; // current size of the dataset as we extend it
        hsize_t dims[2] = {834,1}; // starter dimensions
        hsize_t chunk_dims[2] = {834,1}; // dunk dims
        hsize_t offset[2] = {0,0}; // offset for hyperslab
        H5::DataSpace dataspace(2,dims,maxdims); // dataspace
	      // dataspace for hyperslab
	      H5::DataSpace slab;
	      // enable chunking
	      H5::DSetCreatPropList cparams;
};
