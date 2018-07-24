/* 
   Author: Ziqi Zhang
   University of Rochester
   zzhang73 at u.rochester.edu
*/


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
Compile with: g++ demosaic.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -o demosaic -std=c++11
DYLD_LIBRARY_PATH=../bin ./demosaic 
*/

/*
	The bayer filter pattern of test case from the reverse pipeline 
	G R G R G
	B G B G B
	G R G R G
	B G B G B

*/

int main(int argc, char **argv){
	// Buffer<uint8_t> input = Tools::load_image("images/rgb.jpg");
	Halide::Buffer<uint8_t> input = load_image("images/" + string(argv[1]));
	// Halide::Buffer<uint8_t> output(input.width(), input.height());
	// input.set_min(1, 1);
	//First find the brightest pixel

	float max = -1, mag, max_r, max_g, max_b;
	uint8_t offset_r, offset_g, offset_b;

	Halide::Func demosaic("demosaic");
	// demosaic.trace_stores();

	Var x("x"), y("y"), c("c");
	
	// Halide::Expr value;

	demosaic(x, y, c) = input(x, y, c);

	//Red

	RDom r(1, input.width() - 2, 1, input.height() - 2);

	
	r.where(r.x % 2 == 0);
	r.where(r.y % 2 == 0);

	demosaic(r.x, r.y, 1) = (input(r.x, r.y + 1, 1) / 4 + input(r.x, r.y - 1, 1) / 4 + input(r.x + 1, r.y, 1) / 4 + input(r.x - 1, r.y, 1) / 4);
	demosaic(r.x, r.y, 2) = (input(r.x + 1, r.y + 1, 2) / 4 + input(r.x + 1, r.y - 1, 2) / 4 + input(r.x - 1, r.y + 1, 2) / 4 + input(r.x - 1, r.y - 1, 2) / 4);


	//Blue

	RDom b(1, input.width() - 2, 1, input.height() - 2);

	b.where(b.x % 2 == 1);
	b.where(b.y % 2 == 1);

	demosaic(b.x, b.y, 0) = (input(b.x + 1, b.y + 1, 0) / 4 + input(b.x - 1, b.y + 1, 0) / 4 + input(b.x + 1, b.y - 1, 0) / 4 + input(b.x - 1, b.y - 1, 0) / 4);
	demosaic(b.x, b.y, 1) = (input(b.x + 1, b.y, 1) / 4 + input(b.x, b.y + 1, 1) / 4 + input(b.x - 1, b.y, 1) / 4 + input(b.x, b.y - 1, 1) / 4);

	//Green 01

	RDom g1(1, input.width() - 2, 1, input.height() - 2);
	g1.where(g1.x % 2 == 0);
	g1.where(g1.y % 2 == 1);

	demosaic(g1.x, g1.y, 0) = (input(g1.x, g1.y + 1, 0) / 2 + input(g1.x, g1.y - 1, 0) / 2);
	demosaic(g1.x, g1.y, 2) = (input(g1.x + 1, g1.y, 2) / 2 + input(g1.x - 1, g1.y, 2) / 2);


	//Green 02

	RDom g2(1, input.width() - 2, 1, input.height() - 2);
	g2.where(g2.x % 2 == 1);
	g2.where(g2.y % 2 == 0);

	demosaic(g2.x, g2.y, 0) = (input(g2.x + 1, g2.y, 0) / 2 + input(g2.x - 1, g2.y, 0) / 2);
	demosaic(g2.x, g2.y, 2) = (input(g2.x, g2.y + 1, 2) / 2 + input(g2.x, g2.y - 1, 2) / 2);

	Halide::Buffer<uint8_t> output =
        demosaic.realize(input.width(), input.height(), input.channels());

    save_image(output, "balanced.png");


	return 0;
}