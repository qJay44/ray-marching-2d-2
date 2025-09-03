#pragma once

#include "imconfig-SFML.h"
#include "imgui-SFML.h"
#include "imgui.h"

namespace gui {

extern struct RenderConfig* renderConfig;

bool isHovered();

void toggle();
void draw();

} // namespace gui

