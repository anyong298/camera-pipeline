/*
 * University of Rochester
 * Authors: Prikshet Sharma, Oliver Ziqi Zhang
 * 
 * */

#include<stdio.h>
#include "Halide.h"
#define OBR_HEIGHT 10
using namespace Halide;
//using namespace Halide::Tools;
using namespace ConciseCasts;


Buffer<uint8_t> obr(Buffer<uint8_t> input) 
{
    //Buffer<uint8_t> input = Tools::load_image(argv[1]);
    Func obrOffset;

    Var x, y, c;

    Expr value = f32(input(x, y, c));

    // input_16 is the OBR region cast to 16 bit for calculations so that it doesn't overflow 
    Func input_16("input_16");

    input_16(x, y, c) = u16(input(x, y, c));    

    Func black;
    black(x, y, c) = 100;
    
    RDom r(0, input.width(), 0, 10);

    Expr average = sum(black(r.x, r.y, c))/(OBR_HEIGHT * u16(input.width()));
    
    obrOffset(x, y, c) = u8(max((input_16(x, y, c)) - average, 0));
    Buffer<uint8_t> output(input.height(), input.width(), input.channels());

    obrOffset.realize(output);

    return output;
}

Buffer<uint8_t> demosaic(Buffer<uint8_t> input0) 
{

	Var x("x"), y("y"), c("c");

	Func castutoi("casting");
	castutoi(x, y, c) = cast<int16_t>(input0(x, y, c));
	Buffer<int16_t> input = castutoi.realize(input0.width(), input0.height(), input0.channels());

	// demosaic.trace_stores();

	//Fill Green to Blue tile

	Func demosaicG("Green");

	demosaicG(x, y, c) = input(x, y, c);

	RDom bg(2, input.width() - 4, 2, input.height() - 4);

	bg.where(bg.x % 2 == 1);
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
	demosaicG(bg.x, bg.y, 1) = i16(result);

	//Fill Green to Red tile

	RDom rg(2, input.width() - 4, 2, input.height() - 4);

	rg.where(rg.x % 2 == 0);
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
	demosaicG(rg.x, rg.y, 1) = i16(result);

	Buffer<int16_t> buffer = demosaicG.realize(input0.width(), input0.height(), input0.channels());


	//Up until now, all tiles have Green value

	//Fill Blue to Red tile

	Func demosaicRB("RB");

	demosaicRB(x, y, c) = buffer(x, y, c);

	RDom rb(1, input.width() - 2, 1, input.height() - 2);

	rb.where(rb.x % 2 == 0);
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
	demosaicRB(rb.x, rb.y, 2) = i16(result);

	//Fill Red to Blue tile

	RDom br(1, input.width() - 2, 1, input.height() - 2);

	br.where(br.x % 2 == 1);
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
	demosaicRB(br.x, br.y, 0) = i16(result);

	buffer = demosaicRB.realize(input0.width(), input0.height(), input0.channels());
	

	//Fill Blue / Red to Green tile

	Func demosaicRGB("RGB");

	demosaicRGB(x, y, c) = u8(buffer(x, y, c));

	RDom g(2, input.width() - 4, 2, input.height() - 4);

	g.where((g.x % 2 == 1 && g.y % 2 == 0) || (g.x % 2 == 0 && g.y % 2 == 1));

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
	demosaicRGB(g.x, g.y, 2) = u8(result);


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
	demosaicRGB(g.x, g.y, 0) = u8(result);
	
	Buffer<uint8_t> output =
        demosaicRGB.realize(input.width(), input.height(), input.channels());

	return output;

}

Buffer<uint8_t> denoise_no_approx(Buffer<uint8_t> input) 
{
	//Buffer<uint8_t> input = Tools::load_image(argv[1]);

	printf("channels=%d\n", input.channels());
	float sig_s_f = 2.0;
	float sig_r_f = 2.0;
	int L = sig_r_f * 3;
	int W = sig_s_f * 3;

	Var x, y, c, t;

	// declare range and spatial filters
	Func g_sig_s, g_sig_r;
	RDom omega(-W, W, -W, W), l_r(-L, L, -L, L);;


	g_sig_s(x, y) = f32(0);
	

	g_sig_s(omega.x, omega.y) = f32(exp(-(omega.x * omega.x + omega.y * omega.y) / (2 * sig_s_f * sig_s_f)));

	g_sig_r(t) = f32(exp(- t * t / (2 * sig_r_f * sig_r_f)));


	Func imp_bi_filter, imp_bi_filter_num, imp_bi_filter_den, imp_bi_filter_num_clamped, imp_bi_filter_den_clamped;


	Func box_filtered;

	box_filtered(x, y, c) = u8((float)(1) / ((2 * L + 1) * (2 * L + 1)) * sum(input(x - l_r.x, y - l_r.y, c)));
	
	// Compute box filtered image
	Expr clamped_x = clamp(x, L, input.width() - 2 * L - 1);
	Expr clamped_y = clamp(y, L, input.height() - 2 * L - 1);

	imp_bi_filter_num(x, y, c) = f32(sum(g_sig_s(omega.x, omega.y) * 
                g_sig_r(box_filtered(x - omega.x, y - omega.y, c) - box_filtered(x, y, c)) * 
                input(x - omega.x, y - omega.y, c)));
	
    imp_bi_filter_den(x, y, c) = f32(sum((g_sig_s(omega.x, omega.y)) 
                * g_sig_r((box_filtered(x - omega.x, y - omega.y, c) - box_filtered(x, y, c)))));
	
    imp_bi_filter_num_clamped(x, y, c) = imp_bi_filter_num(clamped_x, clamped_y, c);
	
    imp_bi_filter_den_clamped(x, y, c) = imp_bi_filter_den(clamped_x, clamped_y, c);
	
	imp_bi_filter(x, y, c) = u8(imp_bi_filter_num_clamped(x, y, c) / imp_bi_filter_den_clamped(x, y, c));
	
   Buffer<uint8_t> shifted(input.width() - 2 * W, input.height() - 2 * W, input.channels());
	shifted.set_min(W, W);
	//imp_bi_filter.trace_stores();
	imp_bi_filter.realize(shifted);

	//Tools::save_image(shifted, "denoise.png");
	
    return shifted;
}



float slope[3];
Buffer<uint8_t> white_balance(Buffer<uint8_t> input)
{
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

	Func white_balance("white_balance");

	Var x("x"), y("y"), c("c");

	white_balance(x, y, c) = input(x, y, c);
	white_balance(x, y, 0) = cast<uint8_t>(min(input(x, y, 0) * slope[0], 255.0f));
	white_balance(x, y, 1) = cast<uint8_t>(min(input(x, y, 1) * slope[1], 255.0f));
	white_balance(x, y, 2) = cast<uint8_t>(min(input(x, y, 2) * slope[2], 255.0f));

	Buffer<uint8_t> output =
        white_balance.realize(input.width(), input.height(), input.channels());

	return output;


}


Buffer<uint8_t> gamma_correction(Buffer<uint8_t> input)
{

	Func correct("correct");
	Var x("x"), y("y"), c;

	Expr value = cast<float>(input(x, y, c));

	// value = 255 * (pow((value / 255.0f), (1/2.2f)));
	value = (pow((value), (1/2.2f)));

	correct(x, y, c) = (cast<uint8_t>(value));

	Buffer<uint8_t> output = correct.realize(input.width(), input.height(), input.channels());

	return output;

}

Buffer<uint8_t> demosaic_naive(Buffer<uint8_t> input)
{
	Func demosaic("demosaic");

	Var x("x"), y("y"), c("c");
	
	// Expr value;

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

	//Green

	RDom g1(1, input.width() - 2, 1, input.height() - 2);
	g1.where(g1.x % 2 == 0);
	g1.where(g1.y % 2 == 1);

	demosaic(g1.x, g1.y, 0) = (input(g1.x, g1.y + 1, 0) / 2 + input(g1.x, g1.y - 1, 0) / 2);
	demosaic(g1.x, g1.y, 2) = (input(g1.x + 1, g1.y, 2) / 2 + input(g1.x - 1, g1.y, 2) / 2);

	RDom g2(1, input.width() - 2, 1, input.height() - 2);
	g2.where(g2.x % 2 == 1);
	g2.where(g2.y % 2 == 0);

	demosaic(g2.x, g2.y, 0) = (input(g2.x + 1, g2.y, 0) / 2 + input(g2.x - 1, g2.y, 0) / 2);
	demosaic(g2.x, g2.y, 2) = (input(g2.x, g2.y + 1, 2) / 2 + input(g2.x, g2.y - 1, 2) / 2);

	Buffer<uint8_t> output =
        demosaic.realize(input.width(), input.height(), input.channels());


	return output;
}
