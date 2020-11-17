#include "teventcamera.h"
#include "H5Cpp.h"
#include <iostream>

int main()
{
	// initialise thermal camera
	std::cout << "initialising camera" << std::endl;
	ThermalEventCamera cam(32);
	// return value
	// non-zero indicates an error
	int ret = 0;
	int tlim = 600; // time limit
	std::cout << "time limit set to " << tlim << " secs" << std::endl;
	// setup hdf5 file
	std::cout << "opening hdf5 file tevent_stats_interp.hdf5" << std::endl;
	H5::H5File file("tevent_stats_interp.hdf5",H5F_ACC_TRUNC);
	// dataset sizes
	hsize_t dimsext[2] = {832,1};
	// starter dimensions
	hsize_t dims[2] = {832,1};
	// max size
	hsize_t maxdims[2] = {832,H5S_UNLIMITED};
	// size of chunks
	hsize_t chunk_dims[2] = {832,1};
	// offset of hyperslab
	hsize_t offset[2] = {0,0};
	// dataspace for dataset
	H5::DataSpace dataspace(2,dims,maxdims);
	// dataspace for hyperslab
	H5::DataSpace slab;
	// enable chunking
	H5::DSetCreatPropList cparams;
	cparams.setChunk(2,chunk_dims);
	// set compression to save space
	std::cout << "Setting compression of file" << std::endl;
	cparams.setDeflate(6);
	// set initial value
	auto dtype = H5::PredType::NATIVE_UINT16;
	uint16_t fill_val = 0;
	cparams.setFillValue(dtype,&fill_val);
	// create dataset
	H5::DataSet dataset = file.createDataSet("tevent",dtype,dataspace,cparams);
	// close parameters
	cparams.close();
	// frame for storage
	uint16_t frame[832] = {0};
	// start time
	auto start = std::chrono::system_clock::now();
	// main loop
	// running camera unthreaded
	std::cout << "starting main loop" << std::endl;
	while(1)
	{
		try{
			cam.read(); // read frame
			cam.interpOutliers(frame); // get a copy of it
			// get hyperslab
			slab = dataset.getSpace();
			slab.selectHyperslab(H5S_SELECT_SET,chunk_dims,offset);
			// write data
			dataset.write(frame,dtype,dataspace,slab);
			/* prep for next iteration */
			// extend dataset
			dimsext[1]+=1;
			dataset.extend(dimsext);
			// increase offset for next frame
			offset[1]+=1;
			// get elapsed time
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now()-start);
			// check elapsed time
			if(elapsed.count()>tlim){
				std::cout << "Reached time limit" << std::endl;
				break;
			}
		}catch(H5::FileIException error){
			std::cerr << "HDF5 File Exception! Closing file!" << std::endl;
			error.printErrorStack();
			ret = 1;
			break;
		}catch(H5::DataSetIException error){
			std::cout << "HDF5 Dataset Exception! Closing file" << std::endl;
			error.printErrorStack();
			ret = 2;
			break;
    		}catch(H5::DataSpaceIException error){
			std::cout << "HDF5 Dataspace Exception! Closing file" << std::endl;
			error.printErrorStack();
			ret = 3;
			break;
    		}
	}
	/* cleanup */
	std::cout << "cleaning up" << std::endl;
	// close file
	// becomes inaccessible otherwise
	file.close();
	return ret;
}
