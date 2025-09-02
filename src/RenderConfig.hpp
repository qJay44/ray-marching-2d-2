#pragma once

#include "utils/types.hpp"
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Window/Mouse.hpp>

struct RenderConfig {
  sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({1200, 900}), "CMake SFML Project");
  sf::RenderTexture sceneTexture;
  sf::RenderTexture sdfTexture;
  sf::RenderTexture ping;
  sf::RenderTexture pong;
  sf::RectangleShape screenRect;

  sf::Shader mouseShader = sf::Shader(fspath("mouse.frag"), sf::Shader::Type::Fragment);
  float mouseDrawRadius = 10.f;
  sf::Vector3f mouseDrawColor{1.f, 0.f, 1.f};

  int raysPerPixel = 32;
  int stepsPerRay = 32;
  float epsilon = 0.001f;

  void init() {
    sf::Vector2u winSize = window.getSize();

    sceneTexture = sf::RenderTexture(winSize);
    sdfTexture = sf::RenderTexture(winSize);
    ping = sf::RenderTexture(winSize);
    pong = sf::RenderTexture(winSize);
    screenRect = sf::RectangleShape(sf::Vector2f(winSize));
  }

  void update() {

  }

  void drawAtMouse() {
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
      return;

    mouseShader.setUniform("u_resolution", sf::Vector2f(window.getSize()));
    mouseShader.setUniform("u_pos", sf::Mouse::getPosition(window));
    mouseShader.setUniform("u_color", mouseDrawColor);
    mouseShader.setUniform("u_radius", mouseDrawRadius);

    sceneTexture.draw(screenRect, &mouseShader);
    sceneTexture.display();
  }
};

