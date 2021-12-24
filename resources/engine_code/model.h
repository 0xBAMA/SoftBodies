#ifndef MODEL
#define MODEL

#include "includes.h"

constexpr int numThreads = 12;          // worker threads for the update
enum threadState {
  WORKING,                              // thread is engaged in computation
  WAITING                               // thread has finished the work
};

enum edgeType {
  CHASSIS,                              // chassis member
  SUSPENSION,                           // suspension member
  SUSPENSION1                           // inboard suspension member
};

struct edge {
  edgeType type;                        // references global values of k, damping values
  float length, baseLength;             // current and initial edge length, used to determine compression / tension state
  int node1, node2;                     // indices the nodes on either end of the edge
};

struct face {
  int node1, node2, node3;              // the three points making up the triangle
  glm::vec3 normal;                     // surface normal for the triangle
};

struct node {
  float* mass;                          // pointer to mass of node ( easy runtime update )
  bool anchored;                        // anchored nodes are control points
  glm::vec3 position, oldPosition;      // current and previous position values
  glm::vec3 velocity, oldVelocity;      // current and previous velocity values
  std::vector< edge > edges;            // edges in which this node takes part
};

// consolidate simulation parameters
struct simParameterPack {
  bool  runSimulation       = true;     // toggle per frame update
  float timeScale           = 0.003;    // amount of time that passes per sim tick
  float gravity             = -8.0;     // scales the contribution of force of gravity

  float noiseAmplitudeScale = 0.1;      // scalar on the noise amplitude
  float noiseSpeed          = 5.0;      // how quickly the noise offset increases

  float chassisKConstant    = 1430.0;   // hooke's law spring constant for chassis edges
  float chassisDamping      = 31.3;     // damping factor for chassis edges
  float chassisNodeMass     = 3.0;      // mass of a chassis node
  float anchoredNodeMass    = 0.0;      // mass of an anchored node

  float suspensionKConstant = 790.0;    // hooke's law spring constant for suspension edges
  float suspensionDamping   = 5.22;     // damping factor for suspension edges
};

// consolidate display parameters
struct displayParameterPack {
  bool tensionColorOnly     = false;    // outlines are colored red in compression, blue in tension ( default )
  bool showChassisNodes     = true;     // show points associated with the chassis nodes
  bool showChassisEdges     = true;     // show lines associated with the chassis edges
  bool showSuspensionEdges  = true;     // show the lines associated with suspension edges
  bool showChassisFaces     = true;     // add this to the model, triangles using the same nodes as the edges

  float depthColorScale     = 1.0;      // adjust the weight of the depth coloring
  float chassisRescaleAmnt  = 0.985f;   // scales the polygons, to interfere with the lines less

  float wheelDiameter       = 0.2f;    // offset from the noise read, per wheel

  glm::vec4 outlineColor    = BLACK;    // the highlight color if tensionColor is off
  glm::vec4 compColor       = RED;      // the highlight color of the edges in compression ( tensionColor mode )
  glm::vec4 tensColor       = BLUE;     // the highlight color of the edges in tension ( tensionColor mode )

  glm::vec4 faceColor       = GREEN;    // color of the chassis faces
  glm::vec4 chassisColor    = STEEL;    // color of the chassis members
  glm::vec4 suspColor       = YELLOW;   // color of the suspension members
  glm::vec4 susp1Color      = BROWN;    // color of the inboard suspension members

  glm::vec4 groundLow       = G0;       // color of the ground at lowest point
  glm::vec4 groundHigh      = G1;       // color of the ground at highest point
  glm::vec4 background      = BG;       // OpenGL clear color

  float theta               = -0.27f;   // theta euler angle
  float phi                 = 3.896f;   // phi euler angle
  float roll                = -0.325f;  // additional roll parameter

  float scale               = 0.4f;     // scales the frame points, about zero
};

struct drawParameterPack {
// VBO indexing
  // nodes
  GLuint nodesBase;
  GLuint nodesNum;
  // edges
  GLuint edgesBase;
  GLuint edgesNum;
  // faces
  GLuint facesBase;
  GLuint facesNum;
  // ground
  GLuint groundBase;
  GLuint groundNum;

  // lines scaling
  float lineScale           = 10.0f;
  float outlineRatio        = 1.3f;

  // point scaling
  float pointScale          = 16.0f;

};

class model {
public:
  model();
  ~model();

  int nodeSelect = 0;

  // graph init
  void loadFramePoints();               // populate graph with nodes and edges
  void GPUSetup();                      // set up VAO, VBO, shaders

  // pass new GPU data
  void passNewGPUData();                // update vertex data
  void updateUniforms();                // update uniform variables

  // update functions for model
  void update( /* threadID */ );        // single threaded update - add threadID for threaded update

  // show the model
  void display();                       // render the latest vertex data with the simGeometryShader

  // to query sim completion
  bool allThreadComplete();             // iterate through thread array, return false if any thread is WORKING

  void colorModeSelect( int mode );     // the set of drawing colors to use

  // simulation and display parameter structs
  simParameterPack simParameters;
  displayParameterPack displayParameters;
  drawParameterPack drawParameters;

private:
  // called from loadFramePoints
  void addNode( float* mass, glm::vec3 position, bool anchored );
  void addEdge( int nodeIndex1, int nodeIndex2, edgeType type );
  void addFace( int nodeIndex1, int nodeIndex2, int nodeIndex3, glm::vec3 normal );

  // sim / display data
  std::vector< node > nodes;
  std::vector< edge > edges;
  std::vector< face > faces;

  // keeping the state of each thread
  threadState workerState[ numThreads ];
  void singleThreadSoftbodyUpdate();

  // OpenGL Data Handles
  GLuint simGeometryVAO;
  GLuint simGeometryVBO;
  GLuint simGeometryShader;
  GLuint bodyPanelShader;

  // ground data
  float getGroundPoint( float x, float y );
  FastNoise::SmartNode<> fnGenerator;
  float noiseOffset = 0.0;

};



#endif
