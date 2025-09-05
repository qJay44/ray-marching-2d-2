#include <cstdlib>
#include <format>
#include <direct.h>

#include "ProfilerManager.hpp"
#include "RenderConfig.hpp"
#include "gui.hpp"
#include "utils/utils.hpp"

int main() {
  // Assuming the executable is launching from its own directory
  _chdir("../../../src");

  srand(static_cast<unsigned int>(time(nullptr)));
  sf::RenderWindow window(sf::VideoMode({1200, 900}), "", sf::Style::Default);
  // window.setFramerateLimit(144);

  // GLAD init
  if (!gladLoadGL()) {
    printf("Failed to initialize GLAD\n");
    return EXIT_FAILURE;
  }

  if (!ImGui::SFML::Init(window))
    error("ImGui init error");

  ProfilerManager profilerManager(144);
  RenderConfig renderConfig;
  renderConfig.init(window.getSize());
  renderConfig.addProfilier(&profilerManager);

  gui::renderConfig = &renderConfig;

  // Loop related
  sf::Clock clock;
  sf::Vector2i mousePos;
  float dt;
  legit::ProfilerTask displayTask;

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
      } else if (const auto* winResized = event->getIf<sf::Event::Resized>()) {
        renderConfig.init(winResized->size);
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
      window.setTitle(std::format("FPS: {}, {:.2f} ms", avg.fps / avg.frameIdx, avg.ms / avg.frameIdx));
    } else {
      avg.fps /= avg.frameIdx;
      avg.ms /= avg.frameIdx;
      avg.frameIdx = 1;
      window.setTitle(std::format("FPS: {}, {:.2f} ms", avg.fps, avg.ms));
    }

    profilerManager.clearTasks();
    profilerManager.addTask(displayTask);

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

    // Happens after gui draw so add it manually after clearing tasks at the next frame
    displayTask = profilerManager.startTask([&] {
        window.display();
    }, "window.display()");

  }

  ImGui::SFML::Shutdown();
}

