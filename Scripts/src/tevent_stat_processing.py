import h5py
import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import gaussian_filter
from matplotlib.widgets import Slider
from matplotlib import cm
import matplotlib.animation as animation

# paths for data files
p1 = r"D:\CoronaWork\Scripts\ThermalEventCamera\ThermalEventCamera\tevent_stats.hdf5"
p2 = r"D:\CoronaWork\Scripts\ThermalEventCamera\ThermalEventCamera\tevent_stats_interp.hdf5"
# target path for file
path = p1

# print basic stats of tevent dataset in target path
def basic_stats():
    with h5py.File(path,'r') as file:
        dd = file["tevent"]
        print(dd.shape)
        # basic stats
        dmax = np.amax(dd[()],axis=(0,1))
        print("========= basic stats =========")
        print(f"max 0: {dmax}, shape {dmax.shape}")
        print(f"global {np.max(dd[()])}")
        dmin = np.amin(dd[()],0)
        print(f"min 0: {dmin}, shape {dmin.shape}")
        print(f"global {np.min(dd[()])}")
        dmean = np.mean(dd[()],0)
        print(f"mean 0: {dmean}, shape {dmean.shape}")
        print(f"global {np.mean(dd[()])}")
        dstd = np.std(dd[()],0)
        print(f"std 0: {dstd}, shape {dstd.shape}")
        print(f"global {np.std(dd[()])}")

# print stats about the data after natural log
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
        dmax = np.amax(dlog[mask],axis=(0,1))
        print(f"log max 0: {dmax}, shape {dmax.shape}")
        dmin = np.amin(dlog[mask],0)
        print(f"log min 0: {dmin}, shape {dmin.shape}")
        dmean = np.mean(dlog[mask],0)
        print(f"log mean 0: {dmean}, shape {dmean.shape}")
        dstd = np.std(dlog[mask],0)
        print(f"log std 0: {dstd}, shape {dstd.shape}")

# apply a gaussian filter based on the global standard deviation
def gauss_filter(idx=-1):
    with h5py.File(path,'r') as file:
        do = file["tevent"][()]
        print(f"orig shape {do.shape}")
        sh = do[:832,:].reshape((26,32,-1)).shape
        dd = np.zeros(sh,dtype=do.dtype)
        for ii in range(sh[2]):
            dd[:,:,ii]=do[:832,ii].reshape((26,32))
        print(f"new shape {dd.shape}")
        ft = np.zeros(dd.shape,dtype=dd.dtype)
        gstd = np.std(do)
        print(f"global std {gstd}")
        if idx==-1:
            for ii in range(dd.shape[2]):
                print(ii)
                ft[:,:,ii] = gaussian_filter(dd[:,:,ii],sigma=gstd)
            return ft
        else:
            try:
                iter(idx)
                ft = np.zeros((*dd.shape[:2],len(idx)),dtype=do.dtype)
                for i,ii in zip(range(len(idx)),idx):
                    ft[:,:,i] = gaussian_filter(dd[:,:,ii],sigma=gstd)
                return ft
            except TypeError:
                return gaussian_filter(dd[:,:,idx],sigma=gstd)

# filter data on a fixed threshold
def thresh_filt(t=60000,verbose=True):
    with h5py.File(path,'r') as file:
        do = file["tevent"][()]
        print(f"orig shape {do.shape}")
        sh = do[:832,:].reshape((26,32,-1)).shape
        dd = np.zeros(sh,dtype=do.dtype)
        for ii in range(sh[2]):
            dd[:,:,ii]=do[:832,ii].reshape((26,32))
    dd[dd>=t] = 0
    dd+=1
    dlog = np.log(dd)
    # mask for inf
    mask = np.isfinite(dlog)
    if verbose:
        print(f"filt shape {dd.shape}")
        # basic stats
        dmax = np.amax(dd,axis=(0,1))
        print("========= basic stats =========")
        print(f"max 0: {dmax}, shape {dmax.shape}")
        print(f"global {np.max(dd)}")
        dmin = np.amin(dd,axis=(0,1))
        print(f"min 0: {dmin}, shape {dmin.shape}")
        print(f"global {np.min(dd)}")
        dmean = np.mean(dd,axis=(0,1))
        print(f"mean 0: {dmean}, shape {dmean.shape}")
        print(f"global {np.mean(dd)}")
        dstd = np.std(dd,axis=(0,1))
        print(f"std 0: {dstd}, shape {dstd.shape}")
        print(f"global {np.std(dd)}")
        # log stats
        print("========= log stats =========")
        dmax = np.amax(dlog[mask],0)
        print(f"log max 0: {dmax}")
        dmin = np.amin(dlog[mask],0)
        print(f"log min 0: {dmin}")
        dmean = np.mean(dlog[mask],0)
        print(f"log mean 0: {dmean}")
        dstd = np.std(dlog[mask],0)
        print(f"log std 0: {dstd}")
    dlog[mask] = 0
    return dd,dlog

def get_data(reshape=True):
    with h5py.File(path,'r') as file:
        if reshape:
            return file["tevent"][()][:832,:].reshape((26,32,-1))
        return file["tevent"][()]

# plot given data as a surface with a slider to change data index
def plot_surface(data):
    fig = plt.figure()
    ax = fig.gca(projection='3d')
    # data
    X = np.arange(0,data.shape[0],1)
    Y = np.arange(0,data.shape[1],1)
    X,Y = np.meshgrid(Y,X)
    # generate plot
    surf = [ax.plot_surface(X,Y,data[:,:,0],cmap=cm.coolwarm,
                           linewidth=0,antialiased=False)]
    # set limit on axis
    ax.set_zlim(data.min(),data.max())
    # axes for slider
    axcolor = 'lightgoldenrodyellow'
    axidx = plt.axes([0.25, 0.15, 0.65, 0.03], facecolor=axcolor)
    sidx = Slider(axidx, 'Idx',0,data.shape[2]-1,valinit=0, valstep=1)

    def update(val,surf=surf):
        ii = sidx.val
        print(f"new idx {ii}")
        ax.clear()
        #remove plot
        ax.plot_surface(X,Y,data[:,:,ii],cmap=cm.coolwarm,
                               linewidth=0,antialiased=False)
        #plt.gcf().canvas.draw_idle()
        plt.draw()
    # set slider update function
    sidx.on_changed(update)
    # show plot
    plt.show()

# plot data as contourf with index slide
def plot_contourf(data):
    print(f"data shape {data.shape}")
    fig = plt.figure()
    ax = fig.add_subplot(111)
    # generate plot
    surf = [ax.contourf(data[:,:,0],cmap=cm.coolwarm)]
    # axes for slider
    axcolor = 'lightgoldenrodyellow'
    axidx = plt.axes([0.25, 0.15, 0.65, 0.03], facecolor=axcolor)
    sidx = Slider(axidx, 'Idx',0,data.shape[2]-1,valinit=0, valstep=1)

    def update(val,ax=ax,fig=fig):
        ii = sidx.val
        #print(f"new idx {ii}")
        #surf[0].remove()
        ax.clear()
        #remove plot
        ax.contourf(data[:,:,ii],cmap=cm.coolwarm)
        fig.canvas.draw_idle()
        plt.draw()
    # set slider update function
    sidx.on_changed(update)
    # show plot
    plt.show()

# save data as a contourf animation
def save_contourf(data,fname="tevent-contourf.mp4"):
    print(data.shape)
    fig,ax = plt.subplots()
    FFMpegWriter = animation.writers['ffmpeg']
    metadata = dict(title='event camera', artist='Matplotlib')
    writer = FFMpegWriter(fps=30, metadata=metadata)
    def animate(i):
        ax.clear()
        ff = ax.contourf(data[:,:,i],cmap=cm.coolwarm)
        return ff,
    ani = animation.FuncAnimation(fig,animate,frames=data.shape[2],interval=50,blit=False,repeat=False)
    ani.save(fname)
