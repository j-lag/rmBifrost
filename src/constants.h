#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <bifrost/global_constants.h>
#include <mutex>

constexpr auto SCREEN_WIDTH = 1620;
constexpr auto SCREEN_HEIGHT = 2160;

constexpr auto ENV_DEBUG = "BIFROST_DEBUG";

inline std::mutex g_lvgl_mutex;

#endif // CONSTANTS_H