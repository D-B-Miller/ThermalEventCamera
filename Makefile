#I2C_MODE = LINUX
I2C_LIBS = 
#I2C_LIBS = -lbcm2835
EXP_DIR = Scripts/examples/
SRC_DIR = Scripts/src/
HEAD_DIR = Scripts/src/
BUILD_DIR = Scripts/bin/
MLX_DIR = /home/pi/mlx90640-library-master/

examples = eventthermalcamera
examples_objects = $(addsuffix .o,$(addprefix $(EXP_DIR), $(examples))))
examples_output = $(addprefix $(BUILD_DIR), $(examples))

ifeq ($(PREFIX),)
	PREFIX = /usr/local
endif

ifeq ($(I2C_MODE), LINUX)
	I2C_LIBS = 
endif

all: examples

examples: $(examples_output)

$(examples_objects) : CXXFLAGS+=-std=c++17 -Wall -O2

$(examples_output) : CXXFLAGS+=-I$(MLX_DIR) -std=c++17 -Wall -O2


$(BUILD_DIR)eventthermalcamera: $(EXP_DIR)eventthermalcamera.o $(MLX_DIR)libMLX90640_API.a
	$(CXX) -L/home/pi/mlx90640-library-master $^ -o $@ $(I2C_LIBS)

$(BUILD_DIR)tevent_example: $(SRC_DIR)tevent_example.o $(MLX_DIR)libMLX90640_API.a
	$(CXX) -lpthread -I$(SRC_DIR) -L/home/pi/mlx90640-library-master $^ -o $@ $(I2C_LIBS)

clean:
	rm -f $(examples_output)
	rm -f $(EXP_DIR)*.o