#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{

    TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS, 20, 0.03);
    Size winSize(31, 31);
    Mat gray1, gray2, frame1, frame2;
    vector<uchar> status;
    vector<float> err;
    frame1 = imread("./OpticalFlow/car1.jpg", 1);
    frame2 = imread("./OpticalFlow/car2.jpg", 1);
    
    cvtColor(frame1, gray1, COLOR_BGR2GRAY);
    cvtColor(frame2, gray2, COLOR_BGR2GRAY);
  
    Point2f point;
    point = Point2f(atof(argv[1]), atof(argv[2]));

    vector<Point2f> points[2];
    points[0].push_back(point);

    calcOpticalFlowPyrLK(gray1, gray2, points[0], points[1], 
            status, err, winSize, 3, termcrit, 0, 0.001);
    
    cout<<points[1].at(0) - points[0].at(0)<<endl;

    
}
