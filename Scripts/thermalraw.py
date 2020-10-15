import threading
from datetime import time as timestruct
from datetime import datetime
import dataclasses

# structure for the event data
# holds the sign of the change and the time of update
@dataclasses.dataclass
class EventData:
    """
        dataclass for holding the event data generated by an ThermalRaw
        device
    """
    sign : int
    ut : datetime.time

    def __init__(self,sign:int = 0):
        """
            Constructor for EventData. Accepts a sign value and sets the update
            time ut using datetime.datetime.now().time()

            sign (int) : Sign of the data change. Should be -1,0 or 1 to represent
                        negative, no or positive change in data repsectively.
        """
        self.sign = sign
        self.ut = datetime.now().time()

# class for reading raw data from a i2c device in a threaded manner
# based off event cameras
class ThermalRaw:
    """
        Class for reading raw data from an MLX90640 device connected to
        the Raspberry Pi I2C pins, specifically the device /dev/i2c-1
    """    
    def __init__(self):
        """
            ThermalRaw constructor. Attempts to open a connection with
            /dev/i2c-1 in rb mode to read raw bytes from a thermal camera
            at that location. Changes in the data are logged as EventData
            instances in the class variable out

            No arguments required
        """
        self.dev = None
        # attempt connection to device
        try:
            self.dev = open('/dev/i2c-1','rb')
        except FileNotFoundError as err:
            print(err)
        # if device was created or is closed to a failure to open it
        # print message informing user
        if (self.dev is None) or self.dev.closed:
            print("Failed to open target device!")
        # size to read
        self.__size = 832
        # thread for reading
        self.__thread = threading.Thread(target=self.update,args=(self,))
        # start time for reference
        self.__startt = 0
        # output matrix
        self.data = None
        # last data frame
        self.__last = None
        # output event data
        self.out = [EventData()] * 832
        # only signs of self.out
        self.signs = [0] * 832
        # flag for stopping read thread
        self.__stop = False

    # with keyword handler
    # starts threaded reading behaviour
    def __enter__(self):
        self.start()
        return self

    # with exit handler
    # ensures the devce is closed and any cleanup
    def __exit__(self,tt,value,traceback):
        if (self.dev is not None) and not self.dev.closed:
            self.dev.close()

    # start thread
    def start():
        """
            Start threaded reading of the device
        """
        self.__stop = False
        self.__thread.start()

    # stop thread
    def stop():
        """
            Set stop flag for thread and wait for it to finish
        """
        self.__stop = True
        self.__thread.join()
        
    # read and update output matrix
    def update(self):
        """
            Read from device if it's open and update the out and signs
            array

            The class runs this program in a thread for continuous updates
        """
        # loop while flag is false
        while(not self.__stop):
            # if the device is closed break from loop
            if self.dev.closed:
                print("Cannot read from device! Device closed!")
                return
            else:
                # read data
                self.data = self.dev.read(self.__size)
                # if this is the first time then just update the last frame
                # as a copy of the current frame
                if not self.__last:
                    self.__last = self.data.copy()
                    continue
                else:
                    # iterate over current data and last frame
                    for ii,ll,dd in zip(range(self.__size),self.__last,self.data):
                        # calculate difference
                        diff = dd-ll
                        # set sign and update output matrix
                        if diff == 0:
                            self.out[ii] = EventData(0)
                            self.signs[ii] = 0
                        elif diff>0:
                            self.out[ii] = EventData(1)
                            self.signs[ii] = 1
                        elif diff<0:
                            self.out[ii] = EventData(-1)
                            self.signs[ii] = -1
                        # update last frame
                        self.__last = self.data.copy()
