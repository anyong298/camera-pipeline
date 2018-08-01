#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "optical_flow.h"
#include <iostream>
#include <ctype.h>
#include <vector>

using namespace cv;
using namespace std;

//int main(int argc, char** argv)
//{
//    //input is a point and an integer which specifies the frame that we want. 
//    
//    Point2f point;
//    point = Point2f(atof(argv[1]), atof(argv[2]));
//    int current_frame = 50;
//    int next_frame_offset = 1;
//    vector<Point2f> z = get_offset(point, current_frame, next_frame_offset);
//    cout<<"z "<<z<<endl;
//}


vector<Point2f> get_offset(Point2f point, int current_frame, int next_frame_offset)
{
    TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS, 20, 0.03);
    Size winSize(31, 31);
    Mat gray1, gray2, frame1, frame2;
    vector<uchar> status;
    vector<float> err;
    frame1 = imread("./frames/" + to_string(current_frame) + ".png", 1);
    frame2 = imread("./frames/" + to_string(current_frame + next_frame_offset) + ".png", 1);
    
    cvtColor(frame1, gray1, COLOR_BGR2GRAY);
    cvtColor(frame2, gray2, COLOR_BGR2GRAY);
    
    vector<Point2f> points[2];
    points[0].push_back(point);

    calcOpticalFlowPyrLK(gray1, gray2, points[0], points[1], 
            status, err, winSize, 3, termcrit, 0, 0.001);
    
    return points[1];
}



