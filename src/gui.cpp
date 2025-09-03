#include "RenderConfig.hpp"
#include "gui.hpp"

#include <cassert>

#include "imgui.h"
#include "utils/utils.hpp"

static bool toggleWholeWindow = false;

RenderConfig* gui::renderConfig = nullptr;

bool gui::isHovered() {
  return ImGui::GetIO().WantCaptureMouse;
}

void gui::toggle() {
  toggleWholeWindow = true;
}

void setCursorUnderCurrentWidget() {
  ImVec2 imagePos = ImGui::GetItemRectMin();
  ImVec2 imageSize = ImGui::GetItemRectSize();

  float widgetPosX = imagePos.x;
  float widgetPosY = imagePos.y + imageSize.y + ImGui::GetStyle().ItemInnerSpacing.y;
  ImGui::SetCursorPos(ImVec2(widgetPosX, widgetPosY));
}

void gui::draw() {
  static RunOnce a([]() {
    ImGui::SetNextWindowPos({0, 0});
  });

  assert(renderConfig);

  ImGui::Begin("Debug");

  if (toggleWholeWindow) {
    ImGui::SetWindowCollapsed(!ImGui::IsWindowCollapsed());
    toggleWholeWindow = false;
  }

  if (ImGui::CollapsingHeader("Ray march")) {
    ImGui::SliderInt("Rays per pixel", &renderConfig->raysPerPixel, 1, 128);
    ImGui::SliderInt("Steps per ray", &renderConfig->stepsPerRay, 1, 128);

    ImGui::Text("Epsilon: %f", renderConfig->epsilon); ImGui::SameLine();
    if (ImGui::ArrowButton("##left" , ImGuiDir_Left))  {renderConfig->epsilon *= 0.1f;} ImGui::SameLine();
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {renderConfig->epsilon *= 10.f;}
  }

  if (ImGui::CollapsingHeader("Textures")) {
    static float scale = 0.2f;
    ImGui::SliderFloat("Size", &scale, 0.1f, 1.f);

    float maxWidth = renderConfig->winSize.x * 0.3f;

    if (ImGui::TreeNode("Scene")) {
      // ----- The image ------------------------------------------- //

      sf::Vector2f sceneSize = sf::Vector2f(renderConfig->sceneTexture.getSize());
      ImGui::Image(renderConfig->sceneTexture.getTexture().getNativeHandle(), sceneSize * scale);

      // ----- The config ------------------------------------------ //

      ImGui::SameLine();
      ImGui::BeginGroup();
      ImGui::PushItemWidth(maxWidth);

      RenderConfig::Mouse& mouse = renderConfig->mouse;
      ImGui::SliderFloat("Radius", &mouse.drawRadius, 1.f, 100.f);

      // Mouse draw color edit
      {
        static sf::Vector3f& cfgCol = mouse.drawColor;
        static float col[3] = {cfgCol.x, cfgCol.y, cfgCol.z};
        if (ImGui::ColorEdit3("Color", col))
          cfgCol = {col[0], col[1], col[2]};
      }

      if (ImGui::Button("Clear"))
        renderConfig->clearScene();

      ImGui::PopItemWidth();
      ImGui::EndGroup();

      // ----------------------------------------------------------- //

      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Seed")) {
      const sf::Texture& seedTex = renderConfig->seedTexture.getTexture();
      sf::Vector2f seedSize = sf::Vector2f(seedTex.getSize());
      sf::Texture::bind(&seedTex);
      ImGui::Image(seedTex.getNativeHandle(), seedSize * scale);

      ImGui::TreePop();
    }

    if (ImGui::TreeNode("JFA")) {
      // ----- The image ------------------------------------------- //

      const sf::Texture& jfaTex = renderConfig->jfaSprite.getTexture();
      sf::Vector2f jfaSize = sf::Vector2f(jfaTex.getSize());
      sf::Texture::bind(&jfaTex);
      ImGui::Image(jfaTex.getNativeHandle(), jfaSize * scale);

      // ----- The config ------------------------------------------ //

      ImGui::SameLine();
      ImGui::BeginGroup();
      ImGui::PushItemWidth(maxWidth);

      if (ImGui::Checkbox("Auto calculate passes", &renderConfig->autoJfaPasses))
        renderConfig->calcPassesJFA();

      ImGui::BeginDisabled(renderConfig->autoJfaPasses);
      ImGui::SliderInt("Passes", &renderConfig->jfaPasses, 1, 20);
      ImGui::EndDisabled();

      ImGui::PopItemWidth();
      ImGui::EndGroup();

      // ----------------------------------------------------------- //

      ImGui::TreePop();
    }

    if (ImGui::TreeNode("SDF")) {
      const sf::Texture& sdfTex = renderConfig->sdfTexture.getTexture();
      sf::Vector2f sdfSize = sf::Vector2f(sdfTex.getSize());
      sf::Texture::bind(&sdfTex);
      ImGui::Image(sdfTex.getNativeHandle(), sdfSize * scale);

      ImGui::TreePop();
    }
  }

  ImGui::End();
}

