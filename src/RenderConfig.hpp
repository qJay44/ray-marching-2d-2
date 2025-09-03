#pragma once

#include "utils/types.hpp"

class RenderConfig {
public:
  void init(sf::Vector2u winSize);
  void update();
  void clearScene();

  void onMousePressed(const sf::Vector2f& pos);
  void onMouseReleased();
  void onMouseMoved(const sf::Vector2f& pos);

  const sf::Sprite& getSceneSprite() const;

private:
  friend struct gui;

  sf::Vector2u winSize;

  sf::RenderTexture sceneTexture;
  sf::RenderTexture sdfTexture;
  sf::RenderTexture ping;
  sf::RenderTexture pong;
  sf::RectangleShape screenRect;

  sf::Sprite sceneSprite = sf::Sprite(sceneTexture.getTexture());

  struct Mouse {
    sf::Shader shader = sf::Shader(fspath("mouse.frag"), sf::Shader::Type::Fragment);
    float drawRadius = 10.f;
    sf::Vector3f drawColor{1.f, 0.f, 1.f};
    sf::Vector2f prevPos;
  } mouse;

  int raysPerPixel = 32;
  int stepsPerRay = 32;
  float epsilon = 0.001f;

  bool isDrawing = false;

private:
  void drawMouseAt(const sf::Vector2f& point);
};

