
#ifndef BIFROST_CLIENT_H
#define BIFROST_CLIENT_H

#include <string>
#include <cstdint>
#include <memory>
#include <bifrost/global_constants.h>

class bifrost_client_impl;

class bifrost_client {
public:
    explicit bifrost_client(std::string application_name, std::string window_title, bool prefer_full_screen, uint32_t swapchain_image_count);
    void start();
    void stop();
    std::pair<uint32_t, void *> acquire_swapchain_image();
    void submit_frame(uint32_t framebuffer_id, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, refresh_type refresh_type);
    std::pair<uint32_t, uint32_t> get_swapchain_extent() const;
private:
    std::shared_ptr<bifrost_client_impl> impl;
};

#endif // BIFROST_CLIENT_H
