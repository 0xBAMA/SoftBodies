#version 450

in vec4 color;
in vec3 position;
out vec4 fragColor;

uniform int colorMode;

void main() {
  fragColor = color;

  float scalefactor = mix( smoothstep( 1.15, 0.0, gl_FragCoord.z ), 1.25 - gl_FragCoord.z, 0.3);
  fragColor.xyz *= scalefactor;

  if ( colorMode == 0 ) { // color handling for points
    float distanceToCenter = distance( gl_PointCoord.xy, vec2( 0.5, 0.5 ) );
    if ( distanceToCenter >= 0.5 ) discard;
  //   if ( distanceToCenter >= 0.4 ) fragColor = vec4( vec3( 0.0 ), color.a );
  }
}
