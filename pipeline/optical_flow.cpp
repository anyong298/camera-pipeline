//#include "opencv2/video/tracking.hpp"
//#include "opencv2/imgproc.hpp"
//#include "opencv2/videoio.hpp"
//#include "opencv2/highgui.hpp"
//
//#include <iostream>
//#include <ctype.h>
//
//using namespace cv;
//using namespace std;
//
//static void help()
//{
//    // print a welcome message, and the OpenCV version
//    cout << "\nThis is a demo of Lukas-Kanade optical flow lkdemo(),\n"
//            "Using OpenCV version " << CV_VERSION << endl;
//    cout << "\nIt uses camera by default, but you can provide a path to video as an argument.\n";
//    cout << "\nHot keys: \n"
//            "\tESC - quit the program\n"
//            "\tr - auto-initialize tracking\n"
//            "\tc - delete all the points\n"
//            "\tn - switch the \"night\" mode on/off\n"
//            "To add/remove a feature point click it\n" << endl;
//}
//
//Point2f point;
//bool addRemovePt = false;
//
//static void onMouse( int event, int x, int y, int /*flags*/, void* /*param*/ )
//{
//    if( event == EVENT_LBUTTONDOWN )
//    {
//        point = Point2f((float)x, (float)y);
//        addRemovePt = true;
//    }
//}
//
//int main( int argc, char** argv )
//{
//    VideoCapture cap;
//    TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
//    Size subPixWinSize(10,10), winSize(31,31);
//
//    const int MAX_COUNT = 500;
//    bool needToInit = false;
//    bool nightMode = false;
//
//    help();
//    cv::CommandLineParser parser(argc, argv, "{@input|0|}");
//    string input = parser.get<string>("@input");
//
//    if( input.size() == 1 && isdigit(input[0]) )
//        cap.open(input[0] - '0');
//    else
//        cap.open(input);
//
//    if( !cap.isOpened() )
//    {
//        cout << "Could not initialize capturing...\n";
//        return 0;
//    }
//
//    namedWindow( "LK Demo", 1 );
//    setMouseCallback( "LK Demo", onMouse, 0 );
//
//    Mat gray, prevGray, image, frame;
//    vector<Point2f> points[2];
//
//    for(;;)
//    {
//        cap >> frame;
//        if( frame.empty() )
//            break;
//
//        frame.copyTo(image);
//        cvtColor(image, gray, COLOR_BGR2GRAY);
//
//        if( nightMode )
//            image = Scalar::all(0);
//
//        if( needToInit )
//        {
//            // automatic initialization
//            goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 3, 0, 0.04);//changed 0.04->0.4
//            cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit);
//            addRemovePt = false;
//        }
//        else if( !points[0].empty() )
//        {
//            vector<uchar> status;
//            vector<float> err;
//            if(prevGray.empty())
//                gray.copyTo(prevGray);
//            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
//                                 3, termcrit, 0, 0.001);
//            size_t i, k;
//            for( i = k = 0; i < points[1].size(); i++ )
//            {
//                if( addRemovePt )
//                {
//                    if( norm(point - points[1][i]) <= 5 )
//                    {
//                        addRemovePt = false;
//                        continue;
//                    }
//                }
//
//                if( !status[i] )
//                    continue;
//
//                points[1][k++] = points[1][i];
//                circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
//            }
//            points[1].resize(k);
//        }
//
//        if( addRemovePt && points[1].size() < (size_t)MAX_COUNT )
//        {
//            vector<Point2f> tmp;
//            tmp.push_back(point);
//            cornerSubPix( gray, tmp, winSize, Size(-1,-1), termcrit);
//            points[1].push_back(tmp[0]);
//            addRemovePt = false;
//        }
//
//        needToInit = false;
//        imshow("LK Demo", image);
//
//        char c = (char)waitKey(10);
//        if( c == 27 )
//            break;
//        switch( c )
//        {
//        case 'r':
//            needToInit = true;
//            break;
//        case 'c':
//            points[0].clear();
//            points[1].clear();
//            break;
//        case 'n':
//            nightMode = !nightMode;
//            break;
//        }
//
//        std::swap(points[1], points[0]);
//        cv::swap(prevGray, gray);
//    }
//
//    return 0;
//}

 //Optical Flow estimation Code
#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
using namespace Halide::Tools;

Halide::Func conv(Halide::Func Image, Halide::Func kernel, Halide::Expr k_height, Halide::Expr k_width);

int main(int argc, char **argv) {
	printf("Optical Flow started\n");
	//Algorithm
	//load the images
    Halide::Buffer<uint8_t> img1 = load_image("images/co_1.png");
    Halide::Buffer<uint8_t> img2 = load_image("images/gray.png");

	Halide::Var x("x"),y("y"),c("c");
	Halide::Func gradient("gradient");



	/*.....................Image1 gradient....................................*/
	Halide::Func limit1/*(Halide::_)*/ = Halide::BoundaryConditions::constant_exterior(img1,255);//(Halide::_)/1;
	//Halide::Expr x_gra = Halide::cast<uint8_t>(img1(x,y+1)-img1(x, y));
	Halide::Func fun_x[2],fun_y[2];
	fun_x[0](x,y) = Halide::cast<uint8_t>(limit1(x,y+1)-limit1(x, y));//x_gra;
	//Halide::Expr y_gra = Halide::cast<uint8_t>(img1(x+1,y)-img1(x, y));
	fun_y[0](x,y) = Halide::cast<uint8_t>(limit1(x+1,y)-limit1(x, y));//y_gra;

	//limit/*(Halide::_)*/ = Halide::BoundaryConditions::constant_exterior(img2,255);//(Halide::_)/1;
	//Schedule
	Halide::Buffer<uint8_t> x_img1 = fun_x[0].realize(img1.width(),img1.height());
	Halide::Buffer<uint8_t> y_img1 = fun_y[0].realize(img1.width(),img1.height());
	save_image(x_img1, "images/x_img1.png");
	save_image(x_img1, "images/y_img1.png");	


	/*.....................Image2 gradient....................................*/

	Halide::Func limit2/*(Halide::_)*/ = Halide::BoundaryConditions::constant_exterior(img2,255);//(Halide::_)/1;

	fun_x[1](x,y) = Halide::cast<uint8_t>(limit2(x,y+1)-limit2(x, y));//x_gra;

	//Halide::Expr y_gra = Halide::cast<uint8_t>(img1(x+1,y)-img1(x, y));
	fun_y[1](x,y) = Halide::cast<uint8_t>(limit2(x+1,y)-limit2(x, y));//y_gra;
	x_img1 = fun_x[1].realize(img1.width(),img1.height());
	y_img1 = fun_y[1].realize(img1.width(),img1.height());

	save_image(x_img1, "images/x_img2.png");
	save_image(x_img1, "images/y_img2.png");


	/*.............................Image Diff.....................*/

	Halide::Func fun_It; 
	fun_It(x,y)= Halide::cast<uint8_t>(limit1(x,y)-limit2(x,y));
/*...............................................................*/
	Halide::Func Ix_sq,Iy_sq,IxIy,Ix_It,Iy_It;
	Halide::Func kernel;
	Halide::Expr k_height,k_width;
	k_height=3;
	k_width=3; 
	kernel(x,y)=1;

	Ix_sq(x,y) = Halide::pow(fun_x[0](x,y),2);
	Iy_sq(x,y) = Halide::pow(fun_y[0](x,y),2);
	IxIy(x,y) = (fun_x[0](x,y)*fun_y[0](x,y));
	Ix_It(x,y)= (fun_x[0](x,y)*fun_It(x,y));
	Iy_It(x,y)= (fun_y[0](x,y)*fun_It(x,y));

 //Compute the matrix of smallest singular values lambda2 for each 
// pixel location.  Here, C=[sum_Ix_sqr,sum_Ix_Iy;sum_Ix_Iy,sum_Iy_sqr].
// Since C is just a 2X2 matrix, we can compute this singular value
// analytically in closed form, for all pixels simultaneously
//Param<float> A{"A"};
	Halide::Expr  tau = Halide::cast<float>(0.2);//threshhold for 2nd eigenvalue
	Halide::Func trace,deter,lambda1,lambda2,hitmap;
	trace(x,y) = Ix_sq(x,y) + Iy_sq(x,y);
	deter(x,y) = Ix_sq(x,y)*Iy_sq(x,y) - Halide::pow(IxIy(x,y),2);
	lambda2(x,y) =(trace(x,y)-Halide::sqrt(Halide::pow(trace(x,y),2)-4*deter(x,y)));
	hitmap = lambda2>tau;
	printf("Success!\n");
    return 0;
}

//convolution function
Halide::Func conv(Halide::Func img, Halide::Func kernel, Halide::Expr k_height, Halide::Expr k_width){

    Halide::Var x,y;
    Halide::Func result;
    Halide::RDom kx(0,k_width), ky(0,k_height);
    result(x,y) += kernel(kx,ky)*img(x+kx-(k_width/2),y+ky-(k_height/2));
    return result;

}

