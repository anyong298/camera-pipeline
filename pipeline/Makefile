temporal: temporal.cpp heap.cpp aknn.cpp optical_flow.cpp
	g++ temporal.cpp heap.cpp aknn.cpp optical_flow.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -o executables/temporal -std=c++11 `pkg-config opencv --cflags --libs` -Wall -Werror


ssd: ssd.cpp
	g++ ssd.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -o executables/ssd -std=c++11 -Wall -Werror


pipeline: pipeline.cpp main.cpp
	g++ pipeline.cpp main.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -o executables/pipeline -std=c++11 -Wall -Werror


