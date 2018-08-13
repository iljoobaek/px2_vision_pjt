GCC=gcc
CC=g++
CFLAGS=-Wall -I. -I./glew-1.13.0/include -I/home/nvidia/drive-t186ref-linux/include -I/home/nvidia/drive-t186ref-linux/samples/nvmedia/MOD/winintf -I/home/nvidia/drive-t186ref-linux/samples/nvmedia/utils -L/home/nvidia/opencv-2.4.12/lib -L/home/nvidia/Desktop/glm/glm/ -L/usr/local/lib -L/usr/include/x86-64-linux-gnu/ -L/home/nvidia/drive-t186ref-linux/lib-target -ggdb
#CFLAGS=-Wall -fpermissive -I. -I/home/nvidia/drive-t186ref-linux/samples/nvmedia/MOD/winintf -I/home/nvidia/drive-t186ref-linux/samples/nvmedia/utils -L/home/nvidia/opencv-2.4.12/lib -L/home/nvidia/Desktop/glm/glm/ -L/usr/local/lib -L/usr/include/x86-64-linux-gnu/ -L/home/nvidia/drive-t186ref-linux/lib-target
#CFLAGS=-Wall -fpermissive -I. -L/home/nvidia/opencv-2.4.12/lib -L/home/nvidia/Desktop/glm/glm/ -L/usr/local/lib -L/usr/include/x86-64-linux-gnu/ -L/home/nvidia/drive-t186ref-linux/lib-target

DEPS = shader.hpp texture.hpp objloader.hpp 
OBJ = surround_view.o shader.o texture.o objloader.o eglconsumer_main.o nvmimg_consumer.o eglstrm_setup.o ../utils/buffer_utils.o ../utils/config_parser.o ../utils/log_utils.o ../utils/misc_utils.o ../utils/surf_utils.o ../utils/thread_utils.o winintf/egl_utils.o winintf/win_x11.o
TARGET = surround_view

LIBS= -lGL -lGLEW -lGLU -lGLEW -lGLESv2 -lglfw -lglut -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lavcodec -lavformat -lswscale -lavutil -lnvmedia -lEGL -lX11 -lpthread

LD_PATH=/home/nvidia/drive-t186ref-linux/lib-target

.PHONY = clean

%.o: %.cpp $(DEPS)
	LD_LIBRARY_PATH=$(LD_PATH) $(CC) -c -o $@ $< $(CFLAGS)

%.o: %.c $(DEPS)
	LD_LIBRARY_PATH=$(LD_PATH) $(GCC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	LD_LIBRARY_PATH=$(LD_PATH) $(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	-rm *.o

clean:
	-rm $(TARGET)
	-rm eglconsumer_main.o nvmimg_consumer.o eglstrm_setup.o surround_view.o winintf/*.o
