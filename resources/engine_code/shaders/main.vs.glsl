#version 450

in vec3 vPosition;
in vec4 vColor;
in vec4 vtColor;

// mode 0 is for points
// mode 1 is for the colored lines
// mode 2 is to use the tension/compression colors / outlines
// mode 3 is for triangles
uniform int colorMode;


// point highlight - gl_PointSize must be set in all cases if it is set at all
uniform float defaultPointSize;
uniform int nodeSelect;

uniform float theta;
uniform float phi;
uniform float roll;

uniform float aspect_ratio;
uniform mat4 perspective;

out vec4 color;
out vec3 position;

//thanks to Neil Mendoza via http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat3 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

void main()
{
    position = vPosition;

    //side to side rotation first (about the x axis)
    mat3 rotx = rotationMatrix(vec3(1,0,0), theta);

    //then the up and down rotation (about the y axis)
    mat3 roty = rotationMatrix(vec3(0,1,0), phi);
    vec3 position = roty*rotx*vPosition;


    vec3 roll_vec = roty*rotx*vec3(0,0,1);
    mat3 roll_mat = rotationMatrix(roll_vec, roll);

    position *= roll_mat;


    gl_Position = vec4(position, 1.0);
    gl_Position.x /= aspect_ratio;
    // gl_Position *= perspective;


    switch ( colorMode ) {
      case 0: // points
        gl_Position.z -= 0.001;
        if ( gl_VertexID == nodeSelect ) { // highlight
          gl_PointSize = 1.618 * defaultPointSize;
          color = vec4( 1.0, 0.0, 0.0, 1.0 );
        } else {
          gl_PointSize = defaultPointSize;
          color = vColor;
        }
        break;

      case 1: // regular lines
        gl_Position.z -= 0.001;
        color = vColor;
        break;

      case 2: // outline lines
        color = vtColor;
        break;

      case 3: // triangles
        color = vColor;
        gl_Position.z += 0.001;
        break;
  }
}
