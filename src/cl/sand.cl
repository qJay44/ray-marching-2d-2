__kernel void draw(
  __write_only image2d_t img,
  const float2 pos,
  const float3 color,
  const float radius
) {
  int x = get_global_id(0);
  int y = get_global_id(1);

  float2 coord = (float2)(x, y);
  float2 dist = coord - pos;
  float dd = dot(dist, dist);
  float rr = radius * radius;

  float4 col = (float4)(color, 1.f) * step(dd, rr);
  write_imagef(img, (int2)(x, y), col);
}

__kernel void fall(
  __read_only image2d_t imgRead,
  __write_only image2d_t imgWrite,
  __global int* locks
) {
  // Assuming imgRead and imgWrite have same size
  int width = get_image_width(imgRead);
  int height = get_image_height(imgRead);

  int x = get_global_id(0);
  int y = get_global_id(1);

  if (x >= width || y >= height)
    return;

  // If have sand
  float4 texel = read_imagef(imgRead, (int2)(x, y));
  if (texel.w == 1.f) {
    int belowY = y + 1;

    // If inside image
    if (belowY < height) {
      int belowIdx = belowY * width + x;
      int expected = 0;
      int acquired = atomic_cmpxchg(&locks[belowIdx], expected, 1);

      // If not locked
      if (acquired == 0.f) {
        float4 texelBelow = read_imagef(imgRead, (int2)(x, belowY));

        // If below is empty
        if (texelBelow.w == 0.f) {
          texelBelow = texel;
          texel = 0.f;
          write_imagef(imgWrite, (int2)(x, y), 0.f);
          write_imagef(imgWrite, (int2)(x, belowY), texel);
        } else {
          atomic_xchg(&locks[belowIdx], 0);
        }
      }
    }
  };
}

