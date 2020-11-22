#include "quickhdf5.h"

EventRecorder::EventRecorder(const char* fname){
    // open the file
    this->file.openFile(fname,H5F_ACC_TRUNC);
    /* set dataset parameters */
    // set chunking for time series and data set
    this->dparams.setChunk(2,this->chunk_dims);
    // set compression
    this->dparams.setDeflate(6);
    // fill dataset
    uint16_t fill_val = 0;
    this->dparams.setFillValue(this->dtype,&fill_val);
    // create dataset to hold frames
    this->dset = this->file.createDataSet("tevent",this->dtype,this->dataspacethis->,dparams);
    this->dataspace = this->dset.getSpace();
    this->dparams.close();
    /* set time series dataset */
    // set parameters for dataset
    this->tparams.setChunk(2,this->tchunk_dims);
    // set compression
    this->tparams.setDeflate(6);
    unsigned long tfill = 0;
    this->tparams.setFillValue(this->ttype,&tfill);
    // create dataset to hold time values
    this->tset = this->file.createDataSet("tevent",this->dtype,this->dataspace,this->dparams);
    this->timespace = this->tset.getSpace();
    this->tparams.close();
}

// deconstructor to ensure that the hdf5 file is closed
EventRecorder::~EventRecorder(){
    this->file.close(); // close hdf5 file
    this->rec_end = std::chrono::system_clock::now(); // update ending recorded file
}

void EventRecorder::close(){
    this->f->close(); // ensure file is closed
    this->rec_end = std::chrono::system_clock::now(); // update end recording time
}

// write frame to file
int EventRecorder::writeFrame(uint16_t frame[834]){
       // get time of writing
       auto tt = std::chrono::system_clock::now();
       // if first frame set as time of recording
       if(this->toffset[1]==0){
           this->rec_start = tt;   
       }
       try{
           /*write data frame*/
           this->slab = this->dataspace.getSpace();
           // select hyperslab for writing
           this->slab.selectHyperslab(H5S_SELECT_SET,this->chunk_dims,this->offset);
           // write dataset
           this->dset.write(frame,this->dtype,this->dataspace,this->slab);
           // extend dataset
           this->dimsext[1]+=1;
           this->dset.extend(this->dimsext);
           // increase offset
           this->offset[1]+=1;
       }catch(H5::FileIException error){
	    std::cerr << "Dataspace HDF5 File Exception! Closing file! " << error << std::endl;
	    error.printErrorStack();
	    return 1;
       }catch(H5::DataSetIException error){
	    std::cout << "Dataspace HDF5 Dataset Exception! Closing file" << std::endl;
	    error.printErrorStack();
	    return 2;
       }catch(H5::DataSpaceIException error){
            std::cout << "Dataspace HDF5 Dataspace Exception! Closing file" << std::endl;
            error.printErrorStack();
            return 3;
       }
    
       try{
           /*write time stamp*/
           this->slab = this->timespace.getSpace();
           // select hyperslab for writing
           this->slab.selectHyperslab(H5S_SELECT_SET,this->tchunk_dims,this->toffset);
           // write dataset
           this->tset.write(std::duration_cast<std::chrono::microseconds>(tt-this->rec_start).count(),this->ttype,this->timespace,this->slab);
           // extend dataset
           this->tdimsext[1]+=1;
           this->tset.extend(this->tdimsext);
           // increase offset
           this->toffset[1]+=1;
       }catch(H5::FileIException error){
	   std::cerr << "Timespace HDF5 File Exception! Closing file! " << error << std::endl;
	   error.printErrorStack();
	   return 4;
       }catch(H5::DataSetIException error){
	   std::cout << "Timespace HDF5 Dataset Exception! Closing file" << std::endl;
	   error.printErrorStack();
	   return 5;
       }catch(H5::DataSpaceIException error){
            std::cout << "Timespace HDF5 Dataspace Exception! Closing file" << std::endl;
            error.printErrorStack();
            return 6;
       }
       return 0;       
}
