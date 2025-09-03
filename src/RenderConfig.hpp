#pragma once

#include "utils/types.hpp"

struct RenderConfig {
  sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({1200, 900}), "CMake SFML Project");
  sf::RenderTexture sceneTexture;
  sf::RenderTexture sdfTexture;
  sf::RenderTexture ping;
  sf::RenderTexture pong;
  sf::RectangleShape screenRect;

  sf::Sprite sceneSprite = sf::Sprite(sceneTexture.getTexture());

  sf::Shader mouseShader = sf::Shader(fspath("mouse.frag"), sf::Shader::Type::Fragment);
  float mouseDrawRadius = 10.f;
  sf::Vector3f mouseDrawColor{1.f, 0.f, 1.f};

  int raysPerPixel = 32;
  int stepsPerRay = 32;
  float epsilon = 0.001f;

  void init();
  void update();
  void drawAtMouse();
};

