#version 450

in vec4 color;
in vec3 position;
out vec4 fragColor;

void main() {
  fragColor = color;
  fragColor.xyz *= 1.25 - gl_FragCoord.z;

  // if ( distance( gl_PointCoord.xy, vec2( 0.5, 0.5 ) ) >= 0.5 ) discard;
}
