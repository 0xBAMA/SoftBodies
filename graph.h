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



#include "shader.h"



//note here re: double precision 05/02/2020 4:40AM
//  my intention is to make the physics calculations more stable

//what type of edge is it?
enum edgetype
{
    CHASSIS,
    SUSPENSION,
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

        void set_AR()
        {
          //this is how you query the screen resolution
          SDL_DisplayMode dm;
          SDL_GetDesktopDisplayMode(0, &dm);

          float total_screen_width = dm.w - 150;
          float total_screen_height = dm.h - 150;

          float AR = total_screen_width/total_screen_height;

          cout << "ar is " << AR << endl;
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

    private:
        //OpenGL Data
        GLuint vao;
        GLuint buffer;
        GLuint shader;

        int num_bytes_points;
        int num_bytes_colors;

        int num_edges_to_draw;
        int num_nodes_to_draw;



        std::vector<node> nodes;
        std::vector<edge> edges;
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
    shader = Shader("main.vs.glsl", "main.fs.glsl").Program;

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
     glVertexAttribPointer(glGetAttribLocation(shader, "vPosition"), 3, GL_FLOAT, GL_FALSE, 0, ((GLvoid*) (0)));

     glEnableVertexAttribArray(glGetAttribLocation(shader, "vColor"));
     glVertexAttribPointer(glGetAttribLocation(shader, "vColor"), 4, GL_FLOAT, GL_FALSE, 0, ((GLvoid*) (num_bytes_points)));
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

  temp.node1 = node1;
  temp.node2 = node2;

  temp.type = type;

  edges.push_back(temp);
}

void graph::load_frame_points()
{
    nodes.clear();
    edges.clear();

  #define CHASSIS_NODE_MASS 3.0f
  #define CHASSIS_SCALE 0.3
    //lead with 4 anchor points

  //come back to this, need to be able to visualize the rest of the points before I figure this part out
  add_node(0, glm::dvec3(0,0,0), true);
  add_node(0, glm::dvec3(0,0,0), true);
  add_node(0, glm::dvec3(0,0,0), true);
  add_node(0, glm::dvec3(0,0,0), true);



    //then all the chassis nodes
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.582503, 0.839666, -1.224873), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.582503, 0.839666, -1.224873), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, 0.530926, -1.695220), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, 0.530926, -1.695220), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.582503, 0.839666, -1.224873), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, 0.530926, -1.695220), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, 0.530926, -1.695220), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.582503, 0.839666, -1.224873), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, -0.043335, -1.919456), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.541155, 0.049640, -2.444495), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.722724, 0.353912, -1.295972), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.722724, 0.496110, -0.402032), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.582503, 0.998272, -0.418518), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.582503, 0.976395, 0.314427), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.722724, 0.514985, 0.336304), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.722724, 0.504047, 0.336304), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, 0.478702, 0.762898), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.726352, 0.008355, -0.216081), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, 0.019293, 0.806651), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, -0.204942, 0.057377), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, -0.188535, 0.823059), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, 0.090392, 1.080109), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.210787, 0.976395, 0.440218), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.336924, 0.531393, 0.620700), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.091695, -0.177597, 0.697268), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.534656, 0.489640, 1.424666), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.234896, 0.456826, 1.457481), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.231663, 0.161491, 1.900483), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.609475, 0.090392, 1.080109), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.336924, 0.531393, 0.620700), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, -0.043335, -1.919456), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.541155, 0.049640, -2.444495), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.722724, 0.353912, -1.295972), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.722724, 0.496110, -0.402032), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.582503, 0.998272, -0.418518), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.582503, 0.976395, 0.314427), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.722724, 0.514985, 0.336304), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.722724, 0.504047, 0.336304), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, 0.478702, 0.762898), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.726352, 0.008355, -0.216081), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, 0.019293, 0.806651), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, -0.204942, 0.057377), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, -0.188535, 0.823059), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, 0.090392, 1.080109), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.210787, 0.976395, 0.440218), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.091695, -0.177597, 0.697268), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.534656, 0.489640, 1.424666), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.234896, 0.456826, 1.457481), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.231663, 0.161491, 1.900483), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.609475, 0.090392, 1.080109), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.194168, 0.998272, -0.527854), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.194168, 0.998272, -0.527854), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.203158, 0.530926, -1.695220), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( -0.194168, 0.839666, -1.224873), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.203158, 0.530926, -1.695220), false);
  add_node(CHASSIS_NODE_MASS, CHASSIS_SCALE*glm::dvec3( 0.194168, 0.839666, -1.224873), false);

  #define OFFSET 3
  add_edge(43+OFFSET, 30+OFFSET, CHASSIS);
  add_edge(21+OFFSET, 24+OFFSET, CHASSIS);
  add_edge(20+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(42+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(33+OFFSET, 46+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 25+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 46+OFFSET, CHASSIS);
  add_edge(25+OFFSET, 33+OFFSET, CHASSIS);
  add_edge(32+OFFSET, 53+OFFSET, CHASSIS);
  add_edge(10+OFFSET, 55+OFFSET, CHASSIS);
  add_edge(10+OFFSET, 6+OFFSET, CHASSIS);
  add_edge(32+OFFSET, 55+OFFSET, CHASSIS);
  add_edge(10+OFFSET, 53+OFFSET, CHASSIS);
  add_edge(53+OFFSET, 54+OFFSET, CHASSIS);
  add_edge(55+OFFSET, 56+OFFSET, CHASSIS);
  add_edge(56+OFFSET, 54+OFFSET, CHASSIS);
  add_edge(1+OFFSET, 56+OFFSET, CHASSIS);
  add_edge(55+OFFSET, 53+OFFSET, CHASSIS);
  add_edge(4+OFFSET, 55+OFFSET, CHASSIS);
  add_edge(22+OFFSET, 44+OFFSET, CHASSIS);
  add_edge(25+OFFSET, 28+OFFSET, CHASSIS);
  add_edge(46+OFFSET, 49+OFFSET, CHASSIS);
  add_edge(28+OFFSET, 46+OFFSET, CHASSIS);
  add_edge(25+OFFSET, 49+OFFSET, CHASSIS);
  add_edge(22+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(50+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 34+OFFSET, CHASSIS);
  add_edge(12+OFFSET, 33+OFFSET, CHASSIS);
  add_edge(12+OFFSET, 34+OFFSET, CHASSIS);
  add_edge(1+OFFSET, 33+OFFSET, CHASSIS);
  add_edge(54+OFFSET, 5+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 33+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 5+OFFSET, CHASSIS);
  add_edge(18+OFFSET, 40+OFFSET, CHASSIS);
  add_edge(4+OFFSET, 32+OFFSET, CHASSIS);
  add_edge(1+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(33+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 31+OFFSET, CHASSIS);
  add_edge(9+OFFSET, 33+OFFSET, CHASSIS);
  add_edge(10+OFFSET, 31+OFFSET, CHASSIS);
  add_edge(9+OFFSET, 32+OFFSET, CHASSIS);
  add_edge(9+OFFSET, 31+OFFSET, CHASSIS);
  add_edge(53+OFFSET, 6+OFFSET, CHASSIS);
  add_edge(10+OFFSET, 32+OFFSET, CHASSIS);
  add_edge(34+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(12+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(5+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(1+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(33+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(20+OFFSET, 35+OFFSET, CHASSIS);
  add_edge(13+OFFSET, 42+OFFSET, CHASSIS);
  add_edge(20+OFFSET, 42+OFFSET, CHASSIS);
  add_edge(25+OFFSET, 46+OFFSET, CHASSIS);
  add_edge(42+OFFSET, 46+OFFSET, CHASSIS);
  add_edge(43+OFFSET, 46+OFFSET, CHASSIS);
  add_edge(20+OFFSET, 25+OFFSET, CHASSIS);
  add_edge(21+OFFSET, 25+OFFSET, CHASSIS);
  add_edge(35+OFFSET, 45+OFFSET, CHASSIS);
  add_edge(13+OFFSET, 23+OFFSET, CHASSIS);
  add_edge(14+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(36+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(23+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(45+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(45+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(23+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(52+OFFSET, 51+OFFSET, CHASSIS);
  add_edge(13+OFFSET, 52+OFFSET, CHASSIS);
  add_edge(51+OFFSET, 35+OFFSET, CHASSIS);
  add_edge(23+OFFSET, 45+OFFSET, CHASSIS);
  add_edge(27+OFFSET, 49+OFFSET, CHASSIS);
  add_edge(28+OFFSET, 48+OFFSET, CHASSIS);
  add_edge(28+OFFSET, 49+OFFSET, CHASSIS);
  add_edge(27+OFFSET, 48+OFFSET, CHASSIS);
  add_edge(24+OFFSET, 30+OFFSET, CHASSIS);
  add_edge(24+OFFSET, 27+OFFSET, CHASSIS);
  add_edge(30+OFFSET, 48+OFFSET, CHASSIS);
  add_edge(38+OFFSET, 30+OFFSET, CHASSIS);
  add_edge(39+OFFSET, 30+OFFSET, CHASSIS);
  add_edge(44+OFFSET, 47+OFFSET, CHASSIS);
  add_edge(47+OFFSET, 49+OFFSET, CHASSIS);
  add_edge(44+OFFSET, 49+OFFSET, CHASSIS);
  add_edge(24+OFFSET, 48+OFFSET, CHASSIS);
  add_edge(48+OFFSET, 50+OFFSET, CHASSIS);
  add_edge(44+OFFSET, 50+OFFSET, CHASSIS);
  add_edge(48+OFFSET, 49+OFFSET, CHASSIS);
  add_edge(47+OFFSET, 48+OFFSET, CHASSIS);
  add_edge(39+OFFSET, 47+OFFSET, CHASSIS);
  add_edge(30+OFFSET, 46+OFFSET, CHASSIS);
  add_edge(45+OFFSET, 30+OFFSET, CHASSIS);
  add_edge(36+OFFSET, 45+OFFSET, CHASSIS);
  add_edge(39+OFFSET, 41+OFFSET, CHASSIS);
  add_edge(36+OFFSET, 39+OFFSET, CHASSIS);
  add_edge(41+OFFSET, 44+OFFSET, CHASSIS);
  add_edge(39+OFFSET, 44+OFFSET, CHASSIS);
  add_edge(43+OFFSET, 44+OFFSET, CHASSIS);
  add_edge(42+OFFSET, 43+OFFSET, CHASSIS);
  add_edge(41+OFFSET, 43+OFFSET, CHASSIS);
  add_edge(40+OFFSET, 42+OFFSET, CHASSIS);
  add_edge(41+OFFSET, 42+OFFSET, CHASSIS);
  add_edge(40+OFFSET, 41+OFFSET, CHASSIS);
  add_edge(38+OFFSET, 41+OFFSET, CHASSIS);
  add_edge(33+OFFSET, 40+OFFSET, CHASSIS);
  add_edge(34+OFFSET, 40+OFFSET, CHASSIS);
  add_edge(38+OFFSET, 40+OFFSET, CHASSIS);
  add_edge(34+OFFSET, 37+OFFSET, CHASSIS);
  add_edge(38+OFFSET, 39+OFFSET, CHASSIS);
  add_edge(37+OFFSET, 38+OFFSET, CHASSIS);
  add_edge(36+OFFSET, 37+OFFSET, CHASSIS);
  add_edge(35+OFFSET, 36+OFFSET, CHASSIS);
  add_edge(31+OFFSET, 33+OFFSET, CHASSIS);
  add_edge(6+OFFSET, 32+OFFSET, CHASSIS);
  add_edge(35+OFFSET, 34+OFFSET, CHASSIS);
  add_edge(8+OFFSET, 35+OFFSET, CHASSIS);
  add_edge(34+OFFSET, 33+OFFSET, CHASSIS);
  add_edge(8+OFFSET, 33+OFFSET, CHASSIS);
  add_edge(34+OFFSET, 8+OFFSET, CHASSIS);
  add_edge(33+OFFSET, 7+OFFSET, CHASSIS);
  add_edge(6+OFFSET, 31+OFFSET, CHASSIS);
  add_edge(31+OFFSET, 32+OFFSET, CHASSIS);
  add_edge(16+OFFSET, 24+OFFSET, CHASSIS);
  add_edge(17+OFFSET, 24+OFFSET, CHASSIS);
  add_edge(22+OFFSET, 26+OFFSET, CHASSIS);
  add_edge(26+OFFSET, 28+OFFSET, CHASSIS);
  add_edge(22+OFFSET, 28+OFFSET, CHASSIS);
  add_edge(27+OFFSET, 30+OFFSET, CHASSIS);
  add_edge(27+OFFSET, 29+OFFSET, CHASSIS);
  add_edge(22+OFFSET, 29+OFFSET, CHASSIS);
  add_edge(27+OFFSET, 28+OFFSET, CHASSIS);
  add_edge(26+OFFSET, 27+OFFSET, CHASSIS);
  add_edge(17+OFFSET, 26+OFFSET, CHASSIS);
  add_edge(24+OFFSET, 25+OFFSET, CHASSIS);
  add_edge(23+OFFSET, 24+OFFSET, CHASSIS);
  add_edge(14+OFFSET, 23+OFFSET, CHASSIS);
  add_edge(17+OFFSET, 19+OFFSET, CHASSIS);
  add_edge(14+OFFSET, 17+OFFSET, CHASSIS);
  add_edge(19+OFFSET, 22+OFFSET, CHASSIS);
  add_edge(17+OFFSET, 22+OFFSET, CHASSIS);
  add_edge(21+OFFSET, 22+OFFSET, CHASSIS);
  add_edge(20+OFFSET, 21+OFFSET, CHASSIS);
  add_edge(19+OFFSET, 21+OFFSET, CHASSIS);
  add_edge(18+OFFSET, 20+OFFSET, CHASSIS);
  add_edge(19+OFFSET, 20+OFFSET, CHASSIS);
  add_edge(18+OFFSET, 19+OFFSET, CHASSIS);
  add_edge(16+OFFSET, 19+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 18+OFFSET, CHASSIS);
  add_edge(12+OFFSET, 18+OFFSET, CHASSIS);
  add_edge(16+OFFSET, 18+OFFSET, CHASSIS);
  add_edge(12+OFFSET, 15+OFFSET, CHASSIS);
  add_edge(16+OFFSET, 17+OFFSET, CHASSIS);
  add_edge(15+OFFSET, 16+OFFSET, CHASSIS);
  add_edge(14+OFFSET, 15+OFFSET, CHASSIS);
  add_edge(13+OFFSET, 14+OFFSET, CHASSIS);
  add_edge(9+OFFSET, 11+OFFSET, CHASSIS);
  add_edge(4+OFFSET, 10+OFFSET, CHASSIS);
  add_edge(13+OFFSET, 12+OFFSET, CHASSIS);
  add_edge(2+OFFSET, 13+OFFSET, CHASSIS);
  add_edge(12+OFFSET, 11+OFFSET, CHASSIS);
  add_edge(2+OFFSET, 11+OFFSET, CHASSIS);
  add_edge(12+OFFSET, 2+OFFSET, CHASSIS);
  add_edge(11+OFFSET, 3+OFFSET, CHASSIS);
  add_edge(4+OFFSET, 9+OFFSET, CHASSIS);
  add_edge(9+OFFSET, 10+OFFSET, CHASSIS);
  add_edge(2+OFFSET, 4+OFFSET, CHASSIS);
  add_edge(1+OFFSET, 2+OFFSET, CHASSIS);
  add_edge(3+OFFSET, 4+OFFSET, CHASSIS);
  add_edge(5+OFFSET, 8+OFFSET, CHASSIS);
  add_edge(7+OFFSET, 8+OFFSET, CHASSIS);
  add_edge(6+OFFSET, 7+OFFSET, CHASSIS);
}

void graph::update()
{

}

void graph::send_points_and_edges_to_gpu()
{
  //using glBufferData to send the point data across
  std::vector<glm::vec3> points;
  std::vector<glm::vec4> colors;

  points.clear();
  colors.clear();

  for(auto n : nodes)
  {
    points.push_back(glm::vec3(n.position));
    colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
  }

  num_nodes_to_draw = points.size();

  for(auto e : edges)
  {
    points.push_back(glm::vec3(nodes[e.node1].position));
    points.push_back(glm::vec3(nodes[e.node2].position));

    if(e.type == CHASSIS)
    {
      colors.push_back(glm::vec4(1,0.5,0,0));
      colors.push_back(glm::vec4(1,0.5,0,0));
    }
    else
    {
      colors.push_back(glm::vec4(0,0,0,0));
      colors.push_back(glm::vec4(0,1,0,0));
    }
  }

  num_edges_to_draw = points.size() - num_nodes_to_draw;

  num_bytes_points = points.size() * sizeof(glm::vec3);
  num_bytes_colors = colors.size() * sizeof(glm::vec4);

  glBufferData(GL_ARRAY_BUFFER, num_bytes_points + num_bytes_colors, NULL, GL_DYNAMIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes_points, &points[0]);
  glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points, num_bytes_colors, &colors[0]);

}

void graph::display()
{
    //do your update

    //send your points
    send_points_and_edges_to_gpu();

    //do your draw commands
    glEnable(GL_DEPTH_TEST);

    // cout << "drawing " << num_nodes_to_draw << " points" << endl;
    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, num_nodes_to_draw);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, num_nodes_to_draw, num_edges_to_draw);
}
