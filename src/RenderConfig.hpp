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
  sf::RenderTexture seedTexture;
  sf::RenderTexture sdfTexture;
  sf::RenderTexture ping;
  sf::RenderTexture pong;
  sf::RectangleShape screenRect;

  sf::Sprite sceneSprite = sf::Sprite(sceneTexture.getTexture());
  sf::Sprite jfaSprite = sf::Sprite(sceneSprite);

  struct Mouse {
    sf::Shader shader = sf::Shader(fspath("mouse.frag"), sf::Shader::Type::Fragment);
    float drawRadius = 5.f;
    sf::Vector3f drawColor{1.f, 0.f, 1.f};
    sf::Vector2f prevPos;
  } mouse;

  int raysPerPixel = 32;
  int stepsPerRay = 32;
  float epsilon = 0.001f;

  sf::Shader seedShader = sf::Shader(fspath("seed.frag"), sf::Shader::Type::Fragment);
  sf::Shader jfaShader = sf::Shader(fspath("jfa.frag"), sf::Shader::Type::Fragment);
  sf::Shader sdfShader = sf::Shader(fspath("sdf.frag"), sf::Shader::Type::Fragment);
  int jfaPasses = 1;

  bool isDrawing = false;
  bool autoJfaPasses = true;

private:
  void calcPassesJFA();
  void drawMouseAt(const sf::Vector2f& point);
  void drawSeed();
  void drawJFA();
  void drawSDF();
};

