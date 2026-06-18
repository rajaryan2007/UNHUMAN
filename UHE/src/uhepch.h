#pragma once

#define ENTT_STANDARD_CPP 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <UHE/Core/Log.h>
#include <UHE/Debug/TracySetup.h>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef UHE_PLATFORM_WINDOWS
    #include <Windows.h>
#endif // UHE_PLATFORM_WINDOWS
