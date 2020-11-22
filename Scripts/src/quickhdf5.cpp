#include "quickhdf5.h"

EventRecorder::EventRecorder(const char* fname,int nfps){
    // open the file
    this->file.openFile(fname,H5F_ACC_TRUNC);
    // update logged fps
    fps = nfps;
    // update delay between frames
    this->frame_time_micros = 1000000/fps
    /* set dataset parameters */
    // set chunking for time series and data set
	  dparams.setChunk(2,this->chunk_dims);
    // set compression
    dparams.setDeflate(6);
    // fill dataset
    uint16_t fill_val = 0;
    dparams.setFillValue(dtype,&fill_val);
    // create dataset to hold frames
    dset = this->file.createDataSet("tevent",dtype,dataspace,dparams);
    dparams.close();
    /* set time series dataset */
    // set parameters for dataset
    tparams.setChunk(2,this->tchunk_dims);
    // set compression
    tparams.setDeflate(6);
    unsigned long tfill = 0;
    tparams.setFillValue(this->ttype,&tfill);
    // create dataset to hold time values
    dset = this->file.createDataSet("tevent",dtype,dataspace,dparams);
    tparams.close();
}
