#pragma once

#include <cassert>
#include <chrono>
#include <functional>
#include <vector>

#include "ImGuiProfilerRenderer.h"
#include "ProfilerTask.h"
#include "utils/types.hpp"

class ProfilerManager {
public:
  ProfilerManager(size_t framesCount) : graph(framesCount) {}

  void clearTasks() {
    tasks.clear();
  }

  const legit::ProfilerTask& startTask(
    const std::function<void()>& func,
    const std::string& name,
    const u32* color = nullptr
  ) {
    using namespace std::chrono;

    legit::ProfilerTask task;
    task.name = name;
    task.color = color ? *color : getColorBright(tasks.size());
    task.startTime = 0.f;

    auto start = steady_clock::now();
    func();
    auto end = steady_clock::now();

    duration<float> time = end - start;
    task.endTime = time.count();

    tasks.push_back(task);

    return tasks.back();
  }

  void addTask(const legit::ProfilerTask& task) {
    tasks.push_back(task);
  }

  void render(int graphWidth, int legendWidth, int height, int frameIndexOffset = 1) {
    graph.LoadFrameData(tasks.data(), tasks.size());
    graph.RenderTimings(graphWidth, legendWidth, height, frameIndexOffset, 1.f);
  }

private:
  std::vector<legit::ProfilerTask> tasks;
  ImGuiUtils::ProfilerGraph graph;

private:
  static const u32& getColorBright(size_t i) {
    using namespace legit::Colors;

    static constexpr u32 colors[8] = {
      turqoise, emerald, peterRiver, amethyst, sunFlower, carrot, alizarin, clouds
    };

    return colors[i % 8];
  }

  static const u32& getColorDim(size_t i) {
    using namespace legit::Colors;

    static constexpr u32 colors[8] = {
      greenSea, nephritis, belizeHole, wisteria, orange, pumpkin, pomegranate, silver
    };

    return colors[i % 8];
  }
};

