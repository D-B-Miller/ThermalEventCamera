#I2C_MODE = LINUX
I2C_LIBS = 
#I2C_LIBS = -lbcm2835
EXP_DIR = Scripts/examples/
SRC_DIR = Scripts/src/
HEAD_DIR = Scripts/src/
BUILD_DIR = Scripts/bin/
MLX_DIR = /home/pi/mlx90640-library-master/

examples = eventthermalcamera tevent_example tevent_compare tevent_stats
examples_objects = $(addsuffix .o,$(addprefix $(EXP_DIR), $(examples)))
examples_output = $(addprefix $(BUILD_DIR), $(examples))

hdf5 = tevent_hdf5_all tevent_hdf5 tevent_hdf5_interp quickhdf5_example
hdf5_objects = $(addsuffix .o,$(addprefix $(EXP_DIR), $(examples)))
hdf5_output = $(addprefix $(BUILD_DIR), $(examples))

ifeq ($(PREFIX),)
	PREFIX = /usr/local
endif

ifeq ($(I2C_MODE), LINUX)
	I2C_LIBS = 
endif

all: examples hdf5

examples: $(examples_output)

$(examples_objects) : CXXFLAGS+=-std=c++17 -O2 -I$(MLX_DIR)
$(examples_output) : CXXFLAGS+=-I$(MLX_DIR) -I$(SRC_DIR) -std=c++17 -Wall -O2

hdf5: $(hdf5_output)

$(hdf5_objects) : CXXFLAGS+=-std=c++17 -O2 -I$(MLX_DIR)
$(hdf5_output) : CXXFLAGS+=-I$(MLX_DIR) -I$(SRC_DIR) -std=c++17 -Wall -O2

$(BUILD_DIR)eventthermalcamera: $(EXP_DIR)eventthermalcamera.o $(MLX_DIR)libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library-master $^ -o $@ $(I2C_LIBS)

$(BUILD_DIR)tevent_example: $(SRC_DIR)teventcamera.cpp $(EXP_DIR)tevent_example.cpp $(MLX_DIR)libMLX90640_API.a
	$(CXX) $(CXXFLAGS) -lpthread -L$(MLX_DIR) $^ -o $@ $(I2C_LIBS)

$(BUILD_DIR)tevent_compare: $(SRC_DIR)teventcamera.cpp $(EXP_DIR)tevent_compare.cpp $(MLX_DIR)libMLX90640_API.a
	$(CXX) $(CXXFLAGS) -lpthread -L$(MLX_DIR) $^ -o $@ $(I2C_LIBS)

$(BUILD_DIR)tevent_stats: $(SRC_DIR)teventcamera.cpp $(EXP_DIR)tevent_stats.cpp $(MLX_DIR)libMLX90640_API.a
	$(CXX) $(CXXFLAGS) -lpthread -L$(MLX_DIR) $^ -o $@ $(I2C_LIBS)

$(BUILD_DIR)tevent_hdf5_all: $(SRC_DIR)teventcamera.cpp $(EXP_DIR)tevent_hdf5_all.cpp $(MLX_DIR)libMLX90640_API.a
	h5c++ $(CXXFLAGS) -lpthread -L$(MLX_DIR) $^ -o $@ $(I2C_LIBS)
	
$(BUILD_DIR)tevent_hdf5: $(SRC_DIR)teventcamera.cpp $(EXP_DIR)tevent_hdf5.cpp $(MLX_DIR)libMLX90640_API.a
	h5c++ $(CXXFLAGS) -lpthread -L$(MLX_DIR) $^ -o $@ $(I2C_LIBS)
	
$(BUILD_DIR)tevent_hdf5_interp: $(SRC_DIR)teventcamera.cpp $(EXP_DIR)tevent_hdf5_interp.cpp $(MLX_DIR)libMLX90640_API.a
	h5c++ $(CXXFLAGS) -lpthread -L$(MLX_DIR) $^ -o $@ $(I2C_LIBS)
	
$(BUILD_DIR)quickhdf5_example: $(SRC_DIR)teventcamera.cpp $(EXP_DIR)quickhdf5_example.cpp $(MLX_DIR)libMLX90640_API.a
	h5c++ $(CXXFLAGS) -lpthread -L$(MLX_DIR) $^ -o $@ $(I2C_LIBS)

clean:
	rm -f $(examples_output)
	rm -f $(EXP_DIR)*.o
