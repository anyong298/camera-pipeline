#include<iostream>
#include<time.h>
#include<vector>
#include "temporal.h"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <ctype.h>
#include <math.h>
#include "Halide.h"
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::ConciseCasts;

vector<vector<short>*>* neighbors_h[height][width];
vector<vector<vector<short>*>*>* aknn_across_frames_point = new vector<vector<vector<short>*>*>; 
vector<vector<vector<short>*>*>* aknn_across_frames[width][height];

int main(int argc, char** argv) 
{
    
    clock_t t = clock();
    get_input();
    short current_frame = 6; 
    short x = 360, y = 180;
    for(short i = -H; i < H; i++) {
        load_halide_functions(current_frame + i);
        initiate_neighbors(current_frame + i, neighbors_h);
        interleave_propagate_and_random_search(current_frame + i, neighbors_h);
        Point2f point_current;
        vector<Point2f> point_offset;
        
        point_current = Point2f(x, y);
        cout<<"printing optical flow offsets for pixel ("<<x<<", "<<y<<") between frame " 
            <<current_frame<<" and frame "<<current_frame + i<<endl; 
        point_offset = get_offset(point_current, current_frame, i);
        aknn_across_frames_point->push_back(neighbors_h[y][x]);
        //cout<<neighbors_h[y][x];
        cout<<"x "<<(short)point_offset[0].x<<endl;
        cout<<"y "<<(short)point_offset[0].y<<endl;
    }
    aknn_across_frames[y][x] = aknn_across_frames_point;
    t = clock() - t;
    cout<<"testing non_local_means_estimate"<<endl;
    non_local_means_estimate(x, y, current_frame);
    cout<<(float)t/CLOCKS_PER_SEC<<" seconds"<<endl;
}

void interleave_propagate_and_random_search(short frame, vector<vector<short>*>* neighbors_h[height][width])
{
    for(int i = 0; i < 4; i++) {
        propagate_neighbors(frame, neighbors_h);
        random_search(frame, neighbors_h);
    }
}

float non_local_means_estimate(short x, short y, short frame)
{
    
    float nlm_estimate_unweighted = 0;
    float gamma = 0.9;
    for(short i = frame - H; i <= frame + H; i++) {
        float weight = 0;
        for(ushort j = 1; j <= K; j++) {
            weight += calc_input(x, y, i, j) * 
                exp(-calc_weighted_ssd(frame, i, j) 
                        / (2 * pow(noise_level_factor(), 2))); 
        }
        nlm_estimate_unweighted += pow(gamma, abs(i - frame)) * weight;
    }
    return 1 / normalization_factor(frame) * nlm_estimate_unweighted;
}

float normalization_factor(short frame)
{
    float normalization_factor = 0;
    float gamma = 0.9;
    for(int i = frame - H; i <= frame + H; i++) {
        float weight = 0;
        for(ushort j = 1; j <= K; j++) {
           weight += exp(-calc_weighted_ssd(frame, i, j) / 
                   (2 * pow(noise_level_factor(), 2)));
        }
        normalization_factor += pow(gamma, abs(i - frame)) * weight;
    } 
    return normalization_factor;
}




float noise_level_factor()
{
    return 6;
}

float calc_input(short frame, short frame_offset, short i, short j)
{
    return 10;
}


