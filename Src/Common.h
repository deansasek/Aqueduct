#pragma once

#ifndef COMMON_H
#define COMMON_H

#include <vulkan/vulkan.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <array>
#include <set>
#include <optional>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

enum class API { Vulkan, DirectX12, OpenGL, Undefined };

#endif