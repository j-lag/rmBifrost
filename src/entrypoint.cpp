#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <bifrost/bifrost.h>
#include "constants.h"
#include "hook_typedefs.h"

std::shared_ptr<bifrost> bifrost_instance;

__attribute__((constructor))
void library_entrypoint() {

    spdlog::set_level(spdlog::level::debug);  // Set to log info and above
    spdlog::set_default_logger(spdlog::stdout_color_mt("console"));  // Use colored console logger
    spdlog::flush_on(spdlog::level::info);  // Automatically flush when log level is info or above


    if (std::getenv(ENV_DEBUG)) {
        spdlog::set_level(spdlog::level::debug);
    }
    spdlog::set_pattern("<bifrost> [%H:%M:%S] [%^%l%$] %v");
    
    spdlog::info("Bifrost loading...");
    bifrost_instance = std::make_shared<bifrost>();
}