#include <cstdlib>

#include "RenderConfig.hpp"
#include "gui.hpp"
#include "utils/utils.hpp"

#include "imconfig-SFML.h"
#include "imgui-SFML.h"

int main() {
  // Assuming the executable is launching from its own directory
  CHDIR("../../..");

  srand(static_cast<unsigned int>(time(nullptr)));
  sf::RenderWindow window(sf::VideoMode({1200, 720}), "MyProgram");
  // window.setFramerateLimit(144);

  if (!gladLoadGL([](const char* name) {
    return reinterpret_cast<GLADapiproc>(sf::Context::getFunction(name));
  })) {
    error("GLAD load error");
  }

  if (!ImGui::SFML::Init(window))
    error("ImGui init error");

  RenderConfig renderConfig;
  renderConfig.init(window);

  gui::renderConfig = &renderConfig;

  // Loop related
  sf::Clock clock;
  sf::Vector2i mousePos;
  float dt;

  struct Avg {
    float ms = 0.f;
    size_t fps = 0;
    size_t frameIdx = 0;
  } avg;

  while (window.isOpen()) {
    // ----- Events ----------------------------------- //

    while (const std::optional event = window.pollEvent()) {
      ImGui::SFML::ProcessEvent(window, *event);

      if (event->is<sf::Event::Closed>()) {
        window.close();
      } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        switch (keyPressed->scancode) {
          case sf::Keyboard::Scancode::Q:
            window.close();
            break;
          case sf::Keyboard::Scancode::E:
            gui::toggle();
            break;
          default:
            break;
        };
      } else if (const auto* _ = event->getIf<sf::Event::Resized>()) {
        renderConfig.init(window);
      } else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
        renderConfig.onMouseMoved(sf::Vector2f(mouseMoved->position));
      }
    }

    // ----- Update meta ------------------------------ //

    ImGui::SFML::Update(window, clock.getElapsedTime());

    dt = clock.restart().asSeconds();
    mousePos = sf::Mouse::getPosition(window);

    size_t fps = static_cast<size_t>(1.f / dt);
    float ms = dt * 1000.f;

    if (avg.frameIdx++ < 90) {
      avg.fps += fps;
      avg.ms += ms;
    } else {
      avg.fps /= avg.frameIdx;
      avg.ms /= avg.frameIdx;
      avg.frameIdx = 1;
    }

    // ----- Update objects --------------------------- //

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !gui::isHovered()) {
      renderConfig.onMousePressed(sf::Vector2f(mousePos));
    } else {
      renderConfig.onMouseReleased();
    }

    renderConfig.update();

    // ----- Draw ------------------------------------- //

    window.clear();

    renderConfig.drawGI(window);

    gui::draw();
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();

  return 0;
}

