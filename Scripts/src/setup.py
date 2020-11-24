from distutils.core import setup, Extension

teventcamera_mod = Extension('_teventcamera',
				include_dirs = ['/home/pi/mlx90640-library-master'],
				library_dirs = ['/home/pi/mlx90640-library-master'],
				sources=['/home/pi/mlx90640-library-master/libMLX90640_API.a','teventcamera.cpp'])

setup (name='teventcamera',
	version = '0.1',
	author = "D B Miller",
	description = "Python bindings for teventcamera",
	ext_modules = [teventcamera_mod],
	py_modules = ["teventcamera"])
