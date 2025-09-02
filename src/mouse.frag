uniform vec2 u_resolution;
uniform vec2 u_pos;
uniform vec3 u_color;
uniform float u_radius;

float sdfLineSquared(vec2 p, vec2 from, vec2 to) {
  vec2 toStart = p - from;
  vec2 line = to - from;
  float lineLengthSquared = dot(line, line);
  float t = clamp(dot(toStart, line) / lineLengthSquared, 0.0, 1.0);
  vec2 closestVector = toStart - line * t;

  return dot(closestVector, closestVector);
}

void main() {
  vec2 coord = gl_FragCoord.xy;
  vec2 from = u_pos;
  vec2 to = u_pos * 2;

  if (sdfLineSquared(coord, from, to) <= u_radius * u_radius)
    gl_FragColor = vec4(u_color, 1.f);
}

