#include "gui.hpp"

#include "imconfig-SFML.h"
#include "imgui-SFML.h"
#include "imgui.h"

#include "utils/utils.hpp"
#include "GL/gl.h"
#include <cassert>

#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46 // glad.h

namespace gui {

static bool toggleWholeWindow = false;

RenderConfig* renderConfig = nullptr;

void toggle() {
  toggleWholeWindow = true;
}

void draw() {
  static RunOnce a([]() {
    ImGui::SetNextWindowPos({0, 0});
  });

  assert(renderConfig);

  ImGui::Begin("Debug");

  if (toggleWholeWindow) {
    ImGui::SetWindowCollapsed(!ImGui::IsWindowCollapsed());
    toggleWholeWindow = false;
  }

  if (ImGui::CollapsingHeader("Config")) {
    ImGui::SliderFloat("mouseDrawRadius", &renderConfig->mouseDrawRadius, 1.f, 100.f);
    ImGui::SliderInt("Rays per pixel", &renderConfig->raysPerPixel, 1, 128);
    ImGui::SliderInt("Steps per ray", &renderConfig->stepsPerRay, 1, 128);

    ImGui::Text("Epsilon: %f", renderConfig->epsilon); ImGui::SameLine();
    if (ImGui::ArrowButton("##left" , ImGuiDir_Left))  {renderConfig->epsilon *= 0.1f;} ImGui::SameLine();
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {renderConfig->epsilon *= 10.f;}
  }

  if (ImGui::CollapsingHeader("Textures")) {
    static const GLint swizzle[4] = {GL_RED, GL_RED, GL_RED, GL_ONE};

    static float scale = 0.2f;
    ImGui::SliderFloat("Scaling", &scale, 0.1f, 1.f);

    ImGui::SeparatorText("Scene");
    sf::Vector2f sceneSize = sf::Vector2f(renderConfig->sceneTexture.getSize());
    ImGui::Image(renderConfig->sceneTexture.getTexture().getNativeHandle(), sceneSize * scale, {0.f, 1.f}, {1.f, 0.f});

    ImGui::SeparatorText("SDF");
    sf::Vector2f sdfSize = sf::Vector2f(renderConfig->sdfTexture.getSize());
    sf::Texture::bind(&renderConfig->sdfTexture.getTexture());
    // glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    ImGui::Image(renderConfig->sdfTexture.getTexture().getNativeHandle(), sdfSize * scale, {0.f, 1.f}, {1.f, 0.f});
  }

  ImGui::End();

  ImGui::SFML::Render(renderConfig->window);
}

} // namespace gui

