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


int main(int argc, char** argv) 
{
    
    clock_t t = clock();
    get_input();
    short current_frame = 6; 
    short x = 360, y = 180;
    vector<short>* point = new vector<short>;
    point->push_back(x);
    point->push_back(y);
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
    non_local_means_estimate(current_frame, point);
    cout<<(float)t/CLOCKS_PER_SEC<<" seconds"<<endl;
}

void interleave_propagate_and_random_search(short current_frame, vector<vector<short>*>* neighbors_h[height][width])
{
    for(int i = 0; i < 4; i++) {
        propagate_neighbors(current_frame, neighbors_h);
        random_search(current_frame, neighbors_h);
    }
}

float non_local_means_estimate(short current_frame, vector<short>* patch_coord_current_frame)
{
    
    float nlm_estimate_unweighted = 0;
    float gamma = 0.9;
    short x = patch_coord_current_frame->at(0);
    short y = patch_coord_current_frame->at(1);
    for(short other_frame = current_frame - H; other_frame <= current_frame + H; 
            other_frame++) {
        float weight = 0;
        for(ushort neighbor = 1; neighbor <= K; neighbor++) {
            vector<short>* patch_coord_offset_other_frame = 
                aknn_across_frames[y][x]->at(other_frame - 1)->at(neighbor - 1);
            vector<short>* patch_coord_other_frame = new vector<short>;
            patch_coord_other_frame->push_back(x + patch_coord_offset_other_frame->at(0));
            patch_coord_other_frame->push_back(x + patch_coord_offset_other_frame->at(1));
            weight += calc_input(current_frame, other_frame, 
                    patch_coord_current_frame, patch_coord_other_frame) * 
                exp(-calc_weighted_ssd(current_frame, other_frame, 
                            patch_coord_current_frame, patch_coord_other_frame) / 
                        (2 * pow(noise_level_factor(), 2))); 
        }
        nlm_estimate_unweighted += pow(gamma, abs(other_frame - current_frame)) * weight;
    }
    return 1 / normalization_factor(current_frame) * nlm_estimate_unweighted;
}

float normalization_factor(short current_frame)
{
    float normalization_factor = 0;
    float gamma = 0.9;
    for(int other_frame = current_frame - H; other_frame <= current_frame + H; other_frame++) {
        float weight = 0;
        for(ushort neighbor = 1; neighbor <= K; neighbor++) {
           weight += exp(-calc_weighted_ssd(current_frame, other_frame, neighbor) / 
                   (2 * pow(noise_level_factor(), 2)));
        }
        normalization_factor += pow(gamma, abs(i - current_frame)) * weight;
    } 
    return normalization_factor;
}




float noise_level_factor()
{
    return 6;
}

float calc_input(short current_frame, short other_frame, vector<short>* patch_coord_current_frame, vector<short>* patch_coord_other_frame)
{
    return 10;
}


