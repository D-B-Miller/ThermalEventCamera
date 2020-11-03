import sys
import os
sys.path.append(os.path.realpath('../'))
from src import thermalpig

def test_read():
	dev = thermalpig.ThermalPig(restart_daemon=True)
	for i in range(10):
		print(dev.update())

if __name__ == "__main__":
	test_read()
