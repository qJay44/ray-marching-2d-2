#pragma once

#include "CL/cl.h"
#include "utils/types.hpp"

namespace cl {

class Sand {
public:
  Sand(bool printInfo = false);
  ~Sand();

  void createGrid(cl_uint2 size);
  void draw(cl_float2 pos, cl_float3 color, float radius);
  void fall();

  [[nodiscard]]
  const u16* getPixels() const;

private:
  cl_uint2 size;
  u16* pixels = nullptr;

  cl_device_id device = nullptr;
  size_t maxLocalSize;
  size_t maxDimensions;

  cl_context context;
  cl_command_queue commandQueue;

  cl_mem gpuImage0 = nullptr;
  cl_mem gpuImage1 = nullptr;
  cl_mem* currRead = &gpuImage0;
  cl_mem* currWrite = &gpuImage1;

  cl_mem locks = nullptr;

  cl_kernel kernelDraw;
  cl_kernel kernelFall;
  cl_program program;

private:
  void createLocks();
  void clearLocks();
  void swapImages();
};


} // namespace cl

