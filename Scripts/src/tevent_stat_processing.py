import h5py
import numpy as np
import matplotlib as plt
from scipy.ndimage import gaussian_filter
path = r"D:\CoronaWork\Scripts\ThermalEventCamera\ThermalEventCamera\tevent_stats.hdf5"

def basic_stats():
    with h5py.File(path,'r') as file:
        dd = file["tevent"]
        print(dd.shape)
        # basic stats
        dmax = np.amax(dd[()],0)
        print("========= basic stats =========")
        print(f"max 0: {dmax}, shape {dmax.shape}")
        print(f"global {np.amax(dd[()],axis=(0,1))}")
        dmin = np.amin(dd[()],0)
        print(f"min 0: {dmin}, shape {dmin.shape}")
        print(f"global {np.amin(dd[()],axis=(0,1))}")
        dmean = np.mean(dd[()],0)
        print(f"mean 0: {dmean}, shape {dmean.shape}")
        print(f"global {np.mean(dd[()],axis=(0,1))}")
        dstd = np.std(dd[()],0)
        print(f"std 0: {dstd}, shape {dstd.shape}")
        print(f"global {np.std(dd[()],axis=(0,1))}")

def log_stats():
    with h5py.File(path,'r') as file:
        dd = file["tevent"]
        print(dd.shape)
        # log stats
        dlog = np.log(dd[()]+1)
        # mask for inf
        mask = np.isfinite(dlog)
        print("========= log stats =========")
        dmax = np.amax(dlog[mask],0)
        print(f"log max 0: {dmax}, shape {dmax.shape}")
        dmin = np.amin(dlog[mask],0)
        print(f"log min 0: {dmin}, shape {dmin.shape}")
        dmean = np.mean(dlog[mask],0)
        print(f"log mean 0: {dmean}, shape {dmean.shape}")
        dstd = np.std(dlog[mask],0)
        print(f"log std 0: {dstd}, shape {dstd.shape}")

def log_cmp_test():
    with h5py.File(path,'r') as file:
        dd = file["tevent"]
        dlog = 65535*np.log(dd[()]+1)
        # mask for inf
        mask = np.isfinite(dlog)
        print("========= log stats =========")
        dmax = np.amax(dlog[mask],0)
        print(f"log max 0: {dmax}, shape {dmax.shape}")
        dmin = np.amin(dlog[mask],0)
        print(f"log min 0: {dmin}, shape {dmin.shape}")
        dmean = np.mean(dlog[mask],0)
        print(f"log mean 0: {dmean}, shape {dmean.shape}")
        dstd = np.std(dlog[mask],0)
        print(f"log std 0: {dstd}, shape {dstd.shape}")

def gauss_filter():
    with h5py.File(path,'r') as file:
        do = file["tevent"][()]
        print(f"orig shape {do.shape}")
        dd = do[:832,:].reshape((26,32,-1))
        print(f"new shape {dd.shape}")
        ft = np.zeros(dd.shape,dtype=dd.dtype)
        gstd = np.std(do[()],axis=(0,1))
        print(f"global std {gstd}")
        for ii in range(dd.shape[2]):
            ft[:,:,ii] = gaussian_filter(dd[:,:,ii],sigma=gstd)
    return ft
        
if __name__ == "__main__":
    #basic_stats()
    #log_stats()
    #log_cmp_test()
    filt = gauss_filter()
