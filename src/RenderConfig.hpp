#pragma once

#include "ProfilerManager.hpp"
#include "cl/Sand.hpp"
#include "utils/types.hpp"

class RenderConfig {
public:
  void init(sf::Vector2u winSize);
  void addProfilier(ProfilerManager* pm);
  void update();
  void clearScene();
  void drawGI(sf::RenderWindow& window);

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
  sf::RenderTexture pingJFA;
  sf::RenderTexture pongJFA;
  sf::RectangleShape screenRect;

  const sf::Texture blueNoiseTex = sf::Texture("res/tex/LDR_LLL1_0.png");
  sf::Texture sandTex;

  sf::Sprite sceneSprite = sf::Sprite(sceneTexture.getTexture());
  sf::Sprite jfaSprite = sf::Sprite(sceneSprite);

  sf::Shader seedShader = sf::Shader(fspath("shaders/seed.frag"), sf::Shader::Type::Fragment);
  sf::Shader jfaShader  = sf::Shader(fspath("shaders/jfa.frag") , sf::Shader::Type::Fragment);
  sf::Shader sdfShader  = sf::Shader(fspath("shaders/sdf.frag") , sf::Shader::Type::Fragment);
  sf::Shader giShader   = sf::Shader(fspath("shaders/gi.frag")  , sf::Shader::Type::Fragment);

  struct Mouse {
    sf::Shader shader = sf::Shader(fspath("shaders/mouse.frag"), sf::Shader::Type::Fragment);
    float drawRadius = 5.f;
    sf::Vector3f drawColor{1.f, 0.f, 1.f};
    sf::Vector2f prevPos;
  } mouse;

  cl::Sand clSand;

  int raysPerPixel = 32;
  int stepsPerRay = 32;
  float epsilon = 0.001f;
  int jfaPasses = 1;

  bool isDrawing = false;
  bool autoJfaPasses = true;
  bool isSand = true;

  ProfilerManager* profilerManager = nullptr;

private:
  void calcPassesJFA();
  void drawMouseAt(const sf::Vector2f& point);
  void drawSeed();
  void drawJFA();
  void drawSDF();
};

