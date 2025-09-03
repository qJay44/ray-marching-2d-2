uniform sampler2D u_inputTex;
uniform vec2 u_resolution;
uniform int u_offset;

void main() {
  vec4 nearestSeed = vec4(-2.f);
  float nearestDist = 1e6f;
  vec2 uv = gl_FragCoord.xy / u_resolution;

  for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
      vec2 sampleUV = uv + vec2(x, y) * u_offset / u_resolution;

      // Bounds check
      if (floor(sampleUV) != vec2(0.f))
        continue;

      vec4 sampleValue = texture(u_inputTex, sampleUV);
      vec2 sampleSeed = sampleValue.xy;

      if (sampleSeed.x != 0.f || sampleSeed.y != 0.f) {
        vec2 diff = sampleSeed - uv;
        float dist = dot(diff, diff);

        if (dist < nearestDist) {
          nearestDist = dist;
          nearestSeed = sampleValue;
        }
      }
    }
  }

  gl_FragColor = nearestSeed;
}

