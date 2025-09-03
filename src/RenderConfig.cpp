#include "RenderConfig.hpp"
#include "gui.hpp"

  void RenderConfig::init() {
    sf::Vector2u winSize = window.getSize();

    sceneTexture = sf::RenderTexture(winSize);
    sdfTexture = sf::RenderTexture(winSize);
    ping = sf::RenderTexture(winSize);
    pong = sf::RenderTexture(winSize);
    screenRect = sf::RectangleShape(sf::Vector2f(winSize));

    sceneSprite = sf::Sprite(sceneTexture.getTexture());
    sceneSprite.setScale({1.f, -1.f});
  }

  void RenderConfig::update() {

  }

  void RenderConfig::drawAtMouse() {
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || gui::isHovered())
      return;

    mouseShader.setUniform("u_resolution", sf::Vector2f(window.getSize()));
    mouseShader.setUniform("u_pos", sf::Vector2f(sf::Mouse::getPosition(window)));
    mouseShader.setUniform("u_color", mouseDrawColor);
    mouseShader.setUniform("u_radius", mouseDrawRadius);

    sceneTexture.draw(screenRect, &mouseShader);
    sceneTexture.display();

    sceneSprite.setTexture(sceneTexture.getTexture());
    sceneSprite.setPosition({0.f, static_cast<float>(sceneTexture.getSize().y)});
  }

