#include <stdio.h>
#include <string>
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
#include <vector>
#include <list>
#include "Halide.h"
#include "halide_image_io.h"
#include "temporal.h"
#include <unordered_map>

using namespace std;
using namespace Halide;
using namespace Halide::ConciseCasts;

const int width = 50;
const int height = 50;
const int n_channels = 3;
const int n_frames = 11;

typedef vector<short>* coord_t;
typedef vector<short>* coord_ssd_t;
typedef vector<coord_ssd_t>* heap_t;

heap_t neighbors_h[height][width];

Buffer<uint8_t> input[n_frames];

void get_input() 
{
    string path;

    for(int i = 0; i < n_frames; i++) {
        path = "./frames/" + to_string(i + 1) + ".png";
        input[i] = Tools::load_image(path);   
    }

}


Func D[n_frames], I[n_frames], Dx_right_in[n_frames], Dx_left_in[n_frames], Dy_up_in[n_frames], Dy_down_in[n_frames], 
        Dx_right_out[n_frames], Dx_left_out[n_frames], Dy_up_out[n_frames], Dy_down_out[n_frames];


void load_halide_functions(short frame) 
{
    const int s = 10;
    Var x, y, x_i, y_i, c, i, xo, yo, xi, yi;
  
    RDom u(-s, s, -s, s);
    
    RDom dx_left_out(-s - 1, -s, -s, s);
    RDom dx_left_in(-s, -s + 1, -s, s);
    
    RDom dx_right_out(s, s + 1, -s, s); 
    RDom dx_right_in(s - 1, s, -s, s);
    
    RDom dy_up_out(-s, s, -s - 1, -s);
    RDom dy_up_in(-s, s, -s, -s + 1);
    
    RDom dy_down_out(-s, s, s, s + 1);
    RDom dy_down_in(-s, s, s - 1, s);
    cout<<"loading halide input for frame "<<frame<<endl; 
    I[frame](x, y, c) = input[frame](clamp(x, s, width - s), clamp(y, s, height - s), c);   
    
    D[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + u.x, y + u.y, c) 
                            - I[frame](x_i + u.x, y_i + u.y, c)), 2)));
    
    Dx_left_in[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + dx_left_in.x, y + dx_left_in.y, c) 
                                    - I[frame](x_i + dx_left_in.x, y_i + dx_left_in.y, c)), 2)));
    
    Dx_right_in[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + dx_right_in.x, y + dx_right_in.y, c) 
                        - I[frame](x_i + dx_right_in.x, y_i + dx_right_in.y, c)), 2)));
    
    Dy_up_in[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + dy_up_in.x, y + dy_up_in.y, c) 
                                    - I[frame](x_i + dy_up_in.x, y_i + dy_up_in.y, c)), 2)));
    
    Dy_down_in[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + dy_down_in.x, y + dy_down_in.y, c) 
                                    - I[frame](x_i + dy_down_in.x, y_i + dy_down_in.y, c)), 2)));
    
    Dx_left_out[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + dx_left_out.x, y + dx_left_out.y, c) 
                                    - I[frame](x_i + dx_left_out.x, y_i + dx_left_out.y, c)), 2)));
    
    Dx_right_out[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + dx_right_out.x, y + dx_right_out.y, c) 
                                    - I[frame](x_i + dx_right_out.x, y_i + dx_right_out.y, c)), 2)));
    
    Dy_up_out[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + dy_up_out.x, y + dy_up_out.y, c) 
                                    - I[frame](x_i + dy_up_out.x, y_i + dy_up_out.y, c)), 2)));
    
    Dy_down_out[frame](x, y, x_i, y_i, c) = i16(sum(pow((I[frame](x + dy_down_out.x, y + dy_down_out.y, c) 
                                    - I[frame](x_i + dy_down_out.x, y_i + dy_down_out.y, c)), 2)));
}


void initiate_neighbors(short frame) 
{
    srand(17);
    for(short y = 0; y < height; y++)
        for(short x = 0; x < width; x++) {
            vector<vector<short>*>* neighbors;
            neighbors = new vector<vector<short>*>;
            generate_random_offsets_and_ssds(frame, x, y, neighbors); 
            neighbors_h[y][x] = neighbors;
         }
    cout<<"initial heap size "<<neighbors_h[0][0]->size()<<endl ;
}

void generate_random_offsets_and_ssds(short frame, short x, short y, vector<vector<short>*>* neighbors)
{
    for(ushort i = 0; i < K; i++) {
        vector<short>* ssd_and_offset = get_neighbor_ssd(frame, x, y);
        neighbors->push_back(ssd_and_offset);    
    } 
    sort_neighbors(neighbors);
    //if(x == 33 && y == 33)    
    //    cout<<"neighbors of 33, 33 when generating randoms"<<endl;
    //    print_neighbors(neighbors);    
}

vector<short>* get_neighbor_ssd(short frame, short x, short y) 
{
    vector<short>* coord = new vector<short>;
    Buffer<short> pix(1, 1, 1, 1, 3);
    short offset_x = get_random_x();
    short offset_y = get_random_y(); 
    pix.set_min(x, y, x + offset_x, y + offset_y);   
    D[frame].realize(pix);
    short ssd = pix(x, y, x + offset_x, y + offset_y, 0);
    coord->push_back(ssd);
    //cout<<"ssd "<<ssd; 
    coord->push_back(offset_x);
    coord->push_back(offset_y); 
    return coord;
}



void print_offset_and_ssd(vector<short>* offset_ssd) 
{
    cout<<"ssd "<<offset_ssd->at(0)<<" x_i "<<offset_ssd->at(1)
        <<" y_i "<<offset_ssd->at(2)<<endl;
}

void print_neighbors(vector<vector<short>*>* heap) 
{
    for(uint i = 0; i < heap->size(); i++) {
        print_offset_and_ssd(heap->at(i));
    }
}

//vector<short>* get_random_coord() 
//{
//    vector<short>* coord = new vector<short>;
//    coord->push_back(get_random_x());
//    coord->push_back(get_random_y());
//    return coord;
//}

short get_random_x() 
{
    float random_x;
    for(;;) {
        random_x = width / 3 * box_muller_trans((float) rand() / RAND_MAX);   
        if(random_x < width) 
            return (short) random_x;
    }
}

short get_random_y() 
{
    float random_y;
    for(;;) {
        random_y = height / 3 * box_muller_trans((float) rand() / RAND_MAX);   
        if(random_y < height) 
            return (short) random_y;
    }
}

// converts a uniform random variable into a standard normal variable
float box_muller_trans(float x) 
{
    return sqrt(-2 * log(x)) * cos(2 * M_PI * x);;
}




void propagate_neighbors(short frame) 
{
    for(short y = 1; y < height; ++y)     
        for(short x = 1; x < width; ++x) {
            propagate_scanline(frame, x, y);
        }
   
    for(short y = height - 2; y >= 0; --y)     
        for(short x = width - 2; x >= 0; --x) {
            propagate_reverse_scanline(frame, x, y);
        }
} 


void propagate_scanline(short frame, short x, short y)
{
    short offset_x, offset_y, offset_ssd; 
    
    for(vector<vector<short>*>::iterator it = neighbors_h[y][x - 1]->begin(); 
        it != neighbors_h[y][x - 1]->end(); ++it) {
        vector<short>* new_neighbor = new vector<short>;
        offset_ssd = (*it)->at(0);
        offset_x = (*it)->at(1);
        offset_y = (*it)->at(2);         
        new_neighbor->push_back(calculate_new_ssd(frame, x, y, offset_x, offset_y, offset_ssd, 'r'));
        new_neighbor->push_back((short) offset_x + 1);
        new_neighbor->push_back(offset_y);
        neighbors_h[y][x]->push_back(new_neighbor);
    }

    for(vector<vector<short>*>::iterator it = neighbors_h[y - 1][x]->begin(); 
        it != neighbors_h[y - 1][x]->end(); ++it) {
        coord_ssd_t new_neighbor = new vector<short>;
        offset_ssd = (*it)->at(0);
        offset_x = (*it)->at(1);
        offset_y = (*it)->at(2); 
        new_neighbor->push_back(calculate_new_ssd(frame, x, y, offset_x, offset_y, offset_ssd, 'd'));
        new_neighbor->push_back(offset_x);
        new_neighbor->push_back((short) offset_y + 1);
        neighbors_h[y][x]->push_back(new_neighbor);
    }
    sort_neighbors(neighbors_h[y][x]);
}

void propagate_reverse_scanline(short frame, short x, short y)
{
    short offset_x, offset_y, offset_ssd;
    for(vector<vector<short>*>::iterator it = neighbors_h[y][x + 1]->begin(); 
        it != neighbors_h[y][x + 1]->end(); ++it) {
        coord_ssd_t new_neighbor = new vector<short>;
        offset_ssd = (*it)->at(0);
        offset_x = (*it)->at(1);
        offset_y = (*it)->at(2); 
        new_neighbor->push_back(calculate_new_ssd(frame, x, y, offset_x, offset_y, offset_ssd, 'l')); 
        new_neighbor->push_back((short) offset_x - 1);
        new_neighbor->push_back(offset_y);
    }

    for(vector<vector<short>*>::iterator it = neighbors_h[y + 1][x]->begin(); 
        it != neighbors_h[y + 1][x]->end(); ++it) {
        coord_ssd_t new_neighbor = new vector<short>;
        offset_ssd = (*it)->at(0);
        offset_x = (*it)->at(1);
        offset_y = (*it)->at(2);
        new_neighbor->push_back(calculate_new_ssd(frame, x, y, offset_x, offset_y, offset_ssd, 'u')); 
        new_neighbor->push_back(offset_x);
        new_neighbor->push_back((short)offset_y - 1);
    }
    sort_neighbors(neighbors_h[y][x]);
}

short calculate_new_ssd(short frame, short x, short y, short offset_x, 
                        short offset_y, short offset_ssd, char direction)
{
    Buffer<short> add_b(1, 1, 1, 1, 3);
    Buffer<short> subtract_b(1, 1, 1, 1, 3);
    
    add_b.set_min(x, y, x + offset_x, y + offset_y, 0);
    subtract_b.set_min(x, y, x + offset_x, y + offset_y, 0);
    
    switch(direction) {
        
        case 'r':
            Dx_right_out[frame].realize(add_b); 
            Dx_left_in[frame].realize(subtract_b);
            break;

        case 'd':
            Dy_down_out[frame].realize(add_b);
            Dy_up_in[frame].realize(subtract_b);
            break;

        case 'l':
            Dx_left_out[frame].realize(add_b); 
            Dx_right_in[frame].realize(subtract_b);
            break;

        case 'u':
            Dy_up_out[frame].realize(add_b); 
            Dy_down_in[frame].realize(subtract_b);
            break;
 
        default:
            cerr<<"wrong direction character"<<endl;
    }
    return offset_ssd - subtract_b(x, y, x + offset_x, y + offset_y, 0) 
        + add_b(x, y, x + offset_x, y + offset_y, 0);
}


void random_search(short frame) 
{
    int M = min(log(width / 3), (double)K);    
    cout<<"M "<<M<<endl;
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {     
            for(int i = 0; i < M; ++i) {
                vector<short>* random_guess = new vector<short>;
                short offset_x = get_random_x() * pow(0.5, i);
                short offset_y = get_random_y() * pow(0.5, i);
                 
                Buffer<short> pix(1, 1, 1, 1, 3);

                pix.set_min(x, y, x + offset_x, y + offset_y, 0);   
                D[frame].realize(pix);

                short ssd = pix(x, y, x + offset_x, y + offset_y, 0);
            
                random_guess->push_back(ssd);
                random_guess->push_back(offset_x);
                random_guess->push_back(offset_y);

                //cout<<"ssd "<<ssd<<" offset_x "<<offset_x<<" offset_y "<<offset_y<<endl;

                neighbors_h[y][x]->push_back(random_guess);
            }
            
            sort_neighbors(neighbors_h[y][x]);
            //for(vector<vector<short>*>::iterator it = neighbors_h[y][x]->begin(); 
            //    it != neighbors_h[y][x]->end(); ++it) {
            //    cout<<"ssd "<<(*it)->at(0)<<" offset_x "<<(*it)->at(1)<<" offset_y "
            //        <<(*it)->at(2)<<endl;         
            //}
        }
    } 
}

vector<vector<short>*>* get_neighbors(short x, short y)
{
    return neighbors_h[x][y];
}
