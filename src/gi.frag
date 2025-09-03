#define PI 3.14159265359f
#define TAU (2.f * PI)

uniform sampler2D u_sceneTex;
uniform sampler2D u_sdfTex;
uniform sampler2D u_blueNoiseTex;
uniform vec2 u_resolution;
uniform int u_stepsPerRay;
uniform int u_raysPerPixel;
uniform float u_epsilon;

vec4 rayMarch(vec2 pix, vec2 dir) {
  for (int step = 0; step < u_stepsPerRay; step++) {
    float dist = texture(u_sdfTex, pix).r;
    pix += dir * dist;

    // Bounds check
    if (floor(pix) != vec2(0.f))
      break;

    if (dist < u_epsilon) {
      return texture(u_sceneTex, pix);
    }
  }

  return vec4(0.f);
}

void main() {
  vec2 uv = gl_FragCoord.xy / u_resolution;
  uv.y = 1.f - uv.y;

  vec4 radiance = vec4(0.f);
  vec4 light = texture(u_sceneTex, uv);
  float raysPerPixelNorm = 1.f / float(u_raysPerPixel);

  if (light.a < 0.1f) {
    float angleStep = TAU * raysPerPixelNorm;
    float noise = texture(u_blueNoiseTex, uv).r;

    for (int i = 0; i < u_raysPerPixel; i++) {
      float angle = angleStep * (float(i) + noise);
      vec2 rayDir = vec2(cos(angle), -sin(angle));
      radiance += rayMarch(uv, rayDir);
    }

  } else if (length(light.rgb) >= 0.1f) {
    radiance = light;
  }

  vec4 finalRadiance = vec4(max(light, radiance * raysPerPixelNorm).rgb, 1.f);

  gl_FragColor = finalRadiance;
}

