uniform sampler2D u_surfaceTex;
uniform vec2 u_resolution;

void main() {
  vec2 uv = gl_FragCoord.xy / u_resolution;
  float alpha = texture(u_surfaceTex, uv).a;
  gl_FragColor = vec4(uv * alpha, 0.f, 1.f);
}

