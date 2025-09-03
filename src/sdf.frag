uniform sampler2D u_jfaTex;
uniform vec2 u_resolution;

void main() {
  vec2 uv = gl_FragCoord.xy / u_resolution;
  vec2 nearestSeed = texture(u_jfaTex, uv).rg;
  float dist = clamp(distance(uv, nearestSeed), 0.f, 1.f);

  gl_FragColor = vec4(vec3(dist), 1.f);
}

