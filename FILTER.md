# Designing filter functions for the class
The class ThermalEventCamera, defined [here](Scripts/src/teventcamera.h), supports a custom thresholding function used for comparing the pixels and generating the event data. By default, instances of EventData are created and added to the queue when the pixel values are different. The GIF below shows the signs matrix using this default function.

![](vids/eventcamera-test-vncviewer-na.gif)

The grouping of red cells seen is the author's hand being waved passed the sensor. Unfortunately, reacting to every change ruins the boundary of the hand making it difficult to differentiate from the noise. In order to design a better filter function, the statistics of the noise floor needs to be investigated to understand how activity is perceived.

The plots and statistics were generated using [this](Scripts/src/tevent_stat_processing.py) script.

The data used was recorded using the [tevent_hdf5](Scripts/examples/tevent_hdf5.cpp) and [tevent_hdf5_interp](Scripts/examples/tevent_hdf5_interp.cpp)

## Basic statistics
