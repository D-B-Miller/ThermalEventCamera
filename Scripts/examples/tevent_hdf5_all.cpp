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
	std::cout << "setting up camera" << std::endl;
	ThermalEventCamera cam(32);
	// update comparison function
	cam.setCompare(noise_filt);
	// time limit on recording
	// using long and long as it most closely matches chrono millisecocnds dtype
	long long tlim = 120*1000;
	std::cout << "time limit set to " << tlim << "ms" << std::endl;
	int err = 0; // returned error code
	// filename of output file
	char* fname = "recordall.hdf5";
	std::cout << "filename set to " << fname << std::endl;

	/***** parameters used in hdf5 file *****/
	// dataset parameters
	// they are defined seperately so the user can adjust each dataset
	// according to their need
	H5::DSetCreatPropList temp_params,raw_params,time_params,sign_params;
	// dataspace for hyperslab
	H5::DataSpace slab;

	/* temperature data */
	static float temp[768]; // in
	// new dimensions once extended, updated in loop
	hsize_t temp_dimsext[3] = {24,32,1};
        // starter dimensions
        hsize_t temp_dims[3] = {24,32,1};
    	hsize_t temp_maxdims[3] = {24,32,H5S_UNLIMITED};
    	// size of chunks
    	hsize_t temp_chunk_dims[3] = {24,32,1};
    	// offset for hyperslab
    	hsize_t temp_offset[3] = {0,0,0};
	H5::DataSpace temp_dataspace(3,temp_dims,temp_maxdims);

	/* frame data */
	uint16_t frame[834]; // matrix to hold raw camera data
	hsize_t raw_dimsext[2] = {834,1}; // current size of the dataset as we extend it
    	hsize_t raw_dims[2] = {834,1}; // starter dimensions
    	hsize_t raw_chunk_dims[2] = {834,1}; // dunk dims
    	hsize_t raw_offset[2] = {0,0}; // offset for hyperslab
	hsize_t raw_maxdims[2] = {834,H5S_UNLIMITED}; // max dims of dataset, set to unlimited to allow extension
	H5::DataSpace raw_dataspace = H5::DataSpace(2,raw_dims,raw_maxdims); // dataspace for data

	/*time series dimensions*/
	hsize_t time_dimsext[1] = {1}; // current size of the dataset as we extend it
    	hsize_t time_dims[1] = {1}; // starter dimensions
    	hsize_t time_chunk_dims[1] = {1}; // dunk dims
    	hsize_t time_offset[1] = {0}; // offset for hyperslab
	hsize_t time_maxdims[1] = {H5S_UNLIMITED};
	H5::DataSpace timespace = H5::DataSpace(1,time_dims,time_maxdims);

	/*event data output flag*/
	hsize_t signs_dimsext[2] = {834,1}; // current size of the dataset as we extend it
    	hsize_t signs_dims[2] = {834,1}; // starter dimensions
    	hsize_t signs_chunk_dims[2] = {834,1}; // dunk dims
    	hsize_t signs_offset[2] = {0,0}; // offset for hyperslab
	hsize_t signs_maxdims[2] = {834,H5S_UNLIMITED}; // max dims of dataset, set to unlimited to allow extension
	H5::DataSpace signs_dataspace = H5::DataSpace(2,signs_dims,signs_maxdims); // dataspace for data

    	// open the file
    	H5::H5File f(fname,H5F_ACC_TRUNC);
		std::cout << "file opened!" << std::endl;
    	/* set dataset parameters */
		std::cout << "setting parameters for raw data" << std::endl;
    	// set chunking for raw data
    	raw_params.setChunk(2,raw_chunk_dims);
    	// set compression
    	raw_params.setDeflate(6);
    	// fill dataset
    	uint16_t dset_fill_val = 0;
	H5::PredType dtype = H5::PredType::NATIVE_UINT16;
    	raw_params.setFillValue(dtype,&dset_fill_val);
    	// create dataset to hold frames
    	H5::DataSet raw_set = f.createDataSet("raw",dtype,raw_dataspace,raw_params);
    	raw_dataspace = raw_set.getSpace();
    	raw_params.close();

	/* set time series dataset */
	std::cout << "setting parameters for time data" << std::endl;
	// set parameters for dataset
	time_params.setChunk(1,time_chunk_dims);
	// set compression
	time_params.setDeflate(6);
	long long time_fill_val = 0;
	H5::PredType ttype = H5::PredType::NATIVE_LLONG;
	time_params.setFillValue(ttype,&time_fill_val);
	// create dataset to hold time values
	H5::DataSet time_set = f.createDataSet("time",ttype,timespace,time_params);
	timespace = time_set.getSpace();
	time_params.close();

	/* temperature parameters */
	std::cout << "setting parameters for temperature data" << std::endl;
	// modify dataset creation properties to enable chunking
	temp_params.setChunk(3,temp_chunk_dims);
	// set the initial value of the dataset
	static float temp_fill_val = 0.0;
	H5::PredType ftype = H5::PredType::NATIVE_FLOAT;
	temp_params.setFillValue(ftype,&temp_fill_val);
	H5::DataSet temp_set = f.createDataSet("temperature",ftype,temp_dataspace,temp_params);
	temp_params.close();

	/*sign matrix parameters*/
	std::cout << "setting parameters for signs data" << std::endl;
	// set chunking for time series and data set
    	sign_params.setChunk(2,signs_chunk_dims);
    	// set compression
    	sign_params.setDeflate(6);
    	// fill dataset
    	signed short sign_fill_val = 0;
	H5::PredType stype = H5::PredType::NATIVE_SHORT;
    	sign_params.setFillValue(stype,&sign_fill_val);
    	// create dataset to hold frames
    	H5::DataSet signs_set = f.createDataSet("signs",stype,signs_dataspace,sign_params);
    	signs_dataspace = signs_set.getSpace();
    	sign_params.close();

	std::cout << "starting main writing loop" << std::endl;
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
		std::cout << "writing raw data" << std::endl;
		slab = raw_set.getSpace();
		slab.selectHyperslab(H5S_SELECT_SET,raw_chunk_dims,raw_offset);
		// write event camera data
		raw_set.write(frame,dtype,raw_dataspace,slab);

		///// temperature data
		std::cout << "writing temperature data" << std::endl;
		slab = temp_set.getSpace();
		slab.selectHyperslab(H5S_SELECT_SET,temp_chunk_dims,temp_offset);
		// get temperature values
		cam.getTemperature(temp);
		// write temperature to dataset
		temp_set.write(temp,ftype,temp_dataspace,slab);

		///// signs matrix
		std::cout << "writing signs data" << std::endl;
		// get hyperslab
		slab = signs_set.getSpace();
		slab.selectHyperslab(H5S_SELECT_SET,signs_chunk_dims,signs_offset);
		// write event camera data
		signs_set.write(cam.out,stype,signs_dataspace,slab);

		///// log time
		std::cout << "writing time" << std::endl;
		slab = time_set.getSpace();
		slab.selectHyperslab(H5S_SELECT_SET,time_chunk_dims,time_offset);
		// get elapsed time since start in milliseconds
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tt-startt);
		long long dur[1] = {elapsed.count()};
		// write elapsed time to data
		time_set.write(dur,ttype,timespace,slab);

		/* prep for next iteration */
		//// extend temperature dataset
		temp_dimsext[2]+=1;
		temp_set.extend(temp_dimsext);
		temp_offset[2]+=1;

		//// extend raw data dataset
		raw_dimsext[1]+=1;
		raw_set.extend(raw_dimsext);
		raw_offset[1]+=1;
		
		//// extend signs matrix
		signs_dimsext[1]+=1;
		signs_set.extend(signs_dimsext);
		signs_offset[1]+=1;
		
		//// extend time dataset
		time_dimsext[0]+=1;
		time_set.extend(time_dimsext);
		// increase offset for next frame
		time_offset[0]+=1;

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
