clang -fopenmp \
	./client/cvSocketOpt.c demo.cpp main.cpp\
	-o demo \
	-lstdc++ -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs \
	-I./ -I./client/
