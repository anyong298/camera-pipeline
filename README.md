# camera-pipeline

University of Rochester

Authors: Prikshet Sharma, Oliver Ziqi Zhang

ISP pipeline built using Halide and is under development. The pipeline aims to help optimize optical systems. 

## Getting Started
edit ```pipeline.cpp``` to manage various stages in the pipeline. 
```make pipeline``` to compile the pipeline. 

### Prerequisites

OpenCV latest release: https://opencv.org/releases.html 

### Installing

Clone Halide: https://github.com/halide/Halide/releases 

After cloing the Halide repo, copy the contents of this repo in the Halide folder. 

Run ```make pipeline``` in the folder pipeline.
End with an example of getting some data out of the system or using it for a little demo.

### Pipeline Stages

#Optically Black Region
Camera sensors has a rectangluar region coated with an optically black compound on one edge of the camera sensor. This coated region is known as the optically black region and is used to correct the brightness of the image. Our optically black region function in Halide simply takes the average of the intensities of the pixels in the optically black region and subtracts the that number from each channel of every pixel in the image.   

#Demosaic (Naive and Advanced)
Assuming the camera sensor has a Bayer filter, the pipeline stage is mandatory for any computation photography and computer vision applications. To compute a color channel value for the current pixel, the naive version of demosaic simply takes the average of the surrounding pixels of the given color (RGB).
Advanced Demosaic uses a fancier approach taken from this research paper. 

#Spatial denoising
Please see this link where the algorithm is taken from.

#White Balance 
White balance corrects the 'temperature' of the image by taking the lightest pixel in the image, changing it to white (0, 0, 0) and then normalizing every other pixel based on the lightest pixel.  

#Gamma Correction
Gamma correction requires that the image be converted to YCbCr format, which hasn't yet been implemented. 

#Temporal Denoising 
Temporal Denoising performs the algorithm taken from this MIT and Microsoft research paper and uses the optical flow algorithm from the openCV library. We're still working on an optical flow algorithm implemented in Halide. 

## Running the tests

Some stages in the pipeline require additional arguments, so make sure you put those arguments. 

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

See also the list of [contributors](https://github.com/horizon-research) who participated in University of Rochester's computer vision and computational photoraphy research project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Special thanks to Yuhao Zhu for making this possible. 

