#include "quickhdf5.h"

EventRecorder::EventRecorder(const char* fname,int nfps){
    // open the file
    this->file.openFile(fname,H5F_ACC_TRUNC);
    // update logged fps
    this->fps = nfps;
    // update delay between frames
    this->frame_time_micros = 1000000/this->fps;
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
    this->tparams.close();
}
