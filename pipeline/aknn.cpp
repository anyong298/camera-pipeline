#include<iostream>
#include<time.h>
#include<vector>
#include "temporal.h"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <ctype.h>

            
vector<vector<short>*>* neighbors_h[height][width];
vector<vector<vector<short>*>*>* aknn_across_frames_point = new vector<vector<vector<short>*>*>; 
vector<vector<vector<short>*>*>* aknn_across_frames[width][height];

int main(int argc, char** argv) 
{
    short x = 360, y = 180;
    clock_t t = clock();
    get_input();
    short current_frame = 6; 
    for(short i = -H; i < H; i++) {
        load_halide_functions(current_frame + i);
        initiate_neighbors(current_frame + i, neighbors_h);
        interleave_propagate_and_random_search(current_frame + i, neighbors_h);
         
        Point2f point_current;
        vector<Point2f> point_offset;
        point_current = Point2f(x, y);
        cout<<"printing optical flow offsets for pixel ("<<x<<", "<<y<<") between frame " <<current_frame<<" and frame "<<current_frame + i<<endl; 
        point_offset = get_offset(point_current, current_frame, i);
        aknn_across_frames_point->push_back(neighbors_h[y][x]);
        //cout<<neighbors_h[y][x];
        cout<<"x "<<(short)point_offset[0].x<<endl;
        cout<<"y "<<(short)point_offset[0].y<<endl;
    }
    aknn_across_frames[y][x] = aknn_across_frames_point;
    t = clock() - t;
    cout<<(float)t/CLOCKS_PER_SEC<<" seconds"<<endl;
}

void interleave_propagate_and_random_search(short frame, vector<vector<short>*>* neighbors_h[height][width])
{
    for(int i = 0; i < 4; i++) {
        propagate_neighbors(frame, neighbors_h);
        random_search(frame, neighbors_h);
    }
}
