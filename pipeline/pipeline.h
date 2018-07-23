#include "Halide.h"
#include "halide_image_io.h"
using namespace Halide;
Buffer<uint8_t> obr(Buffer<uint8_t> input);
Buffer<uint8_t> demosaic(Buffer<uint8_t> input0);
Buffer<uint8_t> denoise_no_approx(Buffer<uint8_t> input);
Buffer<uint8_t> white_balance(Buffer<uint8_t> input, char* arg1, char* arg2, char* arg3);
Buffer<uint8_t> gamma_correction(Buffer<uint8_t> input);
