#pragma once

#include <cassert>
#include <chrono>
#include <functional>

#include "ImGuiProfilerRenderer.h"
#include "ProfilerTask.h"
#include "utils/types.hpp"

class ProfilerManager {
public:
  ProfilerManager(u8 tasksCount, size_t framesCount)
    : tasksCount(tasksCount),
      maxFrameTime(1.f / framesCount),
      graph(framesCount) {

    tasks = new legit::ProfilerTask[tasksCount];
  }

  ~ProfilerManager() {
    delete[] tasks;
  }

  void updateTask(size_t i, const std::function<void()>& func, const std::string& name) {
    using namespace std::chrono;
    assert(i < tasksCount);

    legit::ProfilerTask& task = tasks[i];
    task.name = name;
    task.color = getColorBright(i);
    task.startTime = 0.f;

    auto start = steady_clock::now();
    func();
    auto end = steady_clock::now();

    duration<float> time = end - start;
    task.endTime = time.count() * 1000.f;
  }

  void render(int graphWidth, int legendWidth, int height, int frameIndexOffset = 1)  {
    graph.LoadFrameData(tasks, tasksCount);
    graph.RenderTimings(graphWidth, legendWidth, height, frameIndexOffset, maxFrameTime * 1000.f);
  }

private:
  u8 tasksCount = 0;
  float maxFrameTime;
  legit::ProfilerTask* tasks = nullptr;
  ImGuiUtils::ProfilerGraph graph;

private:
  const u32& getColorBright(size_t i) const {
    using namespace legit::Colors;

    static constexpr u32 colors[8] = {
      turqoise, emerald, peterRiver, amethyst, sunFlower, carrot, alizarin, clouds
    };

    return colors[i % 8];
  }

  const u32& getColorDim(size_t i) const {
    using namespace legit::Colors;

    static constexpr u32 colors[8] = {
      greenSea, nephritis, belizeHole, wisteria, orange, pumpkin, pomegranate, silver
    };

    return colors[i % 8];
  }
};

