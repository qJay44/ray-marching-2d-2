uniform vec2 u_pos;
uniform vec3 u_color;
uniform float u_radius;

void main() {
  vec2 dist = gl_FragCoord.xy - u_pos;
  float dd = dot(dist, dist);
  float rr = u_radius * u_radius;

  vec4 col = vec4(u_color, 1.f) * step(dd, rr);
  gl_FragColor = col;
}

