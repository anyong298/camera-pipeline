# Camera Pipeline

Authors: Prikshet Sharma, Oliver Ziqi Zhang

University of Rochester

The under development ISP pipeline is written in Halide that aims optimize optical systems and camera pipeline for computer vision and computational photography applications. Feel free to contact the co-author of the pipeline and the author of this README at pshar10@u.rochester.edu 

## Getting Started
Mix and match the various pipeline stages by editing ```pipeline.cpp```  

Command to compile the pipeline: ```make pipeline```

## Prerequisites

Currently, we use the OpenCV library for computing Optical Flow for the temporal denoising stage. We are implementing an Optical Flow algorithm in Halide. Download the OpenCV latest release here: https://opencv.org/releases.html 

## Installing the Pipeline Framework

Clone Halide: https://github.com/halide/Halide/releases 

After cloning the Halide repo, copy the contents of this repo in the Halide folder. 

In the subfolder pipeline, run ```make pipeline```.


## Implemented Pipeline Stages
All stages are implemented in ```pipeline.cpp```, except 'Temporal Denoising using Optical Flow', which is implemented in ```temporal.cpp```.

### Optically Black Region (OBR)
For those who don't know, camera sensors have one edge coated with an optically black region, which is used to correct image brightness. 
In ```pipeline.cpp``` the function ```Buffer<uint8_t> obr(Buffer<uint8_t> input)``` averages of the intensities of the optically black region pixels, and subtracts the average from each channel of every pixel in the image.   

### Demosaic (Naive and Advanced)
Research shows that this stage is mandatory for any computational photography and computer vision application. This implementation is for Bayer Filters.  

We implemeted two versions, naive and advanced. 

The naive version ```Buffer<uint8_t> demosaic_naive(Buffer<uint8_t> input)``` naively averages the adjacent pixels to compute current pixel(for each color channel).

Advanced Demosaic ```Buffer<uint8_t> demosaic(Buffer<uint8_t> input0)``` is implemented using this research paper. 

### Spatial denoising
Spatial denoising comprises of a spatial and a range filter. 
Explanation of the implementation: Suppose the noisy image is composed of small n x n pixel tiles.


*Concise Spatial Filter Algorithm*
1. Multiply each pixel in a given tile by a weight--the weight is a function (standard gaussian) of the pixel's displacement from the central pixel of the tile.
2. Take mean of weighted pixels. 
3. Use the mean to denoise the tile's central pixel. (See Spatial Denoising Algorithm)

*Concise Range Filter Algorithm*
1. Multiply each pixel in a given tile by a weight. The weight is a function (standard gaussian) of how similar the pixel is to the central pixel of the tile.
2. Take mean of the weighted pixels in the tile.
3. Use the mean to denoise the tile's central pixel. (See Spatial Denoising Algorithm)

* Concise Spatial Denoising Algorithm*
1. Multiply _spatial filter mean_ * _range filter mean_ *_central pixel_. 
2. Normalize the product. Result is the denoised version of the central pixel of the tile. 
3. Repeat for each tile in the image (for each color channel).

Research paper reference: M. Elad, “On the origin of the bilateral filter and ways to improve it,” IEEE Transactions in Image Processing, vol. 11, no. 10, pp. 1141-1151, 2002.

### White Balance 
White balance corrects the color temperature. It takes the lightest pixel in the image, replaces it with white (R=0, G=0, B=0), and then normalizes every other pixel based on lightest pixel's previous value.  

### Gamma Correction
Gamma correction requires that the image be converted to YCbCr format, which hasn't yet been implemented. 

### Temporal Denoising using Optical Flow 

Temporal Denoising performs the algorithm taken from this MIT and Microsoft research paper people.csail.mit.edu/celiu/pdfs/videoDenoising.pdf. The Optical Flow is computed using OpenCV. We're currently implementing Optical Flow in Halide.  

### Break down into end to end tests

An example of how to stage the pipeline is given in main.cpp. For example:

```
Buffer<uint8_t> stage1 = load_image("images/inputs/" + std::string(argv[1]));
Buffer<uint8_t> stage2 = denoise_no_approx(stage1);
save_image(stage2, "images/outputs/" + std::string(argv[1]) + "_obr.png");

```

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

* **Prikshet Sharma** - [zendevil](https://github.com/zendevil)
* **Oliver Ziqi Zhang** - [ziqizh](https://github.com/ziqizh)

See also the list of [contributors](https://github.com/horizon-research) who participated in University of Rochester's Visual Computing research project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Special thanks to Yuhao Zhu for making this possible. 

