#include "model.h"

#include <iostream>
#include <fstream>
#include <string>

model::model() {

  // load the points

  // spawn the threads

}

model::~model() {

}

void model::loadFramePoints() {
  // assumes obj file without the annotations -
    // specifically carFrameWPanels.obj which has a few lines already removed

  std::ifstream infile( "carFrameWPanels.obj" );
  std::vector< glm::vec3 > normals;

  // add the anchored points ( wheel control points )
  int offset = -1; // for 4 anchored wheel points, this is 3 - to eat up the off-by-one from the one-indexed OBJ format

  // add anchored wheel control points here



  while ( infile.peek() != EOF ) {
    std::string read;
    infile >> read;

    if ( read == "v" ) {

      float x, y, z;
      infile >> x >> y >> z;

      // add an unanchored node, with the configured node mass
      addNode( &simParameters.chassisNodeMass, glm::vec3( x, y, z ), false );

    } else if ( read == "vn" ) {

      float x, y, z;
      infile >> x >> y >> z;

      // three floats determine the normal vector
      normals.push_back( glm::vec3( x, y, z ) );

    } else if ( read == "f" ) {

      char throwaway;
      int xindex, yindex, zindex, nindex;
      infile >> xindex >> throwaway >> throwaway >> nindex
             >> yindex >> throwaway >> throwaway >> nindex
             >> zindex >> throwaway >> throwaway >> nindex;

      // this needs the three node indices, as well as the normal from the normals list
      // addFace( , normals[ nindex - 1 ] );

    } else if ( read == "l" ) {

      int index1, index2;
      infile >> index1 >> index2;

      // two node indices ( offset to match the list ), CHASSIS type
      addEdge( index1 + offset, index2 + offset, CHASSIS );

    }
  }

  // add suspension points here

}

void model::GPUSetup() {
  //VAO
  glGenVertexArrays( 1, &simGeometryVAO );
  glBindVertexArray( simGeometryVAO );

  //BUFFER
  glGenBuffers( 1, &simGeometryVBO );
  glBindBuffer( GL_ARRAY_BUFFER, simGeometryVBO );

  // SHADER for points, edges - from old implementation
  simGeometryShader = Shader( "old/shaders/main.vs.glsl", "old/shaders/main.fs.glsl" ).Program;

  // shader for faces - flat shading - todo

}

void model::passNewGPUData() {
  std::vector< glm::vec3 > points;
  std::vector< glm::vec4 > colors;
  std::vector< glm::vec4 > tColors;





  int numBytesPoints  =  points.size() * sizeof( glm::vec3 );
  int numBytesColors  =  colors.size() * sizeof( glm::vec4 );
  int numBytesTColors = tColors.size() * sizeof( glm::vec4 );

  // buffer the data to the GPU




  // set up the pointers to the vertex data

  GLvoid* vPositionBase = 0;
  glEnableVertexAttribArray( glGetAttribLocation( simGeometryShader, "vPosition" ));
  glVertexAttribPointer( glGetAttribLocation( simGeometryShader, "vPosition" ), 3, GL_FLOAT, GL_FALSE, 0, vPositionBase );

  GLvoid* vColorBase = ( GLvoid* ) numBytesPoints;
  glEnableVertexAttribArray( glGetAttribLocation( simGeometryShader, "vColor" ));
  glVertexAttribPointer( glGetAttribLocation( simGeometryShader, "vColor" ), 4, GL_FLOAT, GL_FALSE, 0, vColorBase );

  GLvoid* tColorBase = ( GLvoid* ) ( numBytesPoints + numBytesColors );
  glEnableVertexAttribArray( glGetAttribLocation( simGeometryShader, "vtColor" ));
  glVertexAttribPointer( glGetAttribLocation( simGeometryShader, "vtColor" ), 4, GL_FLOAT, GL_FALSE, 0, tColorBase );

}

void model::colorModeSelect( int mode ) {

}

void model::updateUniforms() {
  SDL_DisplayMode dm;
  SDL_GetDesktopDisplayMode(0, &dm);

  float AR = float( dm.w ) / float( dm.h );
  glm::mat4 proj = glm::perspective( glm::radians( 65.0f ), AR, 0.0f, 10.0f );

  glUniformMatrix4fv( glGetUniformLocation( simGeometryShader, "perspective" ), 1, GL_TRUE, glm::value_ptr( proj ) );
  glUniform1fv( glGetUniformLocation( simGeometryShader, "aspect_ratio" ), 1, &AR );
}

void model::update() {

}

void model::display() {
  // OpenGL config
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_LINE_SMOOTH );
  glEnable( GL_BLEND );
  glEnable( GL_PROGRAM_POINT_SIZE ); // lets you set pointsize in the shader

  // clear the buffer - color, depth

  // use the first shader / VAO / VBO to draw the lines, points
    // regular colors - this is the chassis segments
    // tension colors / black outlines
    // chassis nodes
    // points on the ground surface

  // use the other shader / VAO / VBO to do flat shaded polygons for the body panels
    // body panels
}

bool model::allThreadComplete() {
  return false;
}

void model::addNode( float* mass, glm::vec3 position, bool anchored ) {
  node n;
  n.mass = mass;
  n.anchored = anchored;
  n.position = n.oldPosition = position;
  n.velocity = n.oldVelocity = glm::vec3( 0.0 );
}

void model::addEdge( int nodeIndex1, int nodeIndex2, edgeType type ) {

}

// parameters tbd - probably just the
void model::addFace() {

}
