/* 
 * Author: Oliver Ziqi Zhang
 * University of Rochester
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
Compile with: g++ demosaic_2.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -o demosaic -std=c++11
DYLD_LIBRARY_PATH=../bin ./demosaic2 [file]
DYLD_LIBRARY_PATH=../bin ./demosaic2 raw02.png
*/

/*
	The bayer filter pattern of test case from the reverse pipeline 
	R G R G R
	G B G B G
	R G R G R
	G B G B G

*/

int main(int argc, char **argv){
	Buffer<uint8_t> input0 = load_image("images/" + string(argv[1]));

	float max = -1, mag, max_r, max_g, max_b;

	Var x("x"), y("y"), c("c");

	Func castutoi("casting");
	castutoi(x, y, c) = cast<int16_t>(input0(x, y, c));
	Buffer<int16_t> input = castutoi.realize(input0.width(), input0.height(), input0.channels());

	// demosaic.trace_stores();



	//Fill Green to Blue tile

	Func demosaicG("Green");

	demosaicG(x, y, c) = input(x, y, c);

	RDom bg(2, input.width() - 4, 2, input.height() - 4);

	bg.where(bg.x % 2 == 0);
	bg.where(bg.y % 2 == 1);

	Expr b2g_n, b2g_s, b2g_w, b2g_e, omega_n, omega_e, omega_s, omega_w, result;
	b2g_n = abs(input(bg.x, bg.y + 1, 1) - input(bg.x, bg.y - 1, 1)) + abs(input(bg.x, bg.y, 2) - input(bg.x, bg.y - 2, 2));
	b2g_e = abs(input(bg.x - 1, bg.y, 1) - input(bg.x + 1, bg.y, 1)) + abs(input(bg.x, bg.y, 2) - input(bg.x + 2, bg.y, 2));
	b2g_s = abs(input(bg.x, bg.y + 1, 1) - input(bg.x, bg.y - 1, 1)) + abs(input(bg.x, bg.y, 2) - input(bg.x, bg.y + 2, 2));
	b2g_w = abs(input(bg.x - 1, bg.y, 1) - input(bg.x + 1, bg.y, 1)) + abs(input(bg.x, bg.y, 2) - input(bg.x - 2, bg.y, 2));

	omega_n = 1/(b2g_n + 1.0f);
	omega_e = 1/(b2g_e + 1.0f);
	omega_s = 1/(b2g_s + 1.0f);
	omega_w = 1/(b2g_w + 1.0f);

	b2g_n = input(bg.x, bg.y - 1, 1) + (input(bg.x, bg.y, 2) - input(bg.x, bg.y - 2, 2)) / 2.0f;//G2 + (B5 - B1)/2
	b2g_e = input(bg.x + 1, bg.y, 1) + (input(bg.x, bg.y, 2) - input(bg.x + 2, bg.y, 2)) / 2.0f;//G6 + (B5 - B7)/2
	b2g_s = input(bg.x, bg.y + 1, 1) + (input(bg.x, bg.y, 2) - input(bg.x, bg.y + 2, 2)) / 2.0f;//G8 + (B5 - B9)/2
	b2g_w = input(bg.x - 1, bg.y, 1) + (input(bg.x, bg.y, 2) - input(bg.x - 2, bg.y, 2)) / 2.0f;//G4 + (B5 - B3)/2

	//put the value into green (x, y, 1)
	result = (b2g_n * omega_n + b2g_w * omega_w + b2g_s * omega_s + b2g_e * omega_e)/(omega_w + omega_s + omega_e + omega_n);
	demosaicG(bg.x, bg.y, 1) = cast<int16_t>(result);

	//Fill Green to Red tile

	RDom rg(2, input.width() - 4, 2, input.height() - 4);

	rg.where(rg.x % 2 == 1);
	rg.where(rg.y % 2 == 0);

	Expr r2g_n, r2g_s, r2g_w, r2g_e;
	r2g_n = abs(input(rg.x, rg.y + 1, 1) - input(rg.x, rg.y - 1, 1)) + abs(input(rg.x, rg.y, 0) - input(rg.x, rg.y - 2, 0));
	r2g_e = abs(input(rg.x - 1, rg.y, 1) - input(rg.x + 1, rg.y, 1)) + abs(input(rg.x, rg.y, 0) - input(rg.x + 2, rg.y, 0));
	r2g_s = abs(input(rg.x, rg.y + 1, 1) - input(rg.x, rg.y - 1, 1)) + abs(input(rg.x, rg.y, 0) - input(rg.x, rg.y + 2, 0));
	r2g_w = abs(input(rg.x - 1, rg.y, 1) - input(rg.x + 1, rg.y, 1)) + abs(input(rg.x, rg.y, 0) - input(rg.x - 2, rg.y, 0));

	omega_n = 1/(r2g_n + 1.0f);
	omega_e = 1/(r2g_e + 1.0f);
	omega_s = 1/(r2g_s + 1.0f);
	omega_w = 1/(r2g_w + 1.0f);

	r2g_n = input(rg.x, rg.y - 1, 1) + (input(rg.x, rg.y, 0) - input(rg.x, rg.y - 2, 0)) / 2.0f;//G2 + (B5 - B1)/2
	r2g_e = input(rg.x + 1, rg.y, 1) + (input(rg.x, rg.y, 0) - input(rg.x + 2, rg.y, 0)) / 2.0f;//G6 + (B5 - B7)/2
	r2g_s = input(rg.x, rg.y + 1, 1) + (input(rg.x, rg.y, 0) - input(rg.x, rg.y + 2, 0)) / 2.0f;//G8 + (B5 - B9)/2
	r2g_w = input(rg.x - 1, rg.y, 1) + (input(rg.x, rg.y, 0) - input(rg.x - 2, rg.y, 0)) / 2.0f;//G4 + (B5 - B3)/2

	//fill the green channel
	result = (r2g_n * omega_n + r2g_w * omega_w + r2g_s * omega_s + r2g_e * omega_e)/(omega_w + omega_s + omega_e + omega_n);
	demosaicG(rg.x, rg.y, 1) = cast<int16_t>(result);

	Buffer<int16_t> buffer = demosaicG.realize(input0.width(), input0.height(), input0.channels());


	//Up until now, all tiles have Green value

	//Fill Blue to Red tile

	Func demosaicRB("RB");

	demosaicRB(x, y, c) = buffer(x, y, c);

	RDom rb(1, input.width() - 2, 1, input.height() - 2);

	rb.where(rb.x % 2 == 1);
	rb.where(rb.y % 2 == 0);

	Expr r2b_ne, r2b_se, r2b_sw, r2b_nw;
	r2b_ne = abs(input(rb.x - 1, rb.y + 1, 2) - input(rb.x + 1, rb.y - 1, 2)) + abs(buffer(rb.x, rb.y, 1) - buffer(rb.x + 1, rb.y - 1, 1));//|B7 - B3| + |G5 - G3|
	r2b_se = abs(input(rb.x - 1, rb.y - 1, 2) - input(rb.x + 1, rb.y + 1, 2)) + abs(buffer(rb.x, rb.y, 1) - buffer(rb.x + 1, rb.y + 1, 1));//|B1 - B9| + |G5 - G9|
	r2b_sw = abs(input(rb.x - 1, rb.y + 1, 2) - input(rb.x + 1, rb.y - 1, 2)) + abs(buffer(rb.x, rb.y, 1) - buffer(rb.x - 1, rb.y + 1, 1));//|B3 - B7| + |G5 - G7|
	r2b_nw = abs(input(rb.x - 1, rb.y - 1, 2) - input(rb.x + 1, rb.y + 1, 2)) + abs(buffer(rb.x, rb.y, 1) - buffer(rb.x - 1, rb.y - 1, 1));//|B9 - B1| + |G5 - G1|

	omega_n = 1/(r2b_ne + 1.0f);
	omega_e = 1/(r2b_se + 1.0f);
	omega_s = 1/(r2b_sw + 1.0f);
	omega_w = 1/(r2b_nw + 1.0f);

	r2b_ne = input(rb.x + 1, rb.y - 1, 2) + (buffer(rb.x, rb.y, 1) - buffer(rb.x + 1, rb.y - 1, 1)) / 2.0f;//B3 + (G5 - G3)/2
	r2b_se = input(rb.x + 1, rb.y + 1, 2) + (buffer(rb.x, rb.y, 1) - buffer(rb.x + 1, rb.y + 1, 1)) / 2.0f;//B9 + (G5 - G9)/2
	r2b_sw = input(rb.x - 1, rb.y + 1, 2) + (buffer(rb.x, rb.y, 1) - buffer(rb.x - 1, rb.y + 1, 1)) / 2.0f;//B7 + (G5 - G7)/2
	r2b_nw = input(rb.x - 1, rb.y - 1, 2) + (buffer(rb.x, rb.y, 1) - buffer(rb.x - 1, rb.y - 1, 1)) / 2.0f;//B1 + (G5 - G1)/2

	//put the value into blue (x, y, 2)
	result = (r2b_ne * omega_n + r2b_nw * omega_w + r2b_sw * omega_s + r2b_se * omega_e)/(omega_w + omega_s + omega_e + omega_n);
	demosaicRB(rb.x, rb.y, 2) = cast<int16_t>(result);

	//Fill Red to Blue tile

	RDom br(1, input.width() - 2, 1, input.height() - 2);

	br.where(br.x % 2 == 0);
	br.where(br.y % 2 == 1);

	Expr b2r_ne, b2r_se, b2r_sw, b2r_nw;
	b2r_ne = abs(input(br.x - 1, br.y + 1, 0) - input(br.x + 1, br.y - 1, 0)) + abs(buffer(br.x, br.y, 1) - buffer(br.x + 1, br.y - 1, 1));//|R7 - R3| + |G5 - G3|
	b2r_se = abs(input(br.x - 1, br.y - 1, 0) - input(br.x + 1, br.y + 1, 0)) + abs(buffer(br.x, br.y, 1) - buffer(br.x + 1, br.y + 1, 1));//|R1 - R9| + |G5 - G9|
	b2r_sw = abs(input(br.x - 1, br.y + 1, 0) - input(br.x + 1, br.y - 1, 0)) + abs(buffer(br.x, br.y, 1) - buffer(br.x - 1, br.y + 1, 1));//|R3 - R7| + |G5 - G7|
	b2r_nw = abs(input(br.x - 1, br.y - 1, 0) - input(br.x + 1, br.y + 1, 0)) + abs(buffer(br.x, br.y, 1) - buffer(br.x - 1, br.y - 1, 1));//|R9 - R1| + |G5 - G1|

	omega_n = 1/(b2r_ne + 1.0f);
	omega_e = 1/(b2r_se + 1.0f);
	omega_s = 1/(b2r_sw + 1.0f);
	omega_w = 1/(b2r_nw + 1.0f);

	b2r_ne = input(br.x + 1, br.y - 1, 0) + (buffer(br.x, br.y, 1) - buffer(br.x + 1, br.y - 1, 1)) / 2.0f;//R3 + (G5 - G3)/2
	b2r_se = input(br.x + 1, br.y + 1, 0) + (buffer(br.x, br.y, 1) - buffer(br.x + 1, br.y + 1, 1)) / 2.0f;//R9 + (G5 - G9)/2
	b2r_sw = input(br.x - 1, br.y + 1, 0) + (buffer(br.x, br.y, 1) - buffer(br.x - 1, br.y + 1, 1)) / 2.0f;//R7 + (G5 - G7)/2
	b2r_nw = input(br.x - 1, br.y - 1, 0) + (buffer(br.x, br.y, 1) - buffer(br.x - 1, br.y - 1, 1)) / 2.0f;//R1 + (G5 - G1)/2

	//put the value into red channel (x, y, 0)
	result = (b2r_ne * omega_n + b2r_nw * omega_w + b2r_sw * omega_s + b2r_se * omega_e)/(omega_w + omega_s + omega_e + omega_n);
	demosaicRB(br.x, br.y, 0) = cast<int16_t>(result);

	buffer = demosaicRB.realize(input0.width(), input0.height(), input0.channels());
	

	//Fill Blue / Red to Green tile

	Func demosaicRGB("RGB");

	demosaicRGB(x, y, c) = cast<uint8_t>(buffer(x, y, c));

	RDom g(2, input.width() - 4, 2, input.height() - 4);

	g.where((g.x % 2 == 0 && g.y % 2 == 0) || (g.x % 2 == 1 && g.y % 2 == 1));

	Expr g_n, g_s, g_w, g_e;

	//Blue
	g_n = abs(input(g.x, g.y + 1, 1) - input(g.x, g.y - 1, 1)) + abs(buffer(g.x, g.y, 2) - buffer(g.x, g.y - 2, 2));//B_n = |B8 - B2| + |G5 - G1|
	g_e = abs(input(g.x - 1, g.y, 1) - input(g.x + 1, g.y, 1)) + abs(buffer(g.x, g.y, 2) - buffer(g.x + 2, g.y, 2));//B_n = |B4 - B6| + |G5 - G7|
	g_s = abs(input(g.x, g.y + 1, 1) - input(g.x, g.y - 1, 1)) + abs(buffer(g.x, g.y, 2) - buffer(g.x, g.y + 2, 2));//B_n = |B2 - B8| + |G5 - G9|
	g_w = abs(input(g.x - 1, g.y, 1) - input(g.x + 1, g.y, 1)) + abs(buffer(g.x, g.y, 2) - buffer(g.x - 2, g.y, 2));//B_n = |B6 - B4| + |G5 - G3|

	omega_n = 1/(g_n + 1.0f);
	omega_e = 1/(g_e + 1.0f);
	omega_s = 1/(g_s + 1.0f);
	omega_w = 1/(g_w + 1.0f);

	g_n = buffer(g.x, g.y - 1, 2) + (input(g.x, g.y, 1) - input(g.x, g.y - 2, 1)) / 2.0f;//B2 + (G5 - G1)/2
	g_e = buffer(g.x + 1, g.y, 2) + (input(g.x, g.y, 1) - input(g.x + 2, g.y, 1)) / 2.0f;//B6 + (G5 - G7)/2
	g_s = buffer(g.x, g.y + 1, 2) + (input(g.x, g.y, 1) - input(g.x, g.y + 2, 1)) / 2.0f;//B8 + (G5 - G9)/2
	g_w = buffer(g.x - 1, g.y, 2) + (input(g.x, g.y, 1) - input(g.x - 2, g.y, 1)) / 2.0f;//B4 + (G5 - G3)/2

	//put the value into green (x, y, 1)
	result = (g_n * omega_n + g_w * omega_w + g_s * omega_s + g_e * omega_e)/(omega_w + omega_s + omega_e + omega_n);
	demosaicRGB(g.x, g.y, 2) = cast<uint8_t>(result);


	//Red
	g_n = abs(input(g.x, g.y + 1, 1) - input(g.x, g.y - 1, 1)) + abs(buffer(g.x, g.y, 2) - buffer(g.x, g.y - 2, 2));//B_n = |R8 - R2| + |G5 - G1|
	g_e = abs(input(g.x - 1, g.y, 1) - input(g.x + 1, g.y, 1)) + abs(buffer(g.x, g.y, 2) - buffer(g.x + 2, g.y, 2));//B_n = |R4 - R6| + |G5 - G7|
	g_s = abs(input(g.x, g.y + 1, 1) - input(g.x, g.y - 1, 1)) + abs(buffer(g.x, g.y, 2) - buffer(g.x, g.y + 2, 2));//B_n = |R2 - R8| + |G5 - G9|
	g_w = abs(input(g.x - 1, g.y, 1) - input(g.x + 1, g.y, 1)) + abs(buffer(g.x, g.y, 2) - buffer(g.x - 2, g.y, 2));//B_n = |R6 - R4| + |G5 - G3|

	omega_n = 1/(g_n + 1.0f);
	omega_e = 1/(g_e + 1.0f);
	omega_s = 1/(g_s + 1.0f);
	omega_w = 1/(g_w + 1.0f);

	g_n = buffer(g.x, g.y - 1, 0) + (input(g.x, g.y, 1) - input(g.x, g.y - 2, 1)) / 2.0f;//R2 + (G5 - G1)/2
	g_e = buffer(g.x + 1, g.y, 0) + (input(g.x, g.y, 1) - input(g.x + 2, g.y, 1)) / 2.0f;//R6 + (G5 - G7)/2
	g_s = buffer(g.x, g.y + 1, 0) + (input(g.x, g.y, 1) - input(g.x, g.y + 2, 1)) / 2.0f;//R8 + (G5 - G9)/2
	g_w = buffer(g.x - 1, g.y, 0) + (input(g.x, g.y, 1) - input(g.x - 2, g.y, 1)) / 2.0f;//R4 + (G5 - G3)/2

	//put the value into green (x, y, 1)
	result = (g_n * omega_n + g_w * omega_w + g_s * omega_s + g_e * omega_e)/(omega_w + omega_s + omega_e + omega_n);
	demosaicRGB(g.x, g.y, 0) = cast<uint8_t>(result);
	
	Buffer<uint8_t> output =
        demosaicRGB.realize(input.width(), input.height(), input.channels());

    save_image(output, std::string(argv[1]) + "demosaic.png");


	return 0;
}
