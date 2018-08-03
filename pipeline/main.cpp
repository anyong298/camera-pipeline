#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
#include <float.h> 
#include <math.h>
#include <string>
#include "pipeline.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace Halide::ConciseCasts;


int main(int argc, char** argv)
{
    Buffer<uint8_t> stage1 = load_image("images/inputs/" + std::string(argv[1]));
    Buffer<uint8_t> stage2 = demosaic_naive(stage1);
    Buffer<uint8_t> stage3 = denoise_no_approx(stage2);    
    //Buffer<uint8_t> stage4 = white_balance(stage3, (char*)"1", (char*)"2", (char*)"4");
    //Buffer<uint8_t> final_stage = gamma_correction(stage3);     
    save_image(stage3, "images/outputs/" + std::string(argv[1]) + "_obr.png");
}
