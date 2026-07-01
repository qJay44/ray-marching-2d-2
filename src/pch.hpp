#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#ifdef _WIN32
  #include <direct.h>
  #include "glad/glad.h"
  #define CHDIR(p) _chdir(p);
#else
  #include <unistd.h>
  #include "glad/gl.h"
  #define CHDIR(p) chdir(p);
#endif

