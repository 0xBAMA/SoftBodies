#version 450

in vec3 vPosition;
in vec4 vColor;
in vec4 vtColor;

//color mode = 0 is just use regular colors
//color mode = 1 is to use the tension/compression colors 
uniform int color_mode;

uniform float theta;
uniform float phi;
uniform float roll;

uniform float aspect_ratio;
uniform mat4 perspective;

out vec4 color;
out vec3 position;

//thanks to Neil Mendoza via http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat3 rotationMatrix(vec3 axis, float angle)
{
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

    if(color_mode == 0)
    {
        color = vColor;
        if(color.r > 0.9)
            gl_PointSize = 20.0;
        else
            gl_PointSize = 7.0;
    }
    else
    {
        color = vtColor;
    }
}
