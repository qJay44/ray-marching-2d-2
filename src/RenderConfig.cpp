#include "RenderConfig.hpp"

#include <algorithm>
#include <cmath>

void RenderConfig::init(sf::Vector2u winSize) {
  this->winSize = winSize;

  sceneTexture = sf::RenderTexture(winSize);
  seedTexture = sf::RenderTexture(winSize);
  ping = sf::RenderTexture(winSize);
  pong = sf::RenderTexture(winSize);
  screenRect = sf::RectangleShape(sf::Vector2f(winSize));

  // sceneTexture.clear();
  // seedTexture.clear();
  // ping.clear();
  // pong.clear();

  sceneSprite = sf::Sprite(sceneTexture.getTexture());
  sceneSprite.setScale({1.f, -1.f});
  sceneSprite.setPosition({0.f, static_cast<float>(sceneTexture.getSize().y)});

  seedShader.setUniform("u_surfaceTex", sceneTexture.getTexture());
  seedShader.setUniform("u_resolution", sf::Vector2f(winSize));

  jfaSprite = sf::Sprite(sceneSprite);
  jfaShader.setUniform("u_resolution", sf::Vector2f(winSize));

  calcPassesJFA();
}

void RenderConfig::onMousePressed(const sf::Vector2f& pos) {
  isDrawing = true;
  mouse.prevPos = pos;
  drawMouseAt(pos);
}

void RenderConfig::onMouseReleased() {
  isDrawing = false;
}

void RenderConfig::onMouseMoved(const sf::Vector2f& pos) {
  if (!isDrawing) return;

  sf::Vector2f v = pos - mouse.prevPos;
  float dist = v.length();
  if (dist < 0.1f) return;

  constexpr float step = 0.25f;
  constexpr int maxSteps = 100;
  int steps = std::min(static_cast<int>(dist  / step), maxSteps);

  for (int i = 0; i < steps; i++) {
    float t = static_cast<float>(i) / steps;
    drawMouseAt(mouse.prevPos + t * (pos - mouse.prevPos));
  }

  mouse.prevPos = pos;
}

const sf::Sprite& RenderConfig::getSceneSprite() const {
  return sceneSprite;
}

void RenderConfig::update() {
  drawSeed();
  drawJFA();
}

void RenderConfig::clearScene() {
  sceneTexture.clear({0, 0, 0, 0});
}

void RenderConfig::calcPassesJFA() {
  if (autoJfaPasses)
    jfaPasses = static_cast<int>(std::ceil(std::log2(std::max(winSize.x, winSize.y))));
}

void RenderConfig::drawMouseAt(const sf::Vector2f& point) {
  mouse.shader.setUniform("u_pos", point);
  mouse.shader.setUniform("u_color", mouse.drawColor);
  mouse.shader.setUniform("u_radius", mouse.drawRadius);

  sceneTexture.draw(screenRect, &mouse.shader);
  sceneTexture.display();

  sceneSprite.setTexture(sceneTexture.getTexture());
}

void RenderConfig::drawSeed() {
  seedTexture.draw(screenRect, &seedShader);
  seedTexture.display();
}

void RenderConfig::drawJFA() {
  sf::RenderTexture* inputTex = &ping;
  sf::RenderTexture* outputTex = &pong;

  inputTex->draw(sf::Sprite(seedTexture.getTexture()));
  inputTex->display();

  for (int i = 0; i < jfaPasses; i++) {
    jfaShader.setUniform("u_inputTex", inputTex->getTexture());
    jfaShader.setUniform("u_offset", 1 << (jfaPasses - i - 1));

    outputTex->draw(screenRect, &jfaShader);
    outputTex->display();

    // Odds (1, 3, 5, ...)
    if (i & 1) {
      inputTex = &ping;
      outputTex = &pong;
    } else {
      inputTex = &pong;
      outputTex = &ping;
    }
  }

  jfaSprite.setTexture(jfaPasses & 1 ? pong.getTexture() : ping.getTexture());
}

