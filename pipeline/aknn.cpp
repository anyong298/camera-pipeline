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


typedef vector<vector<vector<short>*>*>* aknn_t;
vector<vector<short>*>* neigh_heap[height][width];
vector<vector<vector<short>*>*>* aknn_across_frames_point = new vector<vector<vector<short>*>*>; 
vector<vector<vector<short>*>*>* ** aknn_across_frames = new vector<vector<vector<short>*>*>* *[height];



int main(int argc, char** argv) 
{

    clock_t t = clock();

    for(int i = 0; i < height; i++) {
    aknn_across_frames[i] = new vector<vector<vector<short>*>*>*[width];
    }
    
    get_frames(); //0.4687 sec


    short curr_frame = 6; 

    // bucket for frames with loaded halide functions
    short loaded_frames[n_frames];  

    for(ushort i = 0; i < n_frames; i++) {
        loaded_frames[i] = 0;
    }

    for(ushort y = 0; y < height; y++)
        for(ushort x = 0; x < width; x++) {
            vector<short>* point = new vector<short>;

            point->push_back(x);
            point->push_back(y);

            
            for(short i = -H; i < H; i++) {
                
                if (loaded_frames[curr_frame + i] == 0) {
                    loaded_frames[curr_frame + i] = 1;
                    load_halide_functions(curr_frame + i); //0.015625 seconds
                } 

                
                
                initiate_neighbors(curr_frame + i, neigh_heap); // creates heap for all (x, y) //2.4 seconds 
                interleave_propagate_and_random_search(curr_frame + i, neigh_heap);
                
                
                Point2f point_curr;
                vector<Point2f> point_offset;
                
                point_curr = Point2f(x, y);
                cout<<"printing optical flow offsets for pixel ("<<x<<", "<<y<<") between frame " 
                    <<curr_frame<<" and frame "<<curr_frame + i<<endl; 

                point_offset = get_offset(point_curr, curr_frame, i);
                aknn_across_frames_point->push_back(neigh_heap[y][x]);
                //cout<<neigh_heap[y][x];
                cout<<"x "<<(short)point_offset[0].x<<endl;
                cout<<"y "<<(short)point_offset[0].y<<endl;

            }

            aknn_across_frames[y][x] = aknn_across_frames_point;
        }

    t = clock() - t;
    //cout<<"testing non_local_means_estimate"<<endl;
    //non_local_means_estimate(curr_frame, point);
    cout<<"time tester "<<(float)t/CLOCKS_PER_SEC<<" seconds"<<endl;
}

void interleave_propagate_and_random_search(short curr_frame, vector<vector<short>*>* neigh_heap[height][width])
{
    for(int i = 0; i < 4; i++) {
        
        propagate_neighbors(curr_frame, neigh_heap);
        
        
        random_search(curr_frame, neigh_heap); // ~0 seconds
        
        
    }
}

float non_local_means_estimate(short curr_frame, vector<short>* patch_coord_curr_frame)
{
    
    float nlm_estimate_unweighted = 0;
    float gamma = 0.9;

    short x = patch_coord_curr_frame->at(0);
    short y = patch_coord_curr_frame->at(1);

    for(short other_frame = curr_frame - H; other_frame <= curr_frame + H; 
            other_frame++) {

        float weight = 0;

        for(ushort neighbor = 1; neighbor <= K; neighbor++) {

            vector<short>* patch_coord_offset_other_frame = 
                aknn_across_frames[y][x]->at(other_frame - 1)->at(neighbor - 1);
            
            vector<short>* patch_coord_other_frame = new vector<short>;
            
            patch_coord_other_frame->push_back(x + patch_coord_offset_other_frame->at(0));
            patch_coord_other_frame->push_back(x + patch_coord_offset_other_frame->at(1));
            
            weight += calc_input(curr_frame, other_frame, 
                    patch_coord_curr_frame, patch_coord_other_frame) * 
                exp(-calc_weighted_ssd(curr_frame, other_frame, 
                            patch_coord_curr_frame, patch_coord_other_frame, neighbor - 1) / 
                        (2 * pow(noise_level_factor(), 2))); 
        }
        nlm_estimate_unweighted += pow(gamma, abs(other_frame - curr_frame)) * weight;
    }
    return 1 / normalization_factor(curr_frame, patch_coord_curr_frame) * nlm_estimate_unweighted;
}

float normalization_factor(short curr_frame, vector<short>* curr_frame_coord)
{
   float normalization_factor = 0;
   float gamma = 0.9;

   for(int other_frame = curr_frame - H; other_frame <= curr_frame + H; other_frame++) {
       float weight = 0;
       for(ushort neighbor = 1; neighbor <= K; neighbor++) {
            vector<short>* other_frame_coord = new vector<short>;

            short x = curr_frame_coord->at(0);
            short y = curr_frame_coord->at(1);

            other_frame_coord = aknn_across_frames[y][x]->at(other_frame - 1)->at(neighbor - 1);
            weight += exp(-calc_weighted_ssd(curr_frame, other_frame, curr_frame_coord, other_frame_coord, neighbor) / 
                  (2 * pow(noise_level_factor(), 2)));
       }
       normalization_factor += pow(gamma, abs(other_frame - curr_frame)) * weight;
   } 

   return normalization_factor;
}


float noise_level_factor()
{
    return 6;
}

float calc_input(short curr_frame, short other_frame, vector<short>* patch_coord_curr_frame, 
    vector<short>* patch_coord_other_frame)
{
    return 10;
}

