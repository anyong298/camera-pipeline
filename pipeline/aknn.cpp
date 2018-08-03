#include<iostream>
#include<time.h>
#include<vector>
#include "temporal.h"
#include "opencv2/highgui.hpp"
#include "optical_flow.h"
#include <iostream>
#include <ctype.h>
#include "optical_flow.h"

const short width = 50;
const short height = 50;
const short H = 5;            
vector<vector<vector<short>*>*>* aknn_across_frames[width][height];

int main(int argc, char** argv) 
{
    clock_t t = clock();
    get_input();
    short current_frame = 5; 
    for(short i = -H; i <= H; i++) {
        load_halide_functions(current_frame + i);
        initiate_neighbors(current_frame + i);
        interleave_propagate_and_random_search(current_frame + i); 
    for(short y = 0; y < height; y++)
        for(short x = 0; x < width; x++) {
            Point2f point;
            point = Point2f(x, y);
            //cout<<"printing neighbors "<<current_frame + i << " "<<x<<" "<<y<<endl;
            //print_neighbors(get_neighbors(x, y));
            cout<<"printing optical flow offsets"<<endl;
            cout<<get_offset(point, current_frame, i)<<endl;
        }
    }
    
    t = clock() - t;
    cout<<(float)t/CLOCKS_PER_SEC<<" seconds"<<endl;
}

void interleave_propagate_and_random_search(short frame)
{
    for(int i = 0; i < 4; i++) {
        propagate_neighbors(frame);
        random_search(frame);
    }
}
