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
#include <thread>

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



#include "../shaders/shader.h"
#include "perlin.h"


//note here re: double precision 05/02/2020 4:40AM
//  my intention is to make the physics calculations more stable

//what type of edge is it?
enum edgetype
{
    CHASSIS,
    SUSPENSION,
    SUSPENSION1,
    TIRE
};

struct edge
{
    edgetype type;  //allows this to be like a 'material', distinct k and damping values held for each edgetype - set by GUI

    double base_length; //how long was this link when it was established
    int node1, node2;   //the indexes of the two nodes involved with this edge
};

struct node
{
    float mass;             //mass of this point

    glm::dvec3 position;        //current value of position
    glm::dvec3 velocity;        //current value of velocity

    glm::dvec3 old_position;        //last update's values
    glm::dvec3 old_velocity;

    bool anchored;              //is this a control point?

    std::vector<edge> edges;        //list of all edges which this node is part of
};



class graph
{
    public:
        //constructor sets up opengl data initially
        graph();

        //low level graph representation
        void add_node(float mass, glm::dvec3 position, bool anchored);
        void add_edge(int node1, int node2, edgetype type);

        //function to load in points, links, using above
        void load_frame_points();

        //function to update physics simulation
        void update();

        //threaded update
        void threaded_update();
        void one_thread_update(int index);



        //function to update the point locations/colors on the GPU, from the CPU side information
        void send_points_and_edges_to_gpu();

        //function to display the points, lines
        void display();

        void set_rotate_phi(float phi)
        {
          glUniform1fv(glGetUniformLocation(shader, "phi"), 1, &phi);
        }

        void set_rotate_theta(float theta)
        {
          glUniform1fv(glGetUniformLocation(shader, "theta"), 1, &theta);
        }

        void set_rotate_roll(float roll)
        {
          glUniform1fv(glGetUniformLocation(shader, "roll"), 1, &roll);
        }

        void set_AR()
        {
          //this is how you query the screen resolution
          SDL_DisplayMode dm;
          SDL_GetDesktopDisplayMode(0, &dm);

          float total_screen_width = dm.w - 150;
          float total_screen_height = dm.h - 150;

          float AR = total_screen_width/total_screen_height;

          glUniform1fv(glGetUniformLocation(shader, "aspect_ratio"), 1, &AR);
        }

        void set_persp()
        {
          SDL_DisplayMode dm;
          SDL_GetDesktopDisplayMode(0, &dm);

          float total_screen_width = dm.w - 150;
          float total_screen_height = dm.h - 150;

          float AR = total_screen_width/total_screen_height;

          glm::mat4 proj = glm::perspective(glm::radians(65.0f), AR, 0.0f, 10.0f);

          glUniformMatrix4fv(glGetUniformLocation(shader, "perspective"), 1, GL_TRUE, glm::value_ptr(proj));
        }

        void set_highlight_index(uint i) {highlight_index = i;}

        //which set of colors to use?
        void color_mode() {glUniform1i(glGetUniformLocation(shader,"color_mode"),0);}
        void tcolor_mode() {glUniform1i(glGetUniformLocation(shader,"color_mode"),1);}

        float timescale = 0.01f;
        float gravity = -2.0f;
        float noise_scale = 0.088f;
        float noise_speed = 5.0f;

        float chassis_k = 1430.0f;
        float chassis_damp = 31.3f;

        float chassis_mass = 3.0f;

        float suspension_k = 790.0f;
        float suspension_damp = 5.22f;

        bool com = false;
        bool tension_color_only = false;

    private:

        //OpenGL Data
        GLuint vao;
        GLuint buffer;
        GLuint shader;

        int num_bytes_points;
        int num_bytes_colors;
        int num_bytes_tcolors;

        int chassis_start, chassis_num;
        int suspension_start, suspension_num;
        int nodes_start, nodes_num;
        int ground_start, ground_num;



        uint highlight_index = 0;

        std::vector<node> nodes;
        std::vector<edge> edges;
    
        PerlinNoise p;  //offset keeps track of vehicle position
        glm::vec3 offset = glm::vec3(0,0,0);
};


graph::graph()
{
    //VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //BUFFER
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    //SHADER
    shader = Shader("shaders/main.vs.glsl", "shaders/main.fs.glsl").Program;

    //get your data
    load_frame_points();

    //send it to the gpu
    send_points_and_edges_to_gpu();

    glUseProgram(shader);

    set_rotate_phi(0.0f);
    set_rotate_theta(0.0f);

    set_AR();
    set_persp();

     // Initialize the vertex position attribute from the vertex shader
     glEnableVertexAttribArray(glGetAttribLocation(shader, "vPosition"));
     glVertexAttribPointer(glGetAttribLocation(shader, "vPosition"), 3, GL_FLOAT, GL_FALSE, 0, ((GLvoid*)  (static_cast<const char*>(0) + 0)));

     glEnableVertexAttribArray(glGetAttribLocation(shader, "vColor"));
     glVertexAttribPointer(glGetAttribLocation(shader, "vColor"), 4, GL_FLOAT, GL_FALSE, 0, ((GLvoid*) (static_cast<const char*>(0) + num_bytes_points)));

     glEnableVertexAttribArray(glGetAttribLocation(shader, "vtColor"));
     glVertexAttribPointer(glGetAttribLocation(shader, "vtColor"), 4, GL_FLOAT, GL_FALSE, 0, ((GLvoid*) (static_cast<const char*>(0) + num_bytes_points+num_bytes_colors)));
}

void graph::add_node(float mass, glm::dvec3 position, bool anchored)
{
    //add a node to the list of nodes
    //you're responsible for all indexing etc in terms of adding anchor nodes to the obj output
    //requires input mass, position, bool anchored
    node temp;

    temp.mass = mass;
    temp.position = temp.old_position = position;
    temp.velocity = temp.old_velocity = glm::dvec3(0,0,0);
    temp.anchored = anchored;
    temp.edges.clear();

    nodes.push_back(temp);

}

void graph::add_edge(int node1, int node2, edgetype type)
{
  edge temp;

  temp.base_length = glm::distance(nodes[node1].position, nodes[node2].position);

  temp.type = type;

  temp.node1 = node1;
  temp.node2 = node2;
  nodes[node1].edges.push_back(temp);

  edges.push_back(temp);

  temp.node1 = node2;
  temp.node2 = node1;
  nodes[node2].edges.push_back(temp);

}

void graph::load_frame_points()
{
    nodes.clear();
    edges.clear();

  #define CHASSIS_NODE_MASS 3.0f
  #define CHASSIS_SCALE 0.3
    //lead with 4 anchor points

  #define WIDTH 0.2
  //come back to this, need to be able to visualize the rest of the points before I figure this part out
  add_node(0, glm::dvec3(-WIDTH*0.875, -0.1, 0.6), true);
  add_node(0, glm::dvec3( WIDTH*0.875, -0.1, 0.6), true);
  add_node(0, glm::dvec3(-WIDTH, -0.1, -0.25), true);
  add_node(0, glm::dvec3( WIDTH, -0.1, -0.25), true);


    //then all the chassis nodes
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, -0.203593, -1.521659), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.582503, 0.679409, -0.827076), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, 0.370669, -1.297424), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.722724, 0.335853, -0.004235), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.582503, 0.838015, -0.020722), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.722724, 0.193655, -0.898175), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.541155, -0.110617, -2.046698), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.582503, 0.816138, 0.712224), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.722724, 0.354728, 0.734101), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.722724, 0.343790, 0.734101), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, 0.318445, 1.160695), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.726352, -0.151903, 0.181716), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, -0.140964, 1.204448), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, -0.365200, 0.455174), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, -0.348792, 1.220856), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.210787, 0.816138, 0.838015), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.336924, 0.371136, 1.018497), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.091695, -0.337854, 1.095065), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.534656, 0.329383, 1.822463), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.234896, 0.296568, 1.855278), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.231663, 0.001234, 2.298280), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, -0.069865, 1.477906), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, -0.203593, -1.521659), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, 0.370669, -1.297424), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.722724, 0.335853, -0.004235), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.582503, 0.838015, -0.020722), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.722724, 0.193655, -0.898175), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.582503, 0.679409, -0.827076), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.541155, -0.110617, -2.046698), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.582503, 0.816138, 0.712224), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.722724, 0.354728, 0.734101), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.722724, 0.343790, 0.734101), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, 0.318445, 1.160695), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.726352, -0.151903, 0.181716), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, -0.140964, 1.204448), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, -0.365200, 0.455174), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, -0.348792, 1.220856), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, -0.069865, 1.477906), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.210787, 0.816138, 0.838015), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.336924, 0.371136, 1.018497), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.091695, -0.337854, 1.095065), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.534656, 0.329383, 1.822463), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.234896, 0.296568, 1.855278), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.231663, 0.001234, 2.298280), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.194168, 0.838015, -0.130058), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.194168, 0.838015, -0.130058), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.203158, 0.370669, -1.297424), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.203158, 0.370669, -1.297424), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.194168, 0.679409, -0.827076), false);
    add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.194168, 0.679409, -0.827076), false);

    #define OFFSET 3
    add_edge(1+OFFSET, 7+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 3+OFFSET, CHASSIS);
    add_edge(4+OFFSET, 6+OFFSET, CHASSIS);
    add_edge(5+OFFSET, 4+OFFSET, CHASSIS);
    add_edge(1+OFFSET, 6+OFFSET, CHASSIS);
    add_edge(5+OFFSET, 8+OFFSET, CHASSIS);
    add_edge(8+OFFSET, 10+OFFSET, CHASSIS);
    add_edge(10+OFFSET, 11+OFFSET, CHASSIS);
    add_edge(4+OFFSET, 10+OFFSET, CHASSIS);
    add_edge(10+OFFSET, 12+OFFSET, CHASSIS);
    add_edge(4+OFFSET, 12+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 12+OFFSET, CHASSIS);
    add_edge(10+OFFSET, 13+OFFSET, CHASSIS);
    add_edge(12+OFFSET, 13+OFFSET, CHASSIS);
    add_edge(13+OFFSET, 14+OFFSET, CHASSIS);
    add_edge(12+OFFSET, 14+OFFSET, CHASSIS);
    add_edge(13+OFFSET, 15+OFFSET, CHASSIS);
    add_edge(14+OFFSET, 15+OFFSET, CHASSIS);
    add_edge(8+OFFSET, 11+OFFSET, CHASSIS);
    add_edge(11+OFFSET, 13+OFFSET, CHASSIS);
    add_edge(8+OFFSET, 16+OFFSET, CHASSIS);
    add_edge(16+OFFSET, 17+OFFSET, CHASSIS);
    add_edge(17+OFFSET, 18+OFFSET, CHASSIS);
    add_edge(11+OFFSET, 19+OFFSET, CHASSIS);
    add_edge(19+OFFSET, 20+OFFSET, CHASSIS);
    add_edge(20+OFFSET, 21+OFFSET, CHASSIS);
    add_edge(20+OFFSET, 22+OFFSET, CHASSIS);
    add_edge(20+OFFSET, 40+OFFSET, CHASSIS);
    add_edge(19+OFFSET, 21+OFFSET, CHASSIS);
    add_edge(11+OFFSET, 17+OFFSET, CHASSIS);
    add_edge(10+OFFSET, 17+OFFSET, CHASSIS);
    add_edge(23+OFFSET, 29+OFFSET, CHASSIS);
    add_edge(24+OFFSET, 23+OFFSET, CHASSIS);
    add_edge(25+OFFSET, 28+OFFSET, CHASSIS);
    add_edge(28+OFFSET, 27+OFFSET, CHASSIS);
    add_edge(25+OFFSET, 27+OFFSET, CHASSIS);
    add_edge(28+OFFSET, 26+OFFSET, CHASSIS);
    add_edge(26+OFFSET, 25+OFFSET, CHASSIS);
    add_edge(24+OFFSET, 29+OFFSET, CHASSIS);
    add_edge(23+OFFSET, 27+OFFSET, CHASSIS);
    add_edge(26+OFFSET, 30+OFFSET, CHASSIS);
    add_edge(30+OFFSET, 32+OFFSET, CHASSIS);
    add_edge(32+OFFSET, 33+OFFSET, CHASSIS);
    add_edge(25+OFFSET, 32+OFFSET, CHASSIS);
    add_edge(32+OFFSET, 34+OFFSET, CHASSIS);
    add_edge(25+OFFSET, 34+OFFSET, CHASSIS);
    add_edge(27+OFFSET, 34+OFFSET, CHASSIS);
    add_edge(32+OFFSET, 35+OFFSET, CHASSIS);
    add_edge(34+OFFSET, 35+OFFSET, CHASSIS);
    add_edge(35+OFFSET, 36+OFFSET, CHASSIS);
    add_edge(34+OFFSET, 36+OFFSET, CHASSIS);
    add_edge(35+OFFSET, 37+OFFSET, CHASSIS);
    add_edge(36+OFFSET, 37+OFFSET, CHASSIS);
    add_edge(37+OFFSET, 38+OFFSET, CHASSIS);
    add_edge(33+OFFSET, 38+OFFSET, CHASSIS);
    add_edge(35+OFFSET, 38+OFFSET, CHASSIS);
    add_edge(30+OFFSET, 33+OFFSET, CHASSIS);
    add_edge(33+OFFSET, 35+OFFSET, CHASSIS);
    add_edge(30+OFFSET, 39+OFFSET, CHASSIS);
    add_edge(39+OFFSET, 40+OFFSET, CHASSIS);
    add_edge(40+OFFSET, 41+OFFSET, CHASSIS);
    add_edge(33+OFFSET, 42+OFFSET, CHASSIS);
    add_edge(42+OFFSET, 43+OFFSET, CHASSIS);
    add_edge(43+OFFSET, 44+OFFSET, CHASSIS);
    add_edge(17+OFFSET, 43+OFFSET, CHASSIS);
    add_edge(38+OFFSET, 44+OFFSET, CHASSIS);
    add_edge(42+OFFSET, 44+OFFSET, CHASSIS);
    add_edge(38+OFFSET, 42+OFFSET, CHASSIS);
    add_edge(33+OFFSET, 40+OFFSET, CHASSIS);
    add_edge(32+OFFSET, 40+OFFSET, CHASSIS);
    add_edge(40+OFFSET, 43+OFFSET, CHASSIS);
    add_edge(17+OFFSET, 20+OFFSET, CHASSIS);
    add_edge(17+OFFSET, 40+OFFSET, CHASSIS);
    add_edge(20+OFFSET, 43+OFFSET, CHASSIS);
    add_edge(21+OFFSET, 44+OFFSET, CHASSIS);
    add_edge(21+OFFSET, 43+OFFSET, CHASSIS);
    add_edge(20+OFFSET, 44+OFFSET, CHASSIS);
    add_edge(16+OFFSET, 39+OFFSET, CHASSIS);
    add_edge(46+OFFSET, 26+OFFSET, CHASSIS);
    add_edge(5+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(45+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(16+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(39+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(39+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(16+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(30+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(8+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(5+OFFSET, 16+OFFSET, CHASSIS);
    add_edge(26+OFFSET, 39+OFFSET, CHASSIS);
    add_edge(15+OFFSET, 18+OFFSET, CHASSIS);
    add_edge(14+OFFSET, 18+OFFSET, CHASSIS);
    add_edge(37+OFFSET, 41+OFFSET, CHASSIS);
    add_edge(36+OFFSET, 41+OFFSET, CHASSIS);
    add_edge(18+OFFSET, 41+OFFSET, CHASSIS);
    add_edge(14+OFFSET, 36+OFFSET, CHASSIS);
    add_edge(5+OFFSET, 36+OFFSET, CHASSIS);
    add_edge(14+OFFSET, 26+OFFSET, CHASSIS);
    add_edge(27+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(2+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(4+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(25+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(7+OFFSET, 29+OFFSET, CHASSIS);
    add_edge(48+OFFSET, 24+OFFSET, CHASSIS);
    add_edge(1+OFFSET, 23+OFFSET, CHASSIS);
    add_edge(1+OFFSET, 29+OFFSET, CHASSIS);
    add_edge(7+OFFSET, 23+OFFSET, CHASSIS);
    add_edge(1+OFFSET, 27+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 23+OFFSET, CHASSIS);
    add_edge(27+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(2+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(12+OFFSET, 34+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 27+OFFSET, CHASSIS);
    add_edge(2+OFFSET, 27+OFFSET, CHASSIS);
    add_edge(4+OFFSET, 25+OFFSET, CHASSIS);
    add_edge(4+OFFSET, 27+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 25+OFFSET, CHASSIS);
    add_edge(18+OFFSET, 44+OFFSET, CHASSIS);
    add_edge(21+OFFSET, 41+OFFSET, CHASSIS);
    add_edge(41+OFFSET, 44+OFFSET, CHASSIS);
    add_edge(18+OFFSET, 21+OFFSET, CHASSIS);
    add_edge(47+OFFSET, 48+OFFSET, CHASSIS);
    add_edge(2+OFFSET, 49+OFFSET, CHASSIS);
    add_edge(49+OFFSET, 50+OFFSET, CHASSIS);
    add_edge(47+OFFSET, 49+OFFSET, CHASSIS);
    add_edge(48+OFFSET, 50+OFFSET, CHASSIS);
    add_edge(7+OFFSET, 48+OFFSET, CHASSIS);
    add_edge(29+OFFSET, 47+OFFSET, CHASSIS);
    add_edge(7+OFFSET, 24+OFFSET, CHASSIS);
    add_edge(7+OFFSET, 47+OFFSET, CHASSIS);
    add_edge(29+OFFSET, 48+OFFSET, CHASSIS);
    add_edge(18+OFFSET, 27+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 41+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 18+OFFSET, CHASSIS);
    add_edge(27+OFFSET, 41+OFFSET, CHASSIS);
    add_edge(36+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(14+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(15+OFFSET, 17+OFFSET, CHASSIS);
    add_edge(37+OFFSET, 40+OFFSET, CHASSIS);
    add_edge(3+OFFSET, 1+OFFSET, CHASSIS);
    add_edge(2+OFFSET, 3+OFFSET, CHASSIS);
    add_edge(4+OFFSET, 2+OFFSET, CHASSIS);
    add_edge(2+OFFSET, 6+OFFSET, CHASSIS);
    add_edge(2+OFFSET, 5+OFFSET, CHASSIS);
    add_edge(3+OFFSET, 7+OFFSET, CHASSIS);
    add_edge(15+OFFSET, 22+OFFSET, CHASSIS);
    add_edge(11+OFFSET, 22+OFFSET, CHASSIS);
    add_edge(13+OFFSET, 22+OFFSET, CHASSIS);
    add_edge(22+OFFSET, 21+OFFSET, CHASSIS);
    add_edge(22+OFFSET, 19+OFFSET, CHASSIS);
    add_edge(28+OFFSET, 24+OFFSET, CHASSIS);
    add_edge(27+OFFSET, 24+OFFSET, CHASSIS);
    add_edge(43+OFFSET, 38+OFFSET, CHASSIS);
    add_edge(28+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(3+OFFSET, 29+OFFSET, CHASSIS);
    add_edge(6+OFFSET, 28+OFFSET, CHASSIS);
    add_edge(50+OFFSET, 28+OFFSET, CHASSIS);
    add_edge(38+OFFSET, 46+OFFSET, CHASSIS);
    add_edge(22+OFFSET, 45+OFFSET, CHASSIS);
    add_edge(22+OFFSET, 38+OFFSET, CHASSIS);
    add_edge(3+OFFSET, 47+OFFSET, CHASSIS);

    add_edge(1, 14, SUSPENSION);
    add_edge(1, 16, SUSPENSION);
    add_edge(1, 18, SUSPENSION);
    add_edge(1, 20, SUSPENSION);
    add_edge(1, 21, SUSPENSION);
    add_edge(1, 22, SUSPENSION);
    add_edge(1, 23, SUSPENSION);
    add_edge(1, 24, SUSPENSION);
    add_edge(1, 25, SUSPENSION);

    add_edge(0, 14, SUSPENSION1);
    add_edge(0, 16, SUSPENSION1);
    add_edge(0, 18, SUSPENSION1);
    add_edge(0, 20, SUSPENSION1);
    add_edge(0, 21, SUSPENSION1);
    add_edge(0, 22, SUSPENSION1);
    add_edge(0, 23, SUSPENSION1);
    add_edge(0, 24, SUSPENSION1);
    add_edge(0, 25, SUSPENSION1);

    add_edge(1, 36, SUSPENSION1);
    add_edge(1, 38, SUSPENSION1);
    add_edge(1, 40, SUSPENSION1);
    add_edge(1, 41, SUSPENSION1);
    add_edge(1, 43, SUSPENSION1);
    add_edge(1, 44, SUSPENSION1);
    add_edge(1, 45, SUSPENSION1);
    add_edge(1, 46, SUSPENSION1);
    add_edge(1, 47, SUSPENSION1);

    add_edge(0, 36, SUSPENSION);
    add_edge(0, 38, SUSPENSION);
    add_edge(0, 40, SUSPENSION);
    add_edge(0, 41, SUSPENSION);
    add_edge(0, 43, SUSPENSION);
    add_edge(0, 44, SUSPENSION);
    add_edge(0, 45, SUSPENSION);
    add_edge(0, 46, SUSPENSION);
    add_edge(0, 47, SUSPENSION);

    //back wheels
    add_edge(2, 4, SUSPENSION1);
    add_edge(2, 6, SUSPENSION1);
    add_edge(2, 7, SUSPENSION1);
    add_edge(2, 8, SUSPENSION1);
    add_edge(2, 9, SUSPENSION1);
    add_edge(2, 15, SUSPENSION1);
    add_edge(2, 17, SUSPENSION1);

    add_edge(2, 26, SUSPENSION);
    add_edge(2, 27, SUSPENSION);
    add_edge(2, 28, SUSPENSION);
    add_edge(2, 29, SUSPENSION);
    add_edge(2, 30, SUSPENSION);
    add_edge(2, 37, SUSPENSION);
    add_edge(2, 39, SUSPENSION);

    add_edge(3, 4, SUSPENSION);
    add_edge(3, 6, SUSPENSION);
    add_edge(3, 7, SUSPENSION);
    add_edge(3, 8, SUSPENSION);
    add_edge(3, 9, SUSPENSION);
    add_edge(3, 15, SUSPENSION);
    add_edge(3, 17, SUSPENSION);

    add_edge(3, 26, SUSPENSION1);
    add_edge(3, 27, SUSPENSION1);
    add_edge(3, 28, SUSPENSION1);
    add_edge(3, 29, SUSPENSION1);
    add_edge(3, 30, SUSPENSION1);
    add_edge(3, 37, SUSPENSION1);
    add_edge(3, 39, SUSPENSION1);
}

void graph::update()
{
    //some random direction
    glm::vec3 direction_of_travel = glm::vec3(0,0,1); 

    //static position in the noise space

    //advance that offset, based on the noise speed
    offset += noise_speed*timescale*direction_of_travel;

    glm::vec3 driver_front_sample_point = offset;
    glm::vec3 passenger_front_sample_point = offset;
    glm::vec3 driver_rear_sample_point = offset;
    glm::vec3 passenger_rear_sample_point = offset;

    glm::vec3 temp;

    temp = nodes[0].position;
    temp.y = 0; 

    driver_front_sample_point += temp;

    temp = nodes[1].position;
    temp.y = 0;

    passenger_front_sample_point += temp;

    temp = nodes[2].position;
    temp.y = 0;

    driver_rear_sample_point += temp;
    
    temp = nodes[3].position;
    temp.y = 0;
    
    passenger_rear_sample_point += temp;
        
    //new anchor positions
    nodes[0].position.y = nodes[0].old_position.y + noise_scale*(-0.7+p.noise(driver_front_sample_point.x, driver_front_sample_point.y, driver_front_sample_point.z));
    nodes[1].position.y = nodes[1].old_position.y + noise_scale*(-0.7+p.noise(passenger_front_sample_point.x, passenger_front_sample_point.y, passenger_front_sample_point.z));
    nodes[2].position.y = nodes[2].old_position.y + noise_scale*(-0.7+p.noise(driver_rear_sample_point.x, driver_rear_sample_point.y, driver_rear_sample_point.z));
    nodes[3].position.y = nodes[3].old_position.y + noise_scale*(-0.7+p.noise(passenger_rear_sample_point.x, passenger_rear_sample_point.y, passenger_rear_sample_point.z));


    //back up old positions and velocities for unanchored nodes
    for(auto& n : nodes)
    {
        if(!n.anchored)
        {
            n.old_position = n.position;
            n.old_velocity = n.velocity;
        }
    }


    for(auto& n : nodes)
    {
        if(!n.anchored)
        {
            glm::dvec3 force(0,0,0);
            double k = 0;
            double d = 0;


            //get your forces from all the connections - accumulate in force vector
            for(uint i = 0; i < n.edges.size(); i++)
            {
                switch(n.edges[i].type)
                {
                    case CHASSIS:
                        k = chassis_k;
                        d = chassis_damp;
                        break;
                    case SUSPENSION:
                    case SUSPENSION1:
                        k = suspension_k;
                        d = suspension_damp;
                        break;
                    case TIRE:
                        //tbd
                        break;
                }

                //get positions of the two nodes involved
                glm::dvec3 my_position = n.old_position;
                glm::dvec3 ur_position = nodes[n.edges[i].node2].anchored ?
                    nodes[n.edges[i].node2].position :          //use 'new position' for anchored nodes (new_position is up to date)
                    nodes[n.edges[i].node2].old_position;       //use old position for unanchored nodes (old value is what you use)

                //less than 1 is shorter, greater than 1 is longer than base length
                double spring_ratio = glm::distance(my_position, ur_position) / n.edges[i].base_length;

                //spring force
                force += -k * glm::normalize(my_position - ur_position) * (spring_ratio - 1);
                //should this be normalized or no?
                //force += -k * (my_position - ur_position) * (spring_ratio - 1);

                //damping force
                force -= d * n.old_velocity;
            }

            //add gravity
            force += (double)n.mass * glm::dvec3(0,-gravity,0);

            //get the resulting acceleration
            glm::dvec3 acceleration = force/(double)n.mass;

            //compute the new velocity
            n.velocity = n.old_velocity + acceleration * (double)timescale;

            //get the new position
            n.position = n.old_position + n.velocity * (double)timescale;
        }
    }

    //send your points
    send_points_and_edges_to_gpu();

}





void graph::threaded_update()
{

    //some random direction
    glm::vec3 direction_of_travel = glm::vec3(0,0,1); 

    //static position in the noise space

    //advance that offset, based on the noise speed
    offset += noise_speed*timescale*direction_of_travel;

    glm::vec3 driver_front_sample_point = offset;
    glm::vec3 passenger_front_sample_point = offset;
    glm::vec3 driver_rear_sample_point = offset;
    glm::vec3 passenger_rear_sample_point = offset;

    glm::vec3 temp;

    temp = nodes[0].position;
    temp.y = 0; 

    driver_front_sample_point += temp;

    temp = nodes[1].position;
    temp.y = 0;

    passenger_front_sample_point += temp;

    temp = nodes[2].position;
    temp.y = 0;

    driver_rear_sample_point += temp;
    
    temp = nodes[3].position;
    temp.y = 0;
    
    passenger_rear_sample_point += temp;
        
    //new anchor positions
    nodes[0].position.y = nodes[0].old_position.y + noise_scale*(-0.7+p.noise(driver_front_sample_point.x, driver_front_sample_point.y, driver_front_sample_point.z));
    nodes[1].position.y = nodes[1].old_position.y + noise_scale*(-0.7+p.noise(passenger_front_sample_point.x, passenger_front_sample_point.y, passenger_front_sample_point.z));
    nodes[2].position.y = nodes[2].old_position.y + noise_scale*(-0.7+p.noise(driver_rear_sample_point.x, driver_rear_sample_point.y, driver_rear_sample_point.z));
    nodes[3].position.y = nodes[3].old_position.y + noise_scale*(-0.7+p.noise(passenger_rear_sample_point.x, passenger_rear_sample_point.y, passenger_rear_sample_point.z));


    //back up old positions and velocities for unanchored nodes
    for(auto& n : nodes)
    {
        if(!n.anchored)
        {
            n.old_position = n.position;
            n.old_velocity = n.velocity;
        }
    }


    std::thread t0(&graph::one_thread_update, this, 0);
    std::thread t1(&graph::one_thread_update, this, 1);
    std::thread t2(&graph::one_thread_update, this, 2);
    std::thread t3(&graph::one_thread_update, this, 3);
    std::thread t4(&graph::one_thread_update, this, 4);
    std::thread t5(&graph::one_thread_update, this, 5);
    std::thread t6(&graph::one_thread_update, this, 6);
    std::thread t7(&graph::one_thread_update, this, 7);
    t0.join();
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();

    //send your points
    send_points_and_edges_to_gpu();
}

void graph::one_thread_update(int index)
{
    for(std::vector<node>::iterator n = nodes.begin()+4+index; n < nodes.end(); n+=4)
    {
        if(!n->anchored)
        {
            glm::dvec3 force(0,0,0);
            double k = 0;
            double d = 0;


            //get your forces from all the connections - accumulate in force vector
            for(uint i = 0; i < n->edges.size(); i++)
            {
                switch(n->edges[i].type)
                {
                    case CHASSIS:
                        k = chassis_k;
                        d = chassis_damp;
                        break;
                    case SUSPENSION:
                    case SUSPENSION1:
                        k = suspension_k;
                        d = suspension_damp;
                        break;
                    case TIRE:
                        //tbd
                        break;
                }

                //get positions of the two nodes involved
                glm::dvec3 my_position = n->old_position;
                glm::dvec3 ur_position = nodes[n->edges[i].node2].anchored ?
                    nodes[n->edges[i].node2].position :          //use 'new position' for anchored nodes (new_position is up to date)
                    nodes[n->edges[i].node2].old_position;       //use old position for unanchored nodes (old value is what you use)

                //less than 1 is shorter, greater than 1 is longer than base length
                double spring_ratio = glm::distance(my_position, ur_position) / n->edges[i].base_length;

                //spring force
                force += -k * glm::normalize(my_position - ur_position) * (spring_ratio - 1);
                //should this be normalized or no?
                //force += -k * (my_position - ur_position) * (spring_ratio - 1);

                //damping force
                force -= d * n->old_velocity;
            }

            //add gravity
            force += (double)n->mass * glm::dvec3(0,-gravity,0);

            //get the resulting acceleration
            glm::dvec3 acceleration = force/(double)n->mass;

            //compute the new velocity
            n->velocity = n->old_velocity + acceleration * (double)timescale;

            //get the new position
            n->position = n->old_position + n->velocity * (double)timescale;
        }
    }
}










void graph::send_points_and_edges_to_gpu()
{
  //using glBufferData to send the point data across
  std::vector<glm::vec3> points;
  std::vector<glm::vec4> colors;
  std::vector<glm::vec4> tcolors;

  points.clear();
  colors.clear();
  tcolors.clear();

  //for(auto n : nodes)
  //{
    //points.push_back(glm::vec3(n.position));
    //if(n.anchored)
      //colors.push_back(glm::vec4(1.0, 0.5, 0.5, 1.0));
    //else
      //colors.push_back(glm::vec4(1.0, 1.0, 0.75, 1.0));
  //}


    
    //#define CHASSIS_NODE_COLOR  glm::vec4(0.36,0.350,0.305,1.0)
    //#define CHASSIS_COLOR       glm::vec4(0.400,0.400,0.320,1.0)
    //#define SUSPENSION_COLOR    glm::vec4(0.1,0.2,0.2,0.5)
    //#define SUSPENSION1_COLOR   glm::vec4(0.2,0.1,0.1,0.3)

    //#define CHASSIS_COLOR       glm::vec4(0.42, 0.24, 0.0, 0.3)

    #define CHASSIS_NODE_COLOR  glm::vec4(0.23, 0.26, 0.08,1.0)
    #define CHASSIS_COLOR       glm::vec4(0.25, 0.28, 0.10,1.0)
    #define SUSPENSION_COLOR   glm::vec4(0.43, 0.36, 0.11,1.0)
    #define SUSPENSION1_COLOR    glm::vec4(0.3, 0.2, 0.1, 0.5)

    #define HIGHLIGHT_COLOR     glm::vec4(0.1,0.2,0.1,0.4)
    #define HIGHLIGHT_COLOR_B   glm::vec4(0.1,0.4,0.6,0.4)

    #define HIGHLIGHT_TENSION       glm::vec4(0,0,0.5,1)
    #define HIGHLIGHT_COMPRESSION   glm::vec4(0.5,0,0,1)

    #define HIGHLIGHT_BLACK            glm::vec4(0,0,0,1)


    //going to need to refine how this works in order to reflect the
    //tension/compression in a more continuous way, some kind of
    //gradient going on


  nodes_start = points.size();
  for(uint i = 0; i < nodes.size(); i++)
  {
    points.push_back(glm::vec3(nodes[i].position));
    tcolors.push_back(glm::vec4(nodes[i].position, 1.0f));

    if(nodes[i].anchored)
      colors.push_back(glm::vec4(0.4, 0.6, 0.5, 1.0));
    else
        if(i == highlight_index)
            colors.push_back(glm::vec4(1.0, 0.0, 0.2, 1.0));
        else
        {
            //float f = dis(gen);
            //colors.push_back(glm::vec4(f,0.6*f,0.2*f,1));
            colors.push_back(CHASSIS_NODE_COLOR);
        }
  }
  nodes_num = points.size() - nodes_start;

  chassis_start = points.size();
  for(auto e : edges)
  {
    if(e.type == CHASSIS)
    {
        points.push_back(glm::vec3(nodes[e.node1].position));
        points.push_back(glm::vec3(nodes[e.node2].position));

        if(tension_color_only)
        {
            if(glm::distance(nodes[e.node1].position, nodes[e.node2].position) < e.base_length)
            {
                tcolors.push_back(HIGHLIGHT_COMPRESSION);
                tcolors.push_back(HIGHLIGHT_COMPRESSION);
            }
            else
            {
                tcolors.push_back(HIGHLIGHT_TENSION);
                tcolors.push_back(HIGHLIGHT_TENSION);
            }
        }
        else
        {
            tcolors.push_back(HIGHLIGHT_BLACK);
            tcolors.push_back(HIGHLIGHT_BLACK);
        }

        colors.push_back(CHASSIS_COLOR);
        colors.push_back(CHASSIS_COLOR);
    }
  }
  chassis_num = points.size() - chassis_start;

  suspension_start = points.size();
  for(auto e : edges)
  {
    if(e.type == SUSPENSION)
    {
        points.push_back(glm::vec3(nodes[e.node1].position));
        points.push_back(glm::vec3(nodes[e.node2].position));

        colors.push_back(SUSPENSION_COLOR);
        colors.push_back(SUSPENSION_COLOR);

        if(tension_color_only)
        {
            if(glm::distance(nodes[e.node1].position, nodes[e.node2].position) < e.base_length)
            {
                tcolors.push_back(HIGHLIGHT_COMPRESSION);
                tcolors.push_back(HIGHLIGHT_COMPRESSION);
            }
            else
            {
                tcolors.push_back(HIGHLIGHT_TENSION);
                tcolors.push_back(HIGHLIGHT_TENSION);
            }
        }
        else
        {
            tcolors.push_back(HIGHLIGHT_BLACK);
            tcolors.push_back(HIGHLIGHT_BLACK);
        }
    }
    else if(e.type == SUSPENSION1)
    {
        points.push_back(glm::vec3(nodes[e.node1].position));
        points.push_back(glm::vec3(nodes[e.node2].position));

        colors.push_back(SUSPENSION1_COLOR);
        colors.push_back(SUSPENSION1_COLOR);

        if(tension_color_only)
        {
            if(glm::distance(nodes[e.node1].position, nodes[e.node2].position) < e.base_length)
            {
                tcolors.push_back(HIGHLIGHT_COMPRESSION);
                tcolors.push_back(HIGHLIGHT_COMPRESSION);
            }
            else
            {
                tcolors.push_back(HIGHLIGHT_TENSION);
                tcolors.push_back(HIGHLIGHT_TENSION);
            }
        }
        else
        {
            tcolors.push_back(HIGHLIGHT_BLACK);
            tcolors.push_back(HIGHLIGHT_BLACK);
        }
    }
  }
  suspension_num = points.size() - suspension_start;




#define NUM_STEPS_LR 100 
#define NUM_STEPS_FB 100

//#define FRONT_EXTENT
  ground_start = points.size();
    float noise_read;

  for(double horizontal = -0.65; horizontal <= 0.65; horizontal += 1.3/NUM_STEPS_LR)
      for(double vertical = -0.9; vertical <= 0.9; vertical += 1.8/NUM_STEPS_FB)
      {
        noise_read = p.noise(horizontal + offset.x, 0, vertical + offset.z);
        points.push_back(glm::vec3(horizontal, -0.2+noise_scale*(-0.7+noise_read), vertical));
        colors.push_back(glm::vec4(noise_read, 0.5*noise_read, 0, 1));
        tcolors.push_back(glm::vec4(noise_read));
      }

    /* 
  for(double horizontal = -0.05; horizontal <= 0.05; horizontal += 0.1/NUM_STEPS_LR)
      for(double vertical = -0.0618; vertical <= 0.0618; vertical += 0.13/NUM_STEPS_FB)
      {
     
        glm::vec3 wheeloffset = nodes[0].position;
        noise_read = p.noise(horizontal+offset.x+wheeloffset.x, 0, vertical+offset.z+wheeloffset.z);
        
        points.push_back(glm::vec3(horizontal+wheeloffset.x, -0.2+noise_scale*(-0.7+noise_read), vertical+wheeloffset.z));
        colors.push_back(glm::vec4(noise_read, 0.5*noise_read, 0, 1));
        tcolors.push_back(glm::vec4(noise_read));
      
      
        wheeloffset = nodes[1].position;
        noise_read = p.noise(horizontal+offset.x+wheeloffset.x, 0, vertical+offset.z+wheeloffset.z);
        
        points.push_back(glm::vec3(horizontal+wheeloffset.x, -0.2+noise_scale*(-0.7+noise_read), vertical+wheeloffset.z));
        colors.push_back(glm::vec4(noise_read, 0.5*noise_read, 0, 1));
        tcolors.push_back(glm::vec4(noise_read));
     

        wheeloffset = nodes[2].position;
        noise_read = p.noise(horizontal+offset.x+wheeloffset.x, 0, vertical+offset.z+wheeloffset.z);
        
        points.push_back(glm::vec3(horizontal+wheeloffset.x, -0.2+noise_scale*(-0.7+noise_read), vertical+wheeloffset.z));
        colors.push_back(glm::vec4(noise_read, 0.5*noise_read, 0, 1));
        tcolors.push_back(glm::vec4(noise_read));
      
      
        wheeloffset = nodes[3].position;
        noise_read = p.noise(horizontal+offset.x+wheeloffset.x, 0, vertical+offset.z+wheeloffset.z);
        
        points.push_back(glm::vec3(horizontal+wheeloffset.x, -0.2+noise_scale*(-0.7+noise_read), vertical+wheeloffset.z));
        colors.push_back(glm::vec4(noise_read, 0.5*noise_read, 0, 1));
        tcolors.push_back(glm::vec4(noise_read));
      }


    
    std::random_device rd;
    std::mt19937 gen(rd()); 
    
    std::uniform_real_distribution<float> disx(-0.5, 0.5);
    std::uniform_real_distribution<float> disz(-0.9, 0.9);

    std::uniform_real_distribution<float> dish(-0.03, 0.02);
    std::uniform_real_distribution<float> disc(-0.1, 0.1);


    float x,z;
    for(int i = 0; i < 10000; i++)
    {
        x = disx(gen);
        z = disz(gen);
        noise_read = p.noise(offset.x+x, 0, offset.z+z);
        
       
        if(p.noise(15*(offset.x+x),0,5*(offset.z+z)) < 0.6)
        {
            colors.push_back(glm::vec4(0.45, 0.6*noise_read+disc(gen), 0.3, 1));
            points.push_back(glm::vec3(x, -0.25+dish(gen)+noise_scale*(-0.7+noise_read), z));
        }
        else
        {
            colors.push_back(glm::vec4(0.3+3*disc(gen), 0.4+4*disc(gen), 0, 1));
            points.push_back(glm::vec3(x, -0.225+dish(gen)+noise_scale*(-0.7+noise_read), z));
        }

        tcolors.push_back(glm::vec4(noise_read));

    }

    */

    

  ground_num = points.size() - ground_start;


  num_bytes_points = points.size() * sizeof(glm::vec3);
  num_bytes_colors = colors.size() * sizeof(glm::vec4);
  num_bytes_tcolors = tcolors.size() * sizeof(glm::vec4);

    glBufferData(GL_ARRAY_BUFFER, num_bytes_points + num_bytes_colors + num_bytes_tcolors, NULL, GL_DYNAMIC_DRAW);
    uint base = 0;
    glBufferSubData(GL_ARRAY_BUFFER, base, num_bytes_points, &points[0]);
    base += num_bytes_points;
    glBufferSubData(GL_ARRAY_BUFFER, base, num_bytes_colors, &colors[0]);
    base += num_bytes_colors;
    glBufferSubData(GL_ARRAY_BUFFER, base, num_bytes_tcolors, &tcolors[0]);
}

void graph::display()
{
    //do your draw commands
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glEnable(GL_PROGRAM_POINT_SIZE); // lets you set pointsize in the shader

    //regular colors
    color_mode();
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, suspension_start, suspension_num);

    glLineWidth(6.0f);
    glDrawArrays(GL_LINES, chassis_start, chassis_num);

    //tension color
    tcolor_mode();
    glLineWidth(4.0f);
    glDrawArrays(GL_LINES, suspension_start, suspension_num);

    glLineWidth(8.0f);
    glDrawArrays(GL_LINES, chassis_start, chassis_num);

    //chassis nodes
    color_mode();
    glPointSize(7.0f);
    glDrawArrays(GL_POINTS, nodes_start, nodes_num);

    glPointSize(2.0f);
    glDrawArrays(GL_POINTS, ground_start, ground_num);
}
