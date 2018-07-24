#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
#include <float.h> 
#include <math.h>
#include <string>


using namespace Halide::Tools;
using namespace Halide;
/*
Compile with: g++ white_balance.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -o wb -std=c++11
DYLD_LIBRARY_PATH=../bin ./wb [-File] [Weight01] [Weight02] [Weight03]
*/
float slope[3];

int main(int argc, char **argv){
	// Buffer<uint8_t> input = Tools::load_image("images/rgb.jpg");
	Halide::Buffer<uint8_t> input = load_image("images/" + std::string(argv[1]));

	//First find the brightest pixel

	Func magnitude("magnitude");
	Var i, j;

	magnitude(i, j) = sqrt(input(i, j, 0) * input(i, j, 0) + input(i, j, 1) * input(i, j, 1) + input(i, j, 2) * input(i, j, 2));
	Buffer<uint8_t> mag = magnitude.realize(input.width(), input.height());

	float max = -1, tmp, max_r, max_g, max_b;
	int w1 = 1, w2 = 1, w3 = 1;

	// sscanf(argv[2], "%d", &w1);
	// sscanf(argv[3], "%d", &w2);
	// sscanf(argv[4], "%d", &w3);

	// printf("%d\n", input(0, 0, 1));

	for(int i = 1; i < input.width() - 1; i++){
		for(int j = 1; j < input.height() - 1; j++){
			tmp = mag(i, j) * w1 + (mag(i + 1, j) + mag(i, j + 1) + mag(i - 1, j) + mag(i, j - 1) ) / 4 * w2 + (mag(i - 1, j - 1) + mag(i + 1, j + 1) + mag(i + 1, j - 1) + mag(i - 1, j + 1)) / 4 * w3; //get teh magnitude of current RGB vector
			if(max < tmp){// if current RGB is bigger(brighter), update max values 
				max = tmp;
				max_r = input(i, j, 0);
				max_g = input(i, j, 1);
				max_b = input(i, j, 2);
				printf("Current brightest pixel is R: %f, G: %f, B:%f\n", max_r, max_g, max_b);
			}
		}
	}
	printf("Current brightest pixel is R: %f, G: %f, B:%f\n", max_r, max_g, max_b);

	slope[0] = 255 / max_r;
	slope[1] = 255 / max_g;
	slope[2] = 255 / max_b;

	Halide::Func white_balance("white_balance");

	Var x("x"), y("y"), c("c");

	white_balance(x, y, c) = input(x, y, c);
	white_balance(x, y, 0) = cast<uint8_t>(Halide::min(input(x, y, 0) * slope[0], 255.0f));
	white_balance(x, y, 1) = cast<uint8_t>(Halide::min(input(x, y, 1) * slope[1], 255.0f));
	white_balance(x, y, 2) = cast<uint8_t>(Halide::min(input(x, y, 2) * slope[2], 255.0f));

	Halide::Buffer<uint8_t> output =
        white_balance.realize(input.width(), input.height(), input.channels());

    save_image(output, std::string(argv[1]) + "_balanced_" + std::string(argv[2]) + std::string(argv[3]) + std::string(argv[4]) + ".png");

	return 0;
}