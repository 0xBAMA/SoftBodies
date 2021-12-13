#include "model.h"

#include <iostream>
#include <fstream>
#include <string>

model::model() {

}

model::~model() {

}

void model::loadFramePoints() {
  // assumes obj file without the annotations -
    // specifically carFrameWPanels.obj which has a few lines already removed

  std::ifstream infile( "carFrameWPanels.obj" );

  while ( infile.peek() != EOF ) {
    std::string read;
    infile >> read;

    if ( read == "v" ) {         // read in three floats and add node to list

      float x, y, z;
      infile >> x >> y >> z;

    } else if ( read == "vn" ) { // read in three floats and add normal vector to list

      float x, y, z;
      infile >> x >> y >> z;

    } else if ( read == "f" ) {  // parsing this is a little more complicated with the slashes
      char throwaway;

      int xindex, yindex, zindex, nindex;
      infile >> xindex >> throwaway >> throwaway >> nindex >> yindex >> throwaway >> throwaway >> nindex >> zindex >> throwaway >> throwaway >> nindex;

      cout << " face with indices " << xindex << " " << yindex << " " << zindex << " and normal " << nindex << endl;


    } else if ( read == "l" ) {  // read in two ints, reference nodes, and add an edge

      int p1, p2;
      infile >> p1 >> p2;

    }
  }
}

void model::GPUSetup() {

}

void model::passNewGPUData() {

}

void model::updateUniforms() {

}

void model::update() {

}

bool model::allThreadComplete() {
  return false;
}

void model::addNode() {

}

void model::addEdge() {

}

void model::addFace() {

}
