#include "H5Cpp.h"
#include <iostream>
#include <chrono>
#include "teventcamera.h"

// noise filter developed through experiments
// see FILTER.md for details
bool noise_filt(uint16_t c, uint16_t p)
{
	// threshold parameters
	// set here for easy editing
	const uint16_t t0 = 65000,t1=t0+350;
	// if neither of the pixels are within the activity threshold
	// ignore them
	if((c<=t1)||(p<=t1)){
		return false;
	}
	else{	// then return true for any difference
		return c!=p? true: false;
	}
}


int main(){
	// create event camera
	ThermalEventCamera cam(32);
	// update comparison function
	cam.setCompare(noise_filt);
	// time limit on recording
	// using long and long as it most closely matches chrono millisecocnds dtype
	long long tlim = 120;
	int err = 0; // returned error code
	// filename of output file
	char* fname = "recordall.hdf5";

	/***** parameters used in hdf5 file *****/
	// dataset parameters
	// they are defined seperately so the user can adjust each dataset
	// according to their need
	H5::DSetCreatPropList cparams,dparams,tparams,oparams;
	// dataspace for hyperslab
	H5::DataSpace slab;

	/* temperature data */
	static float temp[768]; // in
	// new dimensions once extended, updated in loop
	hsize_t fdimsext[3] = {24,32,1};
        // starter dimensions
        hsize_t fdims[3] = {24,32,1};
    	hsize_t fmaxdims[3] = {24,32,H5S_UNLIMITED};
    	// size of chunks
    	hsize_t fchunk_dims[3] = {24,32,1};
    	// offset for hyperslab
    	hsize_t foffset[3] = {0,0,0};
	H5::DataSpace dataspace(3,fdims,fmaxdims);

	/* frame data */
	uint16_t frame[834];
	hsize_t dimsext[2] = {834,1}; // current size of the dataset as we extend it
    	hsize_t dims[2] = {834,1}; // starter dimensions
    	hsize_t chunk_dims[2] = {834,1}; // dunk dims
    	hsize_t offset[2] = {0,0}; // offset for hyperslab
	hsize_t maxdims[2] = {834,H5S_UNLIMITED}; // max dims of dataset, set to unlimited to allow extension
	H5::DataSpace fdataspace = H5::DataSpace(2,dims,maxdims); // dataspace for data

	/*time series dimensions*/
	hsize_t tdimsext[1] = {1}; // current size of the dataset as we extend it
    	hsize_t tdims[1] = {1}; // starter dimensions
    	hsize_t tchunk_dims[1] = {1}; // dunk dims
    	hsize_t toffset[2] = {0,0}; // offset for hyperslab
	hsize_t tmaxdims[2] = {H5S_UNLIMITED};
	H5::DataSpace timespace = H5::DataSpace(1,tdims,tmaxdims);
	
	/*event data output flag*/
	hsize_t odimsext[2] = {834,1}; // current size of the dataset as we extend it
    	hsize_t odims[2] = {834,1}; // starter dimensions
    	hsize_t ochunk_dims[2] = {834,1}; // dunk dims
    	hsize_t ooffset[2] = {0,0}; // offset for hyperslab
	hsize_t omaxdims[2] = {834,H5S_UNLIMITED}; // max dims of dataset, set to unlimited to allow extension
	H5::DataSpace odataspace = H5::DataSpace(2,odims,omaxdims); // dataspace for data

    	// open the file
    	H5::H5File f(fname,H5F_ACC_TRUNC);
    	/* set dataset parameters */
    	// set chunking for time series and data set
    	dparams.setChunk(2,chunk_dims);
    	// set compression
    	dparams.setDeflate(6);
    	// fill dataset
    	uint16_t dset_fill_val = 0;
	H5::PredType dtype = H5::PredType::NATIVE_UINT16;
    	dparams.setFillValue(dtype,&dset_fill_val);
    	// create dataset to hold frames
    	H5::DataSet dset = f.createDataSet("tevent",dtype,dataspace,dparams);
    	dataspace = dset.getSpace();
    	dparams.close();

	/* set time series dataset */
	// set parameters for dataset
	tparams.setChunk(2,tchunk_dims);
	// set compression
	tparams.setDeflate(6);
	long long time_fill_val = 0;
	H5::PredType ttype = H5::PredType::NATIVE_LLONG;
	tparams.setFillValue(ttype,&time_fill_val);
	// create dataset to hold time values
	H5::DataSet tset = f.createDataSet("time",ttype,timespace,tparams);
	timespace = tset.getSpace();
	tparams.close();

	/* temperature parameters */
	// modify dataset creation properties to enable chunking
	cparams.setChunk(3,chunk_dims);
	// set the initial value of the dataset
	static float temp_fill_val = 0.0;
	H5::PredType ftype = H5::PredType::NATIVE_FLOAT;
	cparams.setFillValue(ftype,&temp_fill_val);
	H5::DataSet fset = f.createDataSet("temperature",ftype,fdataspace,cparams);
	cparams.close();
	
	/*sign matrix parameters*/
	// set chunking for time series and data set
    	oparams.setChunk(2,chunk_dims);
    	// set compression
    	oparams.setDeflate(6);
    	// fill dataset
    	signed short sign_fill_val = 0;
	H5::PredType stype = H5::PredType::NATIVE_SHORT;
    	oparams.setFillValue(stype,&sign_fill_val);
    	// create dataset to hold frames
    	H5::DataSet oset = f.createDataSet("signs",stype,odataspace,oparams);
    	odataspace = oset.getSpace();
    	oparams.close();

	// get start time of writing
    	auto startt = std::chrono::system_clock::now();
    	while(1){
       	// get current time to calculate time since start
       	auto tt = std::chrono::system_clock::now();
       	try{
		// thermal event data
		cam.read(); // read frame
		cam.getFrame(frame); // get a copy of it
		// get hyperslab
		slab = dset.getSpace();
		slab.selectHyperslab(H5S_SELECT_SET,chunk_dims,offset);
		// write event camera data
		dset.write(frame,dtype,dataspace,slab);
		
		///// temperature data
		slab = fset.getSpace();
		slab.selectHyperslab(H5S_SELECT_SET,fchunk_dims,foffset);
		// get temperature values
		cam.getTemperature(temp);
		// write temperature to dataset
		fset.write(temp,ftype,fdataspace,slab);
		
		///// signs matrix
		// get hyperslab
		slab = oset.getSpace();
		slab.selectHyperslab(H5S_SELECT_SET,ochunk_dims,ooffset);
		// write event camera data
		oset.write(cam.out,otype,odataspace,slab);
		
		///// log time
		slab = tset.getSpace();
		slab.selectHyperslab(H5S_SELECT_SET,tchunk_dims,toffset);
		// get elapsed time since start in milliseconds
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tt-startt);
		// write elapsed time to data
		tset.write(elapsed.count(),ftype,tspace,slab);

		/* prep for next iteration */
		//// extend temperature dataset
		fdimsext[1]+=1;
		fdataset.extend(fdimsext);
		// increase offset for next frame
		foffset[1]+=1;
		//// extend raw data dataset
		dimsext[1]+=1;
		dataset.extend(dimsext);
		// increase offset for next frame
		offset[1]+=1;
		//// extend signs matrix
		odimsext[1]+=1;
		odataset.extend(odimsext);
		oofset[1]+=1;
		//// extend time dataset
		tdimsext[2]+=1;
		tdataset.extend(tdimsext);
		// increase offset for next frame
		toffset[2]+=1;
		
		// check elapsed time against time limit
		if(elapsed.count()>=tlim){
			std::cout << "Reached time limit!" << std::endl;
			break;
		}
       }catch(H5::FileIException error){
		std::cerr << "Dataspace HDF5 File Exception! Closing file! " <<std::endl;
		error.printErrorStack();
		err = 1;
		break;
       }catch(H5::DataSetIException error){
		std::cerr << "Dataspace HDF5 Dataset Exception! Closing file" <<std::endl;
		error.printErrorStack();
		err = 2;
		break;
       }catch(H5::DataSpaceIException error){
            	std::cerr << "Dataspace HDF5 Dataspace Exception! Closing file" << std::endl;
            	error.printErrorStack();
            	err = 3;
		break;
       }
    }
	// close file
	// needs to be explicitly closed otherwise it becomes inaccessbile
	f.close();
	return err;
}
