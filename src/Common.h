#pragma once

#define NOMINMAX

// vulkan
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// new sdl2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

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