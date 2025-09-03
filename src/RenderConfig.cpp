#include "RenderConfig.hpp"
#include "gui.hpp"
#include <algorithm>


void RenderConfig::init(sf::Vector2u winSize) {
  this->winSize = winSize;

  sceneTexture = sf::RenderTexture(winSize);
  sdfTexture = sf::RenderTexture(winSize);
  ping = sf::RenderTexture(winSize);
  pong = sf::RenderTexture(winSize);
  screenRect = sf::RectangleShape(sf::Vector2f(winSize));

  sceneSprite = sf::Sprite(sceneTexture.getTexture());
  sceneSprite.setScale({1.f, -1.f});
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
}

void RenderConfig::clearScene() {
  sceneTexture.clear({10, 10, 10, 255});
}

void RenderConfig::drawMouseAt(const sf::Vector2f& point) {
  mouse.shader.setUniform("u_resolution", sf::Vector2f(winSize));
  mouse.shader.setUniform("u_pos", point);
  mouse.shader.setUniform("u_color", mouse.drawColor);
  mouse.shader.setUniform("u_radius", mouse.drawRadius);

  sceneTexture.draw(screenRect, &mouse.shader);
  sceneTexture.display();

  sceneSprite.setTexture(sceneTexture.getTexture());
  sceneSprite.setPosition({0.f, static_cast<float>(sceneTexture.getSize().y)});
}

