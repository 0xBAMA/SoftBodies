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
  int offset = 3; // for 4 anchored wheel points, this is 3 - to eat up the off-by-one from the one-indexed OBJ format

  // add anchored wheel control points here
  addNode( &simParameters.anchoredNodeMass, glm::vec3( -0.6125f, -0.38f,  1.95f ),  true ); // front left
  addNode( &simParameters.anchoredNodeMass, glm::vec3(  0.6125f, -0.38f,  1.95f ),  true ); // front right
  addNode( &simParameters.anchoredNodeMass, glm::vec3( -0.7f,    -0.38f, -0.86f ),  true ); // back left
  addNode( &simParameters.anchoredNodeMass, glm::vec3(  0.7f,    -0.38f, -0.86f ),  true ); // back right

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
      normals.push_back( glm::vec3( x, y, z ) );  // three floats determine the normal vector
    } else if ( read == "f" ) {
      char throwaway;
      int xIndex, yIndex, zIndex, nIndex;
      infile >> xIndex >> throwaway >> throwaway >> nIndex
             >> yIndex >> throwaway >> throwaway >> nIndex
             >> zIndex >> throwaway >> throwaway >> nIndex;
      // this needs the three node indices, as well as the normal from the normals list
      addFace( xIndex + offset, yIndex + offset, zIndex + offset, normals[ nIndex - 1 ] );
      // add the three edges of the triangle, since the obj export skips edges which are included in a face
      addEdge( xIndex + offset, yIndex + offset, CHASSIS );
      addEdge( zIndex + offset, yIndex + offset, CHASSIS );
      addEdge( zIndex + offset, xIndex + offset, CHASSIS );
    } else if ( read == "l" ) {
      int index1, index2;
      infile >> index1 >> index2;
      addEdge( index1 + offset, index2 + offset, CHASSIS ); // two node indices ( offset to match the list ), CHASSIS type
    }
  }
// suspension edge
  // front left
  addEdge( 0, 25, SUSPENSION );
  addEdge( 0, 35, SUSPENSION );
  addEdge( 0, 37, SUSPENSION );
  addEdge( 0, 39, SUSPENSION );
  addEdge( 0, 41, SUSPENSION );
  addEdge( 0, 42, SUSPENSION );
  addEdge( 0, 43, SUSPENSION );
  addEdge( 0, 44, SUSPENSION );
  addEdge( 0, 45, SUSPENSION );
  // mirrored SUSPENSION1 edges
  addEdge( 0, 13, SUSPENSION1 );
  addEdge( 0, 15, SUSPENSION1 );
  addEdge( 0, 17, SUSPENSION1 );
  addEdge( 0, 19, SUSPENSION1 );
  addEdge( 0, 20, SUSPENSION1 );
  addEdge( 0, 21, SUSPENSION1 );
  addEdge( 0, 22, SUSPENSION1 );
  addEdge( 0, 23, SUSPENSION1 );
  addEdge( 0, 24, SUSPENSION1 );

  // front right
  addEdge( 1, 13, SUSPENSION );
  addEdge( 1, 15, SUSPENSION );
  addEdge( 1, 17, SUSPENSION );
  addEdge( 1, 19, SUSPENSION );
  addEdge( 1, 20, SUSPENSION );
  addEdge( 1, 21, SUSPENSION );
  addEdge( 1, 22, SUSPENSION );
  addEdge( 1, 23, SUSPENSION );
  addEdge( 1, 24, SUSPENSION );
  // mirrored SUSPENSION1 edges
  addEdge( 1, 35, SUSPENSION1 );
  addEdge( 1, 25, SUSPENSION1 );
  addEdge( 1, 37, SUSPENSION1 );
  addEdge( 1, 39, SUSPENSION1 );
  addEdge( 1, 41, SUSPENSION1 );
  addEdge( 1, 42, SUSPENSION1 );
  addEdge( 1, 43, SUSPENSION1 );
  addEdge( 1, 44, SUSPENSION1 );
  addEdge( 1, 45, SUSPENSION1 );

  // back left
  addEdge( 2, 26, SUSPENSION );
  addEdge( 2, 28, SUSPENSION );
  addEdge( 2, 29, SUSPENSION );
  addEdge( 2, 30, SUSPENSION );
  addEdge( 2, 31, SUSPENSION );
  addEdge( 2, 32, SUSPENSION );
  addEdge( 2, 36, SUSPENSION );
  addEdge( 2, 38, SUSPENSION );
  // mirrored SUSPENSION1 edges
  addEdge( 2, 4, SUSPENSION1 );
  addEdge( 2, 6, SUSPENSION1 );
  addEdge( 2, 7, SUSPENSION1 );
  addEdge( 2, 8, SUSPENSION1 );
  addEdge( 2, 9, SUSPENSION1 );
  addEdge( 2, 10, SUSPENSION1 );
  addEdge( 2, 14, SUSPENSION1 );
  addEdge( 2, 16, SUSPENSION1 );

  // back right
  addEdge( 3, 4, SUSPENSION );
  addEdge( 3, 6, SUSPENSION );
  addEdge( 3, 7, SUSPENSION );
  addEdge( 3, 8, SUSPENSION );
  addEdge( 3, 9, SUSPENSION );
  addEdge( 3, 10, SUSPENSION );
  addEdge( 3, 14, SUSPENSION );
  addEdge( 3, 16, SUSPENSION );
  // mirrored SUSPENSION1 edges
  addEdge( 3, 26, SUSPENSION1 );
  addEdge( 3, 28, SUSPENSION1 );
  addEdge( 3, 29, SUSPENSION1 );
  addEdge( 3, 30, SUSPENSION1 );
  addEdge( 3, 31, SUSPENSION1 );
  addEdge( 3, 32, SUSPENSION1 );
  addEdge( 3, 36, SUSPENSION1 );
  addEdge( 3, 38, SUSPENSION1 );
}

void model::GPUSetup() {
  //VAO
  glGenVertexArrays( 1, &simGeometryVAO );
  glBindVertexArray( simGeometryVAO );

  //BUFFER
  glGenBuffers( 1, &simGeometryVBO );
  glBindBuffer( GL_ARRAY_BUFFER, simGeometryVBO );

  // SHADER for points, edges - from old implementation
  simGeometryShader = Shader( "resources/engine_code/shaders/main.vs.glsl", "resources/engine_code/shaders/main.fs.glsl" ).Program;

  // shader for faces - flat shading - todo
  // bodyPanelShader = Shader();
}

void model::passNewGPUData() {
  // populate the arrays out of the edge and node data
  std::vector< glm::vec3 > points;
  std::vector< glm::vec4 > colors;
  std::vector< glm::vec4 > tColors;

  // nodes
  drawParameters.nodesBase = points.size();
  for( auto n : nodes )
    points.push_back( n.position * displayParameters.scale ),
    colors.push_back( STEEL ),
    tColors.push_back( glm::vec4( 0. ) );
  drawParameters.nodesNum = points.size() - drawParameters.nodesBase;

  // edges
  drawParameters.edgesBase = points.size();
  for( auto e : edges ) {
    points.push_back( nodes[ e.node1 ].position * displayParameters.scale );
    points.push_back( nodes[ e.node2 ].position * displayParameters.scale );
    switch( e.type ) {
      case CHASSIS:
        colors.push_back( displayParameters.chassisColor );
        colors.push_back( displayParameters.chassisColor );
        break;
      case SUSPENSION:
        colors.push_back( displayParameters.suspColor );
        colors.push_back( displayParameters.suspColor );
        break;
      case SUSPENSION1:
        colors.push_back( displayParameters.susp1Color );
        colors.push_back( displayParameters.susp1Color );
        break;
      default: break;
    }
    tColors.push_back( BLACK ); // this will become a mapping that involves length and baselength
    tColors.push_back( BLACK );   // for the edge as well as the compColor and tensColor
  }
  drawParameters.edgesNum = points.size() - drawParameters.edgesBase;

  // faces
  drawParameters.facesBase = points.size();
  for( auto f : faces ) {
    // bring it in a touch, less collision with the chassis edges
    points.push_back( nodes[ f.node1 ].position * displayParameters.scale * displayParameters.chassisRescaleAmnt );
    points.push_back( nodes[ f.node2 ].position * displayParameters.scale * displayParameters.chassisRescaleAmnt );
    points.push_back( nodes[ f.node3 ].position * displayParameters.scale * displayParameters.chassisRescaleAmnt );

    colors.push_back( displayParameters.faceColor );
    colors.push_back( displayParameters.faceColor );
    colors.push_back( displayParameters.faceColor );

    tColors.push_back( BLACK );
    tColors.push_back( BLACK );
    tColors.push_back( BLACK );
  }
  drawParameters.facesNum = points.size() - drawParameters.facesBase;


  // buffer the data to the GPU
  uintptr_t numBytesPoints  =  points.size() * sizeof( glm::vec3 );
  uintptr_t numBytesColors  =  colors.size() * sizeof( glm::vec4 );
  uintptr_t numBytesTColors = tColors.size() * sizeof( glm::vec4 );

  // send it
  glBufferData(GL_ARRAY_BUFFER, numBytesPoints + numBytesColors + numBytesTColors, NULL, GL_DYNAMIC_DRAW);
  uint bufferbase = 0;
  glBufferSubData(GL_ARRAY_BUFFER, bufferbase, numBytesPoints, &points[0]);
  bufferbase += numBytesPoints;
  glBufferSubData(GL_ARRAY_BUFFER, bufferbase, numBytesColors, &colors[0]);
  bufferbase += numBytesColors;
  glBufferSubData(GL_ARRAY_BUFFER, bufferbase, numBytesTColors, &tColors[0]);


  // set up the pointers to the vertex data
  GLvoid* base = 0;
  glEnableVertexAttribArray( glGetAttribLocation( simGeometryShader, "vPosition" ));
  glVertexAttribPointer( glGetAttribLocation( simGeometryShader, "vPosition" ), 3, GL_FLOAT, GL_FALSE, 0, base );

  base = ( GLvoid* ) numBytesPoints;
  glEnableVertexAttribArray( glGetAttribLocation( simGeometryShader, "vColor" ));
  glVertexAttribPointer( glGetAttribLocation( simGeometryShader, "vColor" ), 4, GL_FLOAT, GL_FALSE, 0, base );

  base = ( GLvoid* ) ( numBytesPoints + numBytesColors );
  glEnableVertexAttribArray( glGetAttribLocation( simGeometryShader, "vtColor" ));
  glVertexAttribPointer( glGetAttribLocation( simGeometryShader, "vtColor" ), 4, GL_FLOAT, GL_FALSE, 0, base );
}

void model::colorModeSelect( int mode ) {
  glUniform1i( glGetUniformLocation( simGeometryShader, "color_mode" ), mode );
}

void model::updateUniforms() {
  SDL_DisplayMode dm;
  SDL_GetDesktopDisplayMode( 0, &dm );

  float AR = float( dm.w ) / float( dm.h );
  glm::mat4 proj = glm::perspective( glm::radians( 65.0f ), AR, 10.0f, 10.0f );

  glUniformMatrix4fv( glGetUniformLocation( simGeometryShader, "perspective" ), 1, GL_TRUE, glm::value_ptr( proj ) );
  glUniform1fv( glGetUniformLocation( simGeometryShader, "aspect_ratio" ), 1, &AR );

  // point size and point highlight
  glUniform1fv( glGetUniformLocation( simGeometryShader, "defaultPointSize"), 1, &drawParameters.pointScale );
  glUniform1i( glGetUniformLocation( simGeometryShader, "nodeSelect" ), nodeSelect );

  // rotation parameters
  glUniform1fv( glGetUniformLocation( simGeometryShader, "theta"), 1, &displayParameters.theta );
  glUniform1fv( glGetUniformLocation( simGeometryShader, "phi"), 1, &displayParameters.phi );
  glUniform1fv( glGetUniformLocation( simGeometryShader, "roll"), 1, &displayParameters.roll );

  // if ( ++nodeSelect == 4 ) nodeSelect = 0;
}


void model::update() {
  passNewGPUData();
}

void model::display() {
  // OpenGL config
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_LINE_SMOOTH );
  glEnable( GL_BLEND );
  glEnable( GL_PROGRAM_POINT_SIZE ); // lets you set pointsize in the shader - for suspension point select

  // use the first shader / VAO / VBO to draw the lines, points
  glUseProgram( simGeometryShader );
  glBindVertexArray( simGeometryVAO );
  glBindBuffer( GL_ARRAY_BUFFER, simGeometryVBO );

  updateUniforms();


  // chassis nodes
  if ( displayParameters.showChassisNodes ) {
    colorModeSelect( 0 );
    glDrawArrays( GL_POINTS, drawParameters.nodesBase, drawParameters.nodesNum );
  }

  // body panels
  if ( displayParameters.showChassisFaces ) {
    colorModeSelect( 2 );
    glDrawArrays( GL_TRIANGLES, drawParameters.facesBase, drawParameters.facesNum );
  }

  if ( displayParameters.showChassisEdges ) {
    if ( !displayParameters.tensionColorOnly ) {
      // regular colors - this is the chassis segments
      colorModeSelect( 0 );
      glLineWidth( drawParameters.lineScale );
      glDrawArrays( GL_LINES, drawParameters.edgesBase, drawParameters.edgesNum );
    }

    // tension colors / black outlines
    colorModeSelect( 1 );
    glLineWidth( drawParameters.outlineRatio * drawParameters.lineScale );
    glDrawArrays( GL_LINES, drawParameters.edgesBase, drawParameters.edgesNum );
  }


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
  nodes.push_back( n );
}

void model::addEdge( int nodeIndex1, int nodeIndex2, edgeType type ) {
  edge e;
  e.node1 = nodeIndex1;
  e.node2 = nodeIndex2;
  e.type = type;
  e.baseLength = glm::distance( nodes[ e.node1 ].position, nodes[ e.node2 ].position );
  edges.push_back( e );
}

// parameters tbd - probably just the
void model::addFace( int nodeIndex1, int nodeIndex2, int nodeIndex3, glm::vec3 normal ) {
  face f;
  f.node1 = nodeIndex1;
  f.node2 = nodeIndex2;
  f.node3 = nodeIndex3;

  // TODO: add normals

  faces.push_back( f );
}