#version 450

in vec4 color;
out vec4 fragColor;

void main()
{
    fragColor = color;
    fragColor.xyz *= 1-gl_FragCoord.z;
}
