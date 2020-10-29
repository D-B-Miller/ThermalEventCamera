import threading
import os
import pigpio
import sys
from datetime import time as timestruct
from datetime import datetime
import dataclasses
import struct

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

class ThermalPig:
	def __init__(self,start_daemon=True):
                # 
		self.pi = pigpio.pi()
		if not self.pi.connected:
			print("failed to get pi")
			if start_daemon:
				print("starting pigpiod daemon")
				os.system('sudo pigpiod')
				self.pi  = pigpio.pi()
				if not self.pi.connected:
					print("failed to get pi after creating daemon. check host name")
					return
		else:
			self.__h = self.pi.i2c_open(1,0x33)
			# perform a test read
			try:
				self.pi.i2c_read_device(self.__h,1)
			except BaseException:
				print("Unexpected error during test read ",sys.exc_info()[0])
		# setup thread
		self.__thread = threading.Thread(target=ThermalPig.update,args=(self,),daemon=True)
		# data matrices
		self.out = [EventData()]*832
		self.__last = None
		# stop flag
		self.__stop = False
        # overloaded subtraction operator to handle bytearrays
	def __sub__(a : bytearray,b : bytearray):
            if len(a) != len(b):
                raise ValueError("Cannot subtract arrays of two different lengths!")
            else:
                return bytearray([aa-bb for aa,bb in zip(a,b)])

	def start(self):
            self.__stop = False
            self.__thread.start()

        def stop(self):
            self.__stop = True
            self.__thread.join(5.0)

        def update(self):
            while not self.__stop:
                try:
                    data = self.pi.i2c_read_device(self.__h,1664)
                    if self.__last is None:
                        continue
                    else:
                        diff = data - self.__last
                        # unpack to 

        def __getattr__(self,*args):
                return self.pi.__getattr__(*args)

	def __enter__(self):
                return self

        def __exit__(self):
                self.pi.close()

