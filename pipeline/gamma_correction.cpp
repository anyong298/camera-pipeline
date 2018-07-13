#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
#include <float.h> 
#include <math.h>
#include <string>


using namespace Halide::Tools;
using namespace Halide;
using namespace std;

/*
Compile with: g++ gamma_correction.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -o gamma_corr -std=c++11
DYLD_LIBRARY_PATH=../bin ./gamma_corr [file]
DYLD_LIBRARY_PATH=../bin ./gamma_corr demosaic.png
*/

/*
	The bayer filter pattern of test case from the reverse pipeline 
	G R G R G
	B G B G B
	G R G R G
	B G B G B

*/

int main(int argc, char **argv){
	// Buffer<uint8_t> input = Tools::load_image("images/rgr.jpg");
	Buffer<uint8_t> input = load_image("images/" + string(argv[1]));

	// float gamma = 1/2.2;

	Func correct("correct");
	Halide::Var x("x"), y("y"), c;

	Expr value = cast<float>(input(x, y, c));

	// value = 255 * (pow((value / 255.0f), (1/2.2f)));
	value = (pow((value), (1/2.2f)));

	correct(x, y, c) = (cast<uint8_t>(value));


	Buffer<uint8_t> output = correct.realize(input.width(), input.height(), input.channels());

    save_image(output, "gamma_corrected.png");


	return 0;
}