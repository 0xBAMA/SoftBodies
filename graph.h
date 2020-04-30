/*
 * =====================================================================================
 *
 *       Filename:  graph.h
 *
 *    Description: implementing softbodyies with realtime 3d visualization 
 *
 *        Version:  1.0
 *        Created:  04/29/2020 10:56:50 PM
 *       Compiler:  gcc
 *
 *         Author:  Jon Baker 
 *
 * =====================================================================================
 */


#include <stdio.h>

//stl includes
#include <vector>
#include <cmath>
#include <numeric>
#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <deque>
#include <chrono>
#include <cstdint>
#include <cstdlib>

//iostream aliases
using std::cin;
using std::cout;
using std::cerr;

using std::flush;
using std::endl;

//vector math library GLM
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#include "glm/glm.hpp" //general vector types
#include "glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp" //to send matricies gpu-side




// Initialize glew loader with glewInit()
#include <GL/glew.h>
//SDL includes - windowing, gl context, system info
#include <SDL2/SDL.h>
//allows you to run OpenGL inside of SDL2
#include <SDL2/SDL_opengl.h>




struct node
{

}; 

struct edge 
{ 

};

class graph 
{ 
    public:
        //constructor sets up opengl data initially
        //function to update physics simulation
        //function to update the point locations/colors on the GPU, from the CPU side information

        std::vector<glm::dvec4> points;


    private:
        //OpenGL Data
        GLuint vao;
        GLuint buffer;
        GLuint shader;
};
