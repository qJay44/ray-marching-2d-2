#include "Sand.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

#include "CL/cl.h"
#include "utils/utils.hpp"

#define ATTRIBUTE_COUNT 5u

const cl_platform_info attributeTypes[ATTRIBUTE_COUNT] = {
  CL_PLATFORM_NAME,
  CL_PLATFORM_VENDOR,
  CL_PLATFORM_VERSION,
  CL_PLATFORM_PROFILE,
  CL_PLATFORM_EXTENSIONS
};

const char* const attributeNames[ATTRIBUTE_COUNT] = {
  "CL_PLATFORM_NAME",
  "CL_PLATFORM_VENDOR",
  "CL_PLATFORM_VERSION",
  "CL_PLATFORM_PROFILE",
  "CL_PLATFORM_EXTENSIONS"
};

namespace cl {

Sand::Sand(bool printInfo) {
  cl_platform_id platforms[64];
  cl_uint platformCount;

  [[maybe_unused]]
  cl_int platformsResult = clGetPlatformIDs(64, platforms, &platformCount);
  assert(platformsResult == CL_SUCCESS);

  if (printInfo) {
    for (cl_uint i = 0; i < platformCount; i++) {
      for (size_t j = 0; j < ATTRIBUTE_COUNT; j++) {
        // Get platform attribute value size
        size_t infosize = 0;
        [[maybe_unused]]
        cl_int getPlatformInfoResult = clGetPlatformInfo(platforms[i], attributeTypes[j], 0, nullptr, &infosize);
        assert(getPlatformInfoResult == CL_SUCCESS);
        char* info = new char[infosize];

        // Get platform attribute value
        getPlatformInfoResult = clGetPlatformInfo(platforms[i], attributeTypes[j], infosize, info, nullptr);
        assert(getPlatformInfoResult == CL_SUCCESS);

        printf("%d.%zu %-11s: %s\n", i+1, j+1, attributeNames[j], info);

        delete[] info;
      }
    }
  }

  for (cl_uint i = 0; i < platformCount; i++) {
    cl_device_id devices[64];
    cl_uint deviceCount;
    cl_int deviceResult = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 64, devices, &deviceCount);

    if (deviceResult == CL_SUCCESS) {
      for (cl_uint j = 0; j < deviceCount; j++) {
        char vendorName[256];
        size_t vendorNameLength;
        cl_int deviceInfoResult = clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, 256, vendorName, &vendorNameLength);
        if (deviceInfoResult == CL_SUCCESS) {
          device = devices[j];
          break;
        }
      }
    }
  }

  assert(device);

  if (printInfo) {
    puts("");

    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(maxDimensions), &maxDimensions, nullptr);
    printf("2.1 CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: %zu\n", maxDimensions);

    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxLocalSize), &maxLocalSize, nullptr);
    printf("2.2 CL_DEVICE_MAX_WORK_GROUP_SIZE: %zu\n", maxLocalSize);

    size_t* maxDimensionsValues = new size_t[maxDimensions];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, maxDimensions * sizeof(maxDimensionsValues[0]), maxDimensionsValues, nullptr);

    printf("2.3 CL_DEVICE_MAX_WORK_ITEM_SIZES: ");
    for (size_t i = 0; i < maxDimensions; i++) printf("%zu ", maxDimensionsValues[i]);
    printf("\n\n");

    delete[] maxDimensionsValues;
  }

  cl_int contextResult;
  context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &contextResult);
  assert(contextResult == CL_SUCCESS);

  cl_int commandQueueResult;
  commandQueue = clCreateCommandQueueWithProperties(context, device, 0, &commandQueueResult);
  assert(commandQueueResult == CL_SUCCESS);

  cl_int programResult;
  std::string clFile = readFile("cl/sand.cl");
  const char* programSource = clFile.c_str();
  size_t programSourceLength = 0;
  program = clCreateProgramWithSource(context, 1, &programSource, &programSourceLength, &programResult);
  assert(programResult == CL_SUCCESS);

  cl_int buildResult = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
  if (buildResult != CL_SUCCESS) {
    size_t logSize;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
    char *log = new char[logSize];
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
    error("Build Log:\n{}\n", log);
  }

  cl_int kernelResult;
  kernelDraw = clCreateKernel(program, "draw", &kernelResult);
  assert(kernelResult == CL_SUCCESS);

  kernelFall = clCreateKernel(program, "fall", &kernelResult);
  assert(kernelResult == CL_SUCCESS);
}

Sand::~Sand() {
  if (pixels) delete[] pixels;

  if (gpuImage0) clReleaseMemObject(gpuImage0);
  if (gpuImage1) clReleaseMemObject(gpuImage1);
  if (locks)     clReleaseMemObject(locks);

	clReleaseKernel(kernelFall);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
	clReleaseDevice(device);
}

void Sand::createGrid(cl_uint2 size) {
  this->size = size;

  if (pixels) delete[] pixels;
  if (gpuImage0) clReleaseMemObject(gpuImage0);
  if (gpuImage1) clReleaseMemObject(gpuImage1);

  pixels = new u16[size.x * size.y];

  cl_image_format format;
  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNORM_INT16;

  for (size_t i = 0; i < 2; i++) {
    cl_mem& currImage = i & 1 ? gpuImage1 : gpuImage0;
    cl_mem_flags flags = i & 1 ? CL_MEM_WRITE_ONLY : CL_MEM_READ_ONLY;

    cl_int gpuImageMallocResult;
    #ifdef CL_VERSION_1_2
      cl_image_desc imageDesc;
      memset(&imageDesc, 0, sizeof(imageDesc));
      imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
      imageDesc.image_width = size.x;
      imageDesc.image_height = size.y;
      currImage = clCreateImage(context, flags, &format, &imageDesc, nullptr, &gpuImageMallocResult);
    #else
      currImage = clCreateImage2D(context, flags, &format, size.x, size.y, 0, nullptr, &gpuImageMallocResult);
    #endif
    assert(gpuImageMallocResult == CL_SUCCESS);

    [[maybe_unused]]
    cl_int kernelArgResult = clSetKernelArg(kernelDraw, 0, sizeof(cl_mem), &currImage);
    assert(kernelArgResult == CL_SUCCESS);

    kernelArgResult = clSetKernelArg(kernelFall, 0, sizeof(cl_mem), &currImage);
    assert(kernelArgResult == CL_SUCCESS);
  }

  createLocks();
}

void Sand::draw(cl_float2 pos, cl_float3 color, float radius) {
  assert(currRead != nullptr);

  const size_t globalWorkSize[2] = {size.x, size.y};
  const size_t localWorkSize[2] = {16, 16};

  [[maybe_unused]]
  cl_int errCode;

  errCode = clSetKernelArg(kernelDraw, 0, sizeof(cl_mem)   , currRead); assert(errCode == CL_SUCCESS);
  errCode = clSetKernelArg(kernelDraw, 1, sizeof(cl_float2), &pos);     assert(errCode == CL_SUCCESS);
  errCode = clSetKernelArg(kernelDraw, 2, sizeof(cl_float3), &color);   assert(errCode == CL_SUCCESS);
  errCode = clSetKernelArg(kernelDraw, 3, sizeof(float)    , &radius);  assert(errCode == CL_SUCCESS);

  errCode = clEnqueueNDRangeKernel(commandQueue, kernelDraw, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr); assert(errCode == CL_SUCCESS);
  errCode = clFinish(commandQueue); assert(errCode == CL_SUCCESS);
}

void Sand::fall() {
  assert(currRead != nullptr);
  assert(currWrite != nullptr);

  constexpr size_t origin[3] = {0, 0, 0};
  const size_t region[3] = {size.x, size.y, 1};

  const size_t globalWorkSize[2] = {size.x, size.y};
  const size_t localWorkSize[2] = {16, 16};

  [[maybe_unused]]
  cl_int errCode;

  errCode = clSetKernelArg(kernelFall, 0, sizeof(cl_mem), currRead);  assert(errCode == CL_SUCCESS);
  errCode = clSetKernelArg(kernelFall, 1, sizeof(cl_mem), currWrite); assert(errCode == CL_SUCCESS);
  errCode = clSetKernelArg(kernelFall, 2, sizeof(cl_mem), &locks);    assert(errCode == CL_SUCCESS);

  errCode = clEnqueueNDRangeKernel(commandQueue, kernelFall, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr); assert(errCode == CL_SUCCESS);
  errCode = clEnqueueReadImage(commandQueue, *currWrite, CL_TRUE, origin, region, 0, 0, pixels, 0, nullptr, nullptr);         assert(errCode == CL_SUCCESS);

  errCode = clFinish(commandQueue); assert(errCode == CL_SUCCESS);

  clearLocks();
  swapImages();
}

const u16* Sand::getPixels() const {
  return pixels;
}

void Sand::createLocks() {
  if (locks) clReleaseMemObject(locks);

  cl_int errCode;
  locks = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * size.x * size.y, nullptr, &errCode);
  assert(errCode == CL_SUCCESS);

  clearLocks();
}

void Sand::clearLocks() {
  constexpr int zero = 0;

  [[maybe_unused]]
  cl_int errCode = clEnqueueFillBuffer(commandQueue, locks, &zero, sizeof(int), 0, sizeof(int) * size.x * size.y, 0, nullptr, nullptr);
  assert(errCode == CL_SUCCESS);

  errCode = clFinish(commandQueue);
  assert(errCode == CL_SUCCESS);
}

void Sand::swapImages() {
  assert(currRead != nullptr);
  assert(currWrite != nullptr);

  cl_mem* temp = currRead;
  currRead = currWrite;
  currWrite = temp;
}

} // namespace cl

