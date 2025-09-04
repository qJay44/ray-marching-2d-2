#include "RenderConfig.hpp"

#include <algorithm>
#include <cmath>

constexpr sf::Color clearCol{0, 0, 0, 0};

template<typename T = u8>
void createTextureGl(
  const sf::Texture* tex,
  GLenum internalFormat,
  GLenum format = GL_RGBA,
  GLenum type = GL_UNSIGNED_BYTE,
  const T* pixels = nullptr
) {
  GLuint id = tex->getNativeHandle();
  sf::Vector2u size = tex->getSize();

  glGenTextures(1, &id);
  sf::Texture::bind(tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0, format, type, pixels);
  sf::Texture::bind(nullptr);
}

template<typename T = u8>
void updateTextureGl(
  const sf::Texture* tex,
  GLenum format = GL_RGBA,
  GLenum type = GL_UNSIGNED_BYTE,
  const T* pixels = nullptr
) {
  sf::Vector2u size = tex->getSize();
  sf::Texture::bind(tex);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, format, type, pixels);
  sf::Texture::bind(nullptr);
}

void RenderConfig::init(sf::Vector2u winSize) {
  this->winSize = winSize;
  sf::Vector2f winSizeF(winSize);

  sceneTexture = sf::RenderTexture(winSize);
  seedTexture = sf::RenderTexture(winSize);
  sdfTexture = sf::RenderTexture(winSize);
  pingJFA = sf::RenderTexture(winSize);
  pongJFA = sf::RenderTexture(winSize);
  screenRect = sf::RectangleShape(winSizeF);
  sandTex = sf::Texture(winSize);

  createTextureGl(&sandTex, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
  createTextureGl(&sceneTexture.getTexture(), GL_RGBA16F);
  createTextureGl(&seedTexture.getTexture(), GL_RG16F);
  createTextureGl(&pingJFA.getTexture(), GL_RG16F);
  createTextureGl(&pongJFA.getTexture(), GL_RG16F);
  createTextureGl(&sdfTexture.getTexture(), GL_R16F);

  sceneSprite = sf::Sprite(sceneTexture.getTexture());
  sceneSprite.setScale({1.f, -1.f});
  sceneSprite.setPosition({0.f, static_cast<float>(sceneTexture.getSize().y)});

  seedShader.setUniform("u_surfaceTex", sceneTexture.getTexture());
  seedShader.setUniform("u_resolution", winSizeF);

  jfaSprite = sf::Sprite(sceneSprite);
  jfaShader.setUniform("u_resolution", winSizeF);

  sdfShader.setUniform("u_jfaTex", jfaSprite.getTexture());
  sdfShader.setUniform("u_resolution", winSizeF);

  giShader.setUniform("u_sceneTex", sceneTexture.getTexture());
  giShader.setUniform("u_sdfTex", sdfTexture.getTexture());
  giShader.setUniform("u_blueNoiseTex", blueNoiseTex);
  giShader.setUniform("u_resolution", winSizeF);

  clSand.createGrid({{winSize.x, winSize.y}});

  calcPassesJFA();
}

void RenderConfig::addProfilier(ProfilerManager* pm) {
  profilerManager = pm;
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
  profilerManager->updateTask(5, [&]() {
    clSand.fall();
  }, "Sand::fall");

  updateTextureGl(&sandTex, GL_RGBA, GL_HALF_FLOAT, clSand.getPixels());

  // temp
  sceneTexture.draw(sf::Sprite(sandTex));
  sceneTexture.display();

  drawSeed();
  drawJFA();
  drawSDF();
}

void RenderConfig::clearScene() {
  sceneTexture.clear(clearCol);
}

void RenderConfig::drawGI(sf::RenderWindow& window) {
  profilerManager->updateTask(4, [&]() {
    giShader.setUniform("u_stepsPerRay", stepsPerRay);
    giShader.setUniform("u_raysPerPixel", raysPerPixel);
    giShader.setUniform("u_epsilon", epsilon);
    window.draw(screenRect, &giShader);
  }, "drawGI");
}

void RenderConfig::calcPassesJFA() {
  if (autoJfaPasses)
    jfaPasses = static_cast<int>(std::ceil(std::log2(std::max(winSize.x, winSize.y))));
}

void RenderConfig::drawMouseAt(const sf::Vector2f& point) {
  profilerManager->updateTask(0, [&]() {
    mouse.shader.setUniform("u_pos", point);
    mouse.shader.setUniform("u_color", mouse.drawColor);
    mouse.shader.setUniform("u_radius", mouse.drawRadius);

    if (isSand) {
      cl_float2 clPoint{{point.x, point.y}};
      cl_float3 clDrawColor{{mouse.drawColor.x, mouse.drawColor.y, mouse.drawColor.z}};
      clSand.draw(clPoint, clDrawColor, mouse.drawRadius);
    } else {
      sceneTexture.draw(screenRect, &mouse.shader);
      sceneTexture.display();
      sceneSprite.setTexture(sceneTexture.getTexture());
    }
  }, "drawMouseAt");
}

void RenderConfig::drawSeed() {
  profilerManager->updateTask(1, [&]() {
    seedTexture.clear(clearCol);
    seedTexture.draw(screenRect, &seedShader);
    seedTexture.display();
  }, "drawSeed");
}

void RenderConfig::drawJFA() {
  profilerManager->updateTask(2, [&]() {
    sf::RenderTexture* inputTex = &pingJFA;
    sf::RenderTexture* outputTex = &pongJFA;
    sf::RenderTexture* lastTex = outputTex;

    inputTex->clear(clearCol);
    inputTex->draw(sf::Sprite(seedTexture.getTexture()));
    inputTex->display();

    for (int i = 0; i < jfaPasses; i++) {
      jfaShader.setUniform("u_inputTex", inputTex->getTexture());
      jfaShader.setUniform("u_offset", 1 << (jfaPasses - i - 1));

      outputTex->clear(clearCol);
      outputTex->draw(screenRect, &jfaShader);
      outputTex->display();

      sf::RenderTexture* temp = inputTex;
      inputTex = outputTex;
      outputTex = temp;
      lastTex = temp;
    }

    jfaSprite.setTexture(lastTex->getTexture());
  }, "drawJFA");
}

void RenderConfig::drawSDF() {
  profilerManager->updateTask(3, [&]() {
    sdfTexture.clear(clearCol);
    sdfTexture.draw(screenRect, &sdfShader);
    sdfTexture.display();
    sdfShader.setUniform("u_jfaTex", jfaSprite.getTexture());
  }, "drawSDF");
}

