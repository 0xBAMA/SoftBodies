#version 450

in vec4 color;
in vec3 position;
out vec4 fragColor;

void main()
{
    fragColor = color;
    fragColor.xyz *= 1.75-2*gl_FragCoord.z;

    //this is dumb, but position based effects have a lot of potential
    //fragColor *= clamp(3*(sin(1000*position.x)-0.5) +3*(sin(1000*position.y)-0.5) +3*(sin(1000*position.z)-0.5) , 0.75, 1.2);
    
    
    //distance from zero
    //fragColor *= 2 - sin(200*distance(position, vec3(0)));


    //trying some different stuff
    //if(fract(50*position.x) < 0.05 || fract(50*position.y) < 0.05 || fract(50*position.z) < 0.05)
    //{
      // fragColor = vec4(10*position.z,20*position.y,10*position.x,1); 
    //}
}
