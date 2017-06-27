
# compiler
CXX :=g++ -Wall -w -O3 -msse2 -fPIC -ggdb -pthread -std=c++11
CC :=gcc -lm -Wall -w -O3 -msse2 -fPIC -ggdb -pthread -std=c++11

#CXX :=g++ -fPIC 
#CC :=gcc -fPIC 

#shared library
SFLAG := -fPIC -shared

# include && lib
INCLUDE = -I/usr/local/include/opencv -I/usr/local/include/opencv2 -I/usr/local/include -I/usr/local/cuda/include
LIBARAY = -L./bin -L/usr/local/lib -L/usr/local/lib64 -L/usr/local/cuda/lib64 -L/usr/lib -L/usr/lib64

# flags
CFLAGS =  -fopenmp  -DHAVE_JPEG -DHAVE_PNG
CXXFLAGS = -fopenmp  -DHAVE_JPEG -DHAVE_PNG \
	 $(INCLUDE) 
#$(APIEXPAND_INC)

LDFLAGS  = -lm -lpthread -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_ml
LDFLAGS  += -lcublas_device -lcublas -lcudadevrt -lcudart  -lcufft -lcufftw -lculibos -lcurand -lcusparse -lnppc -lnppi -lnpps
LDFLAGS  += -ldl -lrt -ljpeg -lpng  -lz  -lstdc++ -lglog -lboost_system

# -lopenblas
# -lcurl

# BIN && LIB
BIN = ./bin
SRC = ./src

###### generation rules
#.SUFFIXES: .cpp .c .o .so .a .d
.c.o:
	@echo ----------------------------------------------
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ----------------------------------------------

.cpp.o:
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo ----------------------------------------------


###### main
#.PHONY : all 
all: lib Demo_expandImg

#deps: $(addprefix $(OBJSPATH),$(DEPS))
Demo_expandImg:$(BIN) $(SRC)/Demo_expandImg.cpp
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) $(SRC)/Demo_expandImg.cpp  -o $(BIN)/Demo_expandImg -lmainboby $(LIBARAY) $(LDFLAGS)
	@echo ----------------------------------------------

lib: 
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) -shared -o -fPIC -o $(BIN)/libmainboby.so  $(LIBARAY) $(LDFLAGS)
	@echo ----------------------------------------------

clean:
	rm -f $(TARGET)
	rm $(BIN)/libmainboby
	rm -f *.o 
	rm $(BIN)/Demo_expandImg

rebuild: clean all
install:
	install -T $(TARGET) $(INSTALL_PATH)/bin/$(TARGET)
