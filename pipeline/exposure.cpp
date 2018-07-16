#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
#include <float.h> 
#include <math.h>
#include <string>


using namespace Halide::Tools;
using namespace Halide;

/*
Compile with: g++ exposure.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -o expo -std=c++11
DYLD_LIBRARY_PATH=../bin ./expo balanced.png 7.1
*/
float slope[3];

int main(int argc, char **argv){
	// Buffer<uint8_t> input = Tools::load_image("images/rgb.jpg");
	Halide::Buffer<uint8_t> input = load_image("images/" + std::string(argv[1]));

	//First find the brightest pixel

	double exposureCompensation;

	sscanf(argv[2], "%lf", &exposureCompensation);

	Halide::Func exposure("exposure");

	Halide::Var x("x"), y("y"), c;

	Expr value = input(x, y, c);

	value = cast<float>(value);

	value = value * (float)(std::pow(2.0, exposureCompensation));

	exposure(x, y, c) = cast<uint8_t>(min(value, 255.0f));

	Halide::Buffer<uint8_t> output =
        exposure.realize(input.width(), input.height(), input.channels());

    save_image(output, std::string(argv[1]) + "_exposureAdjusted.png");

	return 0;
}