import threading
import os
import pigpio
import sys

class ThermalPig:
	def __init__(self,start_daemon=True):
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

