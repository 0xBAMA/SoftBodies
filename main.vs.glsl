#version 450

in vec3 vPosition;
in vec4 vColor;

uniform float theta;
uniform float phi;

uniform float aspect_ratio;
uniform mat4 perspective;

out vec4 color;

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
    //side to side rotation first (about the x axis)
    mat3 rotx = rotationMatrix(vec3(1,0,0), theta);

    //then the up and down rotation (about the y axis)
    mat3 roty = rotationMatrix(vec3(0,1,0), phi);
    vec3 position = roty*rotx*vPosition;

    gl_Position = vec4(position, 1.0);
    gl_Position.x /= aspect_ratio;
    // gl_Position *= perspective;
    color = vColor;
}
