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

